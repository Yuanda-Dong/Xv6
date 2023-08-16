#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    int pair1[2]; // parent send to child
    int pair2[2]; // child send to parent
    pipe(pair1);
    pipe(pair2);

    if (fork() == 0){ // i'm child
        read(pair1[0],(void *)0,1);
        printf("%d: received ping\n", getpid());
        write(pair2[1], "goodbye world", 1);

    }else{ // i'm parent
        write(pair1[1],"hello world",1);
        read(pair2[0],(void *)0,1);
        printf("%d: received pong\n", getpid());
    }

    return 0;

}
