struct buf {
    int valid; // has data been read from disk?
    int disk;  // does disk "own" buf? if disk own buf, it can write to it
    uint dev; // device 
    uint blockno; // block number
    struct sleeplock lock; //lock
    uint refcnt;
    struct buf *prev; // LRU cache list
    struct buf *next;
    uchar data[BSIZE];
};
