#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bench.h"
#include "matrix_io.h"
#include "pool.h"
#include "util.h"

#define MAX_ITERS 10000u
#define MAX_WINDOW 999u
#define MAX_REPEAT 1000u
#define BLOCKS_PER_THREAD 4u
#define GEN_MIN 0
#define GEN_MAX 255

enum {
    OPT_ROWS = 256,
    OPT_COLS,
    OPT_SEED,
    OPT_REPEAT,
    OPT_BENCH,
    OPT_SAVE
};

typedef struct {
    size_t threads;
    size_t window;
    size_t iters;
    size_t repeat;
    size_t block;
    size_t rows;
    size_t cols;
    size_t bench_max;
    unsigned long seed;
    const char *input;
    const char *save;
    int manual;
    int random_src;
    int check;
    int print_matrix;
    int threads_given;
} options;

static void usage(FILE *out, const char *prog)
{
    fprintf(out,
        "usage: %s --threads P (--input FILE | --manual | --random) [options]\n"
        "\n"
        "  -t, --threads P      максимум одновременно работающих вычислительных потоков,\n"
        "                       включая главный (1..%u, по умолчанию 1)\n"
        "  -w, --window W       ширина окна фильтра, нечётное число (по умолчанию 3)\n"
        "  -k, --iters K        число применений медианного фильтра (по умолчанию 1)\n"
        "  -i, --input FILE     матрица из файла: ровно rows*cols целых чисел\n"
        "  -m, --manual         ввод матрицы с клавиатуры\n"
        "  -r, --random         сгенерировать матрицу (нужны --rows и --cols)\n"
        "      --rows R         число строк\n"
        "      --cols C         число столбцов\n"
        "      --seed S         начальное значение генератора (по умолчанию 1)\n"
        "  -b, --block N        строк в одном блоке задачи (по умолчанию rows/(4*P))\n"
        "      --repeat N       повторов измерения (по умолчанию 1)\n"
        "  -c, --check          сверить результат с последовательным эталоном\n"
        "      --bench P        прогнать P = 1, 2, 4, ... и вывести T_p, S(P), E(P)\n"
        "      --save FILE      сохранить результат в файл\n"
        "  -p, --print-matrix   вывести результирующую матрицу\n"
        "  -h, --help           эта справка\n",
        prog, POOL_MAX_THREADS);
}

static size_t auto_block_rows(size_t rows, size_t threads)
{
    size_t block = rows / (threads * BLOCKS_PER_THREAD);

    if (block == 0) {
        block = 1;
    }
    if (block > rows) {
        block = rows;
    }
    return block;
}

static int compare_with_reference(const int *data, const int *ref, size_t rows, size_t cols)
{
    for (size_t i = 0, n = rows * cols; i < n; ++i) {
        if (data[i] != ref[i]) {
            fprintf(stderr, "check FAILED at row %zu col %zu: got %d, expected %d\n",
                    i / cols, i % cols, data[i], ref[i]);
            return -1;
        }
    }
    printf("check: OK\n");
    return 0;
}

