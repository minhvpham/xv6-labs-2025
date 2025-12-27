#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static void sieve_stage(int in_fd) {
    int prime;
    if (read(in_fd, &prime, sizeof prime) != sizeof prime) {
        close(in_fd);
        exit(0);
    }
    printf("prime %d\n", prime);

    int next_pipe[2];
    pipe(next_pipe);
    int child = fork();
    if (child == 0) {
        close(next_pipe[1]);
        close(in_fd);
        sieve_stage(next_pipe[0]);
        exit(0);
    }

    close(next_pipe[0]);
    int candidate;
    while (read(in_fd, &candidate, sizeof candidate) == sizeof candidate) {
        if (candidate % prime != 0) {
            write(next_pipe[1], &candidate, sizeof candidate);
        }
    }
    close(next_pipe[1]);
    close(in_fd);
    wait(0);
}

int main(int argc, char *argv[]) {
    if (argc != 1) {
        fprintf(2, "usage: primes\n");
        exit(1);
    }

    int first_pipe[2];
    pipe(first_pipe);
    int child = fork();
    if (child == 0) {
        close(first_pipe[1]);
        sieve_stage(first_pipe[0]);
        exit(0);
    }

    close(first_pipe[0]);
    for (int value = 2; value <= 280; value++) {
        write(first_pipe[1], &value, sizeof value);
    }
    close(first_pipe[1]);
    wait(0);
    exit(0);
}