#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"
// Write a simple version of the UNIX xargs program: its arguments describe a
// command to run, it reads lines from the standard input, and it runs the
// command for each line, appending the line to the command's arguments. Your
// solution should be in the file user/xargs.c.

// some hints:
//     Use fork and exec to invoke the command on each line of input. Use wait
//     in the parent to wait for the child to complete the command. To read
//     individual lines of input, read a character at a time until a newline
//     ('\n') appears. kernel/param.h declares MAXARG, which may be useful if
//     you need to declare an argv array.

int readline(char *buffer, char **arguments) {
    int cur = 0;
    int arg_cur = 0;
    int start = 0;
    while (read(0, &buffer[cur], 1)) {
        if (buffer[cur] == ' ') {
            buffer[cur] = 0;
            arguments[arg_cur++] = &buffer[start];
            start = cur + 1;
        }
        if (buffer[cur] == '\n') {
            buffer[cur] = 0;
            arguments[arg_cur++] = &buffer[start];
            start = cur + 1;
            return 0;
        }
        cur += 1;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    char buffer[512];
    char command[16] = {'.', '/'};
    char *agruments[MAXARG];
    strcpy(command + 2, argv[1]);
    memcpy((void *)agruments, argv + 1,
           (argc - 1) * 8); // here we have copied the arguments on RHS to xargs
    while (readline(buffer, agruments + argc - 1) == 0) {
        // We need to append the arguments from LHS to arguments, LHS is buffer
        // which is a string, we need to strsplit on buffer
        if (fork() == 0) { // child
            if (exec(command, agruments)) {
                printf("exec error\n");
            }
        } else {
            wait(0);
        }
    }
    return 0;
}
