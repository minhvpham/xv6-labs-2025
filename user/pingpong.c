#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main(int argc, char* argv[]) {
    if (argc > 1) {
        fprintf(2, "Usage: pingpong...\n");
        exit(1);
    }
    int num;
    int p1[2], p2[2];
    pipe(p1);
    pipe(p2);
    if(fork() == 0){
        while(read(p1[0], &num, sizeof(num)) > 0){
            printf("%d: received ping\n", getpid());
            write(p2[1], &num, sizeof(num));
            close(p1[0]);
            close(p2[1]);
        }
        exit(0);
    }
    else{
        write(p1[1], &num, sizeof(num));
        close(p1[1]);
        while(read(p2[0], &num, sizeof(num)) > 0){
            printf("%d: received pong\n", getpid());
            close(p2[0]);
        }
    }
}