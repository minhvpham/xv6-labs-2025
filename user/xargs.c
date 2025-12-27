#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"
#include "user/user.h"

static char *clone_token(const char *src) {
    int len = strlen(src);
    char *copy = malloc(len + 1);
    if (copy == 0)
        return 0;
    for (int i = 0; i < len; i++)
        copy[i] = src[i];
    copy[len] = 0;
    return copy;
}

int main(int argc, char *argv[]) {
    if (argc < 2)
        exit(0);

    char *args[MAXARG];
    int fixed = argc - 1;
    for (int i = 0; i < fixed; i++)
        args[i] = argv[i + 1];

    char buf[512];
    int len = 0;
    int next = fixed;
    char ch;

    while (read(0, &ch, 1) == 1) {
        int sep = (ch == ' ' || ch == '\n');
        if (!sep) {
            if (len + 1 < sizeof buf)
                buf[len++] = ch;
            continue;
        }

        if (len > 0) {
            buf[len] = 0;
            args[next++] = clone_token(buf);
            len = 0;
        }

        if (ch == '\n') {
            args[next] = 0;
            if (fork() == 0) {
                exec(argv[1], args);
                exit(0);
            }
            wait(0);
            next = fixed;
        }
    }
    exit(0);
}
