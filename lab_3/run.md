# Сборка
```sh
cmake -S . -B build-linux
cmake --build build-linux
```

# Запуск из каталога с бинарником

```sh
cd build-linux/bin
```

## Ручной ввод матрицы, 4 потока, окно 3, 5 применений

```sh
./lab3 --manual --threads 4 --window 3 --iters 5
```

## Матрица из файла + проверка эталоном

В файле ровно `rows*cols` целых чисел, размеры задаются ключами:

```sh
./lab3 --input matrix.txt --rows 3 --cols 4 --threads 8 --window 3 --iters 4 --check
```

## Сгенерированная матрица

```sh
./lab3 --random --rows 2048 --cols 2048 --threads 8 --iters 10 --check
```

## Исследование для отчёта: таблица T_p, S(P), E(P)

```sh
./lab3  --random --rows 2048 --cols 2048 --iters 10 --bench 16 --repeat 5 --check
```

## Сохранение результата

```sh
./lab3 --input matrix.txt --rows 3 --cols 4 --threads 4 --save out.txt --print-matrix
```