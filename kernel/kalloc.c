// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
    struct run *next;
};

struct {
    struct spinlock lock;
    struct run *freelist;
} kmemA[NCPU];

void kinit() {
    uint64 ke = PGROUNDUP((uint64)end);
    uint64 slice = (PHYSTOP - ke) / NCPU;
    for (int i = 0; i < NCPU; i++) {
        initlock(&kmemA[i].lock, "kmem");
        freerange((void *)(ke + i * slice), (void *)(ke + (i + 1) * slice));
    }
}

void freerange(void *pa_start, void *pa_end) {
    char *p;
    p = (char *)PGROUNDUP((uint64)pa_start);
    for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
        kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void *pa) {
    struct run *r;

    if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
        panic("kfree");

    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run *)pa;

    push_off();
    int cpu_id = cpuid();

    acquire(&kmemA[cpu_id].lock);
    r->next = kmemA[cpu_id].freelist;
    kmemA[cpu_id].freelist = r;
    release(&kmemA[cpu_id].lock);

    pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *kalloc(void) {
    struct run *r;

    push_off();
    int cpu_id = cpuid();

    acquire(&kmemA[cpu_id].lock);
    r = kmemA[cpu_id].freelist;
    if (r){
        kmemA[cpu_id].freelist = r->next;
        release(&kmemA[cpu_id].lock);
        pop_off();
        return (void *)r;
    }
    release(&kmemA[cpu_id].lock);

    // steal memory from other cpu's freelist
    for (int i = 0; i < NCPU && !r; ++i) {
        if (cpu_id == i)
            continue;
        acquire(&kmemA[i].lock);
        r = kmemA[i].freelist;
        if (r)
            kmemA[i].freelist = r->next;
        release(&kmemA[i].lock);
    }

    if (r)
        memset((char *)r, 5, PGSIZE); // fill with junk
    else{
        pop_off();
        return 0;
    }

    pop_off();
    return (void *)r;
}
