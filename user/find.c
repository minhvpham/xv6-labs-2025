#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

#define PATH_MAX_LEN 512

static char *basename_from(const char *path) {
    static char short_name[DIRSIZ + 1];
    const char *tail = path + strlen(path);

    while (tail > path && *(tail - 1) == '/')
        tail--;
    const char *start = tail;
    while (start > path && *(start - 1) != '/')
        start--;

    int len = tail - start;
    if (len >= DIRSIZ)
        return (char *)start;

    memmove(short_name, start, len);
    short_name[len] = 0;
    return short_name;
}

static int should_skip(const char *name) {
    if (name[0] != '.')
        return 0;
    if (name[1] == 0)
        return 1;
    if (name[1] == '.' && name[2] == 0)
        return 1;
    return 1;
}

static void emit_match_if_needed(const char *path, const char *target) {
    if (strcmp(basename_from(path), target) == 0)
        printf("%s\n", path);
}

static void search_tree(const char *path, const char *needle) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(2, "cannot open %s\n", path);
        return;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        fprintf(2, "cannot stat %s\n", path);
        close(fd);
        return;
    }

    if (st.type == T_FILE || st.type == T_DEVICE) {
        emit_match_if_needed(path, needle);
        close(fd);
        return;
    }

    if (st.type != T_DIR) {
        close(fd);
        return;
    }

    if (strlen(path) + 1 + DIRSIZ + 1 > PATH_MAX_LEN) {
        printf("path too long\n");
        close(fd);
        return;
    }

    struct dirent de;
    int base_len = strlen(path);
    char child[PATH_MAX_LEN];

    memmove(child, path, base_len);
    child[base_len] = '/';

    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        if (de.inum == 0)
            continue;

        memmove(child + base_len + 1, de.name, DIRSIZ);
        child[base_len + 1 + DIRSIZ] = 0;

        char *leaf = child + base_len + 1;
        if (should_skip(leaf))
            continue;

        if (stat(child, &st) < 0) {
            printf("cannot stat %s\n", child);
            continue;
        }
        search_tree(child, needle);
    }

    close(fd);
}
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(2, "usage: find path name\n");
        exit(1);
    }

    search_tree(argv[1], argv[2]);
    exit(0);
}