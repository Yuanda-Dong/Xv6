#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64 sys_exit(void) {
    int n;
    argint(0, &n);
    exit(n);
    return 0; // not reached
}

uint64 sys_getpid(void) { return myproc()->pid; }

uint64 sys_fork(void) { return fork(); }

uint64 sys_wait(void) {
    uint64 p;
    argaddr(0, &p);
    return wait(p);
}

uint64 sys_sbrk(void) {
    uint64 addr;
    int n;

    argint(0, &n);
    addr = myproc()->sz;
    if (growproc(n) < 0)
        return -1;
    return addr;
}

uint64 sys_sleep(void) {
    int n;
    uint ticks0;

    argint(0, &n);
    acquire(&tickslock);
    ticks0 = ticks;
    while (ticks - ticks0 < n) {
        if (killed(myproc())) {
            release(&tickslock);
            return -1;
        }
        sleep(&ticks, &tickslock);
    }
    release(&tickslock);
    return 0;
}

// Finally, it takes a user address to a
// buffer to store the results into a bitmask (a datastructure that uses one bit
// per page and where the first page corresponds to the least significant bit).

// Some hints:
// - For the output bitmask, it's easier to store a temporary buffer in the
// kernel and copy it to the user (via copyout()) after filling it with the
// right bits.
// - It's okay to set an upper limit on the number of pages that can be scanned.
// walk() in kernel/vm.c is very useful for finding the right PTEs.
// - You'll need to define PTE_A, the access bit, in kernel/riscv.h. Consult the
// RISC-V privileged architecture manual to determine its value.
// - Be sure to clear PTE_A after checking if it is set. Otherwise, it won't be
// possible to determine if the page was accessed since the last time pgaccess()
// was called (i.e., the bit will be set forever).
// - vmprint() may come in handy to debug page tables.

#ifdef LAB_PGTBL
int sys_pgaccess(void) {
    struct proc *p = myproc();
    uint64 start; argaddr(0, &start); // the start of virtual address
    int n; argint(1, &n); // number of pages to check
    uint64 dest; argaddr(2, &dest); // destination
    unsigned int bitmask = 0;
    for (int i = 0; i < n; i++){
        uint64 va = start + i * PGSIZE;
        pte_t* pte = walk(p->pagetable, va, 0);
        if (*pte & 1UL<<6){
            bitmask |= 1UL <<i;
        }
        *pte &= ~(1UL<<6);
    }
    copyout(p->pagetable, dest, (void *) &bitmask, sizeof(bitmask));

    return 0;
}
#endif

uint64 sys_kill(void) {
    int pid;

    argint(0, &pid);
    return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64 sys_uptime(void) {
    uint xticks;

    acquire(&tickslock);
    xticks = ticks;
    release(&tickslock);
    return xticks;
}

// Some garbage collectors (a form of automatic memory management) can benefit
// from information about which pages have been accessed (read or write). In
// this part of the lab, you will add a new feature to xv6 that detects and
// reports this information to userspace by inspecting the access bits in the
// RISC-V page table. The RISC-V hardware page walker marks these bits in the
// PTE whenever it resolves a TLB miss.

// You will receive full credit for this part of the lab if the pgaccess test
// case passes when running pgtbltest.
