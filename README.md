# intentional_deadlock
Two programs written in C that intentionally cause a deadlock. One between threads, and the other between processes.

## Environment
* Have GCC with C99 capability (*literally any GCC should work*)

## Building
* Run Either:
1. `gcc deadlock_threads.c -lpthread`
2. `gcc deadlock_processes.c`

## Executing
By default, the generated execuatble from GCC is `a.out`. If that's not there I can't help you.
1. `./a.out`

Enjoy your deadlock!
