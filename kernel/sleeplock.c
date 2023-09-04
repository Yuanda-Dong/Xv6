// Sleeping locks

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"

void initsleeplock(struct sleeplock *lk, char *name) {
    initlock(&lk->lk, "sleep lock");
    lk->name = name;
    lk->locked = 0;
    lk->pid = 0;
}

void acquiresleep(struct sleeplock *lk) {
    acquire(&lk->lk);
    while (lk->locked) { // checks sleep lock while holding spin lock
        sleep(lk, &lk->lk);
    }
    // holds the spin lock and sleep lock
    lk->locked = 1;
    lk->pid = myproc()->pid;
    release(&lk->lk);
}

void releasesleep(struct sleeplock *lk) {
    acquire(&lk->lk);
    lk->locked = 0;
    lk->pid = 0;
    wakeup(lk);
    release(&lk->lk);
}

int holdingsleep(struct sleeplock *lk) {
    int r;

    acquire(&lk->lk);
    r = lk->locked && (lk->pid == myproc()->pid);
    release(&lk->lk);
    return r;
}
