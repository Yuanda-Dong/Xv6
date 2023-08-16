#include "kernel/types.h"
#include "user/user.h"
// Your goal is to use pipe and fork to set up the pipeline. The first process
// feeds the numbers 2 through 35 into the pipeline. For each prime number, you
// will arrange to create one process that reads from its left neighbor over a
// pipe and writes to its right neighbor over another pipe. Since xv6 has
// limited number of file descriptors and processes, the first process can stop
// at 35.

//     Be careful to close file descriptors that a process doesn't need, because
//     otherwise your program will run xv6 out of resources before the first
//     process reaches 35. Once the first process reaches 35, it should wait
//     until the entire pipeline terminates, including all children,
//     grandchildren, &c. Thus the main primes process should only exit after
//     all the output has been printed, and after all the other primes processes
//     have exited.
// Hint: read returns zero when the write-side of a pipe is closed.

int main(int argc, char *argv[]) {

    int reading[2]; // for reading from parent
    int writing[2]; // for writing to child

    pipe(writing);
    // I'm the first process
    if (fork() == 0) { // I'm child
        close(writing[1]);
        reading[0] = writing[0];
        pipe(writing);
    } else { // I'm parent
        close(writing[0]);
        for (int i = 2; i <= 35; i++) {
            write(writing[1], &i, 4);
        }
        close(writing[1]);
        wait(0);
        exit(0);
    }
    int p;
jump:
    if (read(reading[0], &p, 4) == 0) {
        close(reading[0]);
        exit(0);
    };
    printf("prime %d\n", p);

    int forked = 0;
    int n;
    while (read(reading[0], &n, 4) != 0) {
        if (n % p != 0 && !forked) {
            forked = 1;
            if (fork() == 0) { // I'm child
                close(reading[0]);
                close(writing[1]);
                reading[0] =
                    writing[0]; // I should read from where my parnet writes
                pipe(writing);
                goto jump;
            } else {
                write(writing[1], &n, 4);
            }
        } else if (n % p != 0 &&
                   forked) { // writing is ok to use since we have forked
            write(writing[1], &n, 4);
        }
    }
    close(reading[0]);
    close(writing[1]);
    wait(0);
    exit(0);
}
