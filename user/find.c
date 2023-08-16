#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// find (moderate)

// Write a simple version of the UNIX find program: find all the files in a directory tree with a specific name. Your solution should be in the file user/find.c.

// Some hints:
//     Look at user/ls.c to see how to read directories.
//     Use recursion to allow find to descend into sub-directories.
//     Don't recurse into "." and "..".
//     Changes to the file system persist across runs of qemu; to get a clean file system run make clean and then make qemu.
//     You'll need to use C strings. Have a look at K&R (the C book), for example Section 5.5.
//     Note that == does not compare strings like in Python. Use strcmp() instead.
// Your solution is correct if produces the following output (when the file system contains the files b, a/b and a/aa/b):
//     $ make qemu
//     ...
//     init: starting sh
//     $ echo > b
//     $ mkdir a
//     $ echo > a/b
//     $ mkdir a/aa
//     $ echo > a/aa/b
//     $ find . b
//     ./b
//     ./a/b
//     ./a/aa/b
//     $

char *fmtname(char *path) {
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
    return buf;
}

void ls(char *path) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    fd = open(path, 0);
    fstat(fd, &st);

    switch (st.type) {
    case T_FILE:
        printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
        break;

    case T_DIR:
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if (stat(buf, &st) < 0) {
                printf("ls: cannot stat %s\n", buf);
                continue;
            }
            printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
        }
        break;
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    int i;

    if (argc < 2) {
        ls(".");
        exit(0);
    }
    for (i = 1; i < argc; i++)
        ls(argv[i]);
    exit(0);
}
