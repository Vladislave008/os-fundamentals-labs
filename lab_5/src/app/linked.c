#include "lab5.h"
#include "command_loop.h"

int main(void)
{
    Lab5Functions functions = { PrimeCount, GCF };
    return run_commands(&functions, 0, 0);
}