static void print_matrix(const char *title, const int *data, size_t rows, size_t cols)
{
    printf("%s (%zux%zu)\n", title, rows, cols);
    for (size_t r = 0; r < rows; ++r) {
        printf("%4zu:", r);
        for (size_t c = 0; c < cols; ++c) {
            printf(" %d", data[r * cols + c]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    static const struct option long_options[] = {
        {"threads",    required_argument, NULL, 't'},
        {"window",     required_argument, NULL, 'w'},
        {"iters",      required_argument, NULL, 'k'},
        {"input",      required_argument, NULL, 'i'},
        {"manual",     no_argument,       NULL, 'm'},
        {"random",     no_argument,       NULL, 'r'},
        {"block",      required_argument, NULL, 'b'},
        {"check",      no_argument,       NULL, 'c'},
        {"print-matrix", no_argument,     NULL, 'p'},
        {"help",       no_argument,       NULL, 'h'},
        {"rows",       required_argument, NULL, OPT_ROWS},
        {"cols",       required_argument, NULL, OPT_COLS},
        {"seed",       required_argument, NULL, OPT_SEED},
        {"repeat",     required_argument, NULL, OPT_REPEAT},
        {"bench",      required_argument, NULL, OPT_BENCH},
        {"save",       required_argument, NULL, OPT_SAVE},
        {NULL, 0, NULL, 0}
    };

    options cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.threads = 1;
    cfg.window = 3;
    cfg.iters = 1;
    cfg.repeat = 1;
    cfg.seed = 1;

    int opt;
    while ((opt = getopt_long(argc, argv, "t:w:k:i:mrb:chp", long_options, NULL)) != -1) {
        switch (opt) {
        case 't':
            cfg.threads = parse_size_range(optarg, 1, POOL_MAX_THREADS);
            cfg.threads_given = 1;
            break;
        case 'w':
            cfg.window = parse_size_range(optarg, 1, MAX_WINDOW);
            break;
        case 'k':
            cfg.iters = parse_size_range(optarg, 1, MAX_ITERS);
            break;
        case 'i':
            cfg.input = optarg;
            break;
        case 'm':
            cfg.manual = 1;
            break;
        case 'r':
            cfg.random_src = 1;
            break;
        case 'b':
            cfg.block = parse_size_range(optarg, 1, MATRIX_MAX_DIM);
            break;
        case 'c':
            cfg.check = 1;
            break;
        case 'p':
            cfg.print_matrix = 1;
            break;
        case 'h':
            usage(stdout, argv[0]);
            return 0;
        case OPT_ROWS:
            cfg.rows = parse_size_range(optarg, 1, MATRIX_MAX_DIM);
            break;
        case OPT_COLS:
            cfg.cols = parse_size_range(optarg, 1, MATRIX_MAX_DIM);
            break;
        case OPT_SEED:
            cfg.seed = (unsigned long)parse_size_range(optarg, 1, 1000000);
            break;
        case OPT_REPEAT:
            cfg.repeat = parse_size_range(optarg, 1, MAX_REPEAT);
            break;
        case OPT_BENCH:
            cfg.bench_max = parse_size_range(optarg, 1, POOL_MAX_THREADS);
            break;
        case OPT_SAVE:
            cfg.save = optarg;
            break;
        default:
            usage(stderr, argv[0]);
            return 1;
        }
    }

    if (optind < argc) {
        fprintf(stderr, "unexpected argument '%s'\n", argv[optind]);
        usage(stderr, argv[0]);
        return 1;
    }

    int sources = (cfg.input != NULL) + cfg.manual + cfg.random_src;
    if (sources == 0) {
        fprintf(stderr, "no input source: use --input FILE, --manual or --random\n");
        usage(stderr, argv[0]);
        return 1;
    }
    if (sources > 1) {
        die("choose exactly one input source: --input, --manual or --random");
    }
    if (cfg.input != NULL && (cfg.rows == 0 || cfg.cols == 0)) {
        die("--input needs --rows and --cols: the file contains only numbers");
    }
    if (cfg.random_src && (cfg.rows == 0 || cfg.cols == 0)) {
        die("--random needs --rows and --cols");
    }
    if (cfg.window % 2 == 0) {
        die("--window must be odd");
    }
    if (cfg.bench_max != 0 && cfg.threads_given) {
        die("--bench sets the thread count itself, --threads cannot be combined with it");
    }
    if (cfg.bench_max == 0 && cfg.repeat != 1) {
        die("--repeat only makes sense together with --bench");
    }

    matrix input = {NULL, 0, 0};
    if (cfg.input != NULL) {
        matrix_load_file(cfg.input, &input, cfg.rows, cfg.cols);
    } else if (cfg.manual) {
        matrix_load_stdin(&input, cfg.rows, cfg.cols);
    } else {
        matrix_generate(&input, cfg.rows, cfg.cols, cfg.seed, GEN_MIN, GEN_MAX);
    }

    int *src = input.data;
    size_t rows = input.rows;
    size_t cols = input.cols;
    size_t cells = rows * cols;

    int *dst = (int *)malloc(cells * sizeof(*dst));
    int *ref = cfg.check ? (int *)malloc(cells * sizeof(*ref)) : NULL;
    if (dst == NULL || (cfg.check && ref == NULL)) {
        die("out of memory");
    }

    filter_job job;
    if (filter_job_init(&job, src, dst, rows, cols, cfg.window, cfg.iters, 1) != 0) {
        die("cannot init filter job");
    }

    int rc = 0;

    if (cfg.bench_max == 0) {
        size_t block = cfg.block != 0 ? cfg.block : auto_block_rows(rows, cfg.threads);
        filter_job_set_block_rows(&job, block);

        if (filter_run(&job, cfg.threads) != 0) {
            die("filter_run failed");
        }
        printf("elapsed=%.6f s\n", job.elapsed);
    } else {
        double *times = (double *)malloc(cfg.repeat * sizeof(*times));
        if (times == NULL) {
            die("out of memory");
        }

        printf("%-5s %-8s %-15s %-15s %-8s %-9s\n",
               "P", "blocks", "T_p best (ms)", "T_p med (ms)", "S(P)", "E(P),%");
        printf("-----------------------------------------------------------\n");

        double base = 0.0;
        for (size_t p = 1; p <= cfg.bench_max; p *= 2) {
            size_t block = cfg.block != 0 ? cfg.block : auto_block_rows(rows, p);
            filter_job_set_block_rows(&job, block);

            for (size_t r = 0; r < cfg.repeat; ++r) {
                if (filter_run(&job, p) != 0) {
                    die("filter_run failed");
                }
                times[r] = job.elapsed;
            }

            double best = 0.0;
            double median = 0.0;
            bench_stats(times, cfg.repeat, &best, &median);
            if (p == 1) {
                base = best;
            }

            double speedup = base / best;
            double efficiency = 100.0 * speedup / (double)p;

            printf("%-5zu %-8zu %-15.3f %-15.3f %-8.3f %-9.2f\n",
                   p, job.total_blocks, best * 1e3, median * 1e3, speedup, efficiency);
        }
        free(times);
    }

    if (rc == 0 && cfg.check) {
        filter_job reference;
        if (filter_job_init(&reference, src, ref, rows, cols, cfg.window, cfg.iters, 1) != 0) {
            die("cannot init filter job");
        }
        if (filter_run_serial(&reference) != 0) {
            die("serial reference failed");
        }
        if (compare_with_reference(dst, ref, rows, cols) != 0) {
            rc = 2;
        }
        filter_job_destroy(&reference);
    }

    if (rc == 0 && cfg.print_matrix) {
        print_matrix("result", dst, rows, cols);
    }

    if (rc == 0 && cfg.save != NULL) {
        matrix result = {dst, rows, cols};
        matrix_save(cfg.save, &result);
    }

    filter_job_destroy(&job);
    free(src);
    free(dst);
    free(ref);
    return rc;
}
