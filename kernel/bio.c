// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 13
struct buf *table[NBUCKET];
struct spinlock locks[NBUCKET];
struct buf buf[10 * NBUF];

void binit() {
    // lock protects the bucket of cached block buffers, the reference count
    // (b->refcnt) in each block buffer, and the identities of the cached blocks
    // (b->dev and b->blockno).

    for (int i = 0; i < NBUCKET; i++) {
        initlock(&locks[i], "bcache");
        table[i] = &buf[i];
    }

    for (int i = NBUCKET; i < 10 * NBUF; i++){
        struct buf *b = table[i%NBUCKET];
        while(b->next){
            b = b->next;
        }
        // b->next is empty
        b->next = &buf[i];
        initsleeplock(&buf[i].lock,"buffer");
    }
}

static struct buf *bget(uint dev, uint blockno){
    struct buf *b;
    acquire(&locks[blockno % NBUCKET]);
    struct buf* bucket = table[blockno % NBUCKET];
    b = bucket;
    // Is the block cached in the bucket ? 
    for (b = bucket; b; b = b->next){
        if (b->dev == dev && b->blockno == blockno){
            b->refcnt ++;
            release(&locks[blockno % NBUCKET]);
            acquiresleep(&b->lock);
            return b;
        }
    }
    // Not cached.
    // Find a buffer in the bucket with refcnt == 0
    // for (b = bu)
    for (b = bucket; b; b = b->next){
        if (b->refcnt == 0) {
            b->dev = dev;
            b->blockno = blockno;
            b->valid = 0;
            b->refcnt = 1;
            release(&locks[blockno % NBUCKET]);
            acquiresleep(&b->lock);
            return b;
        }
    }
    panic("bget: no buffers");

}


// Return a locked buf with the contents of the indicated block.
struct buf *bread(uint dev, uint blockno) {
    struct buf *b;

    b = bget(dev, blockno);
    if (!b->valid) {
        virtio_disk_rw(b, 0);
        b->valid = 1;
    }
    return b;
}

// Write b's contents to disk.  Must be locked.
void bwrite(struct buf *b) {
    if (!holdingsleep(&b->lock))
        panic("bwrite");
    virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void brelse(struct buf *b) {
    if (!holdingsleep(&b->lock))
        panic("brelse");

    releasesleep(&b->lock);

    acquire(&locks[b->blockno % NBUCKET]);
    b->refcnt--;
    release(&locks[b->blockno % NBUCKET]);
}

void bpin(struct buf *b) {
    acquire(&locks[b->blockno % NBUCKET]);
    b->refcnt++;
    release(&locks[b->blockno % NBUCKET]);
}

void bunpin(struct buf *b) {
    acquire(&locks[b->blockno % NBUCKET]);
    b->refcnt--;
    release(&locks[b->blockno % NBUCKET]);
}
