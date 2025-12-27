#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(2, "WRONG COMMAND!!!!!\n");
        exit(1);
    }
    int timer = atoi(argv[1]);
    sleep(timer * 10);
    exit(0);
}