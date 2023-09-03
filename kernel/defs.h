struct buf;
struct context;
struct file;
struct inode;
struct pipe;
struct proc;
struct spinlock;
struct sleeplock;
struct stat;
struct superblock;
struct mbuf;
struct sock;

// bio.c
void binit(void);
struct buf *bread(uint dev, uint blockno);
void brelse(struct buf *b);
void bwrite(struct buf *b);
void bpin(struct buf *b);
void bunpin(struct buf *b);

// console.c
void consoleinit(void);
void consoleintr(int c);
void consputc(int c);

// exec.c
int exec(char *path, char **argv);

// file.c
struct file *filealloc(void);
void fileclose(struct file *f);
struct file *filedup(struct file *f);
void fileinit(void);
int fileread(struct file *f, uint64 addr, int n);
int filestat(struct file *f, uint64 addr);
int filewrite(struct file *f, uint64 addr, int n);

// fs.c
void fsinit(int dev);
int dirlink(struct inode *dp, char *name, uint inum);
struct inode *dirlookup(struct inode *dp, char *name, uint *poff);
struct inode *ialloc(uint dev, short type);
struct inode *idup(struct inode *ip);
void iinit();
void ilock(struct inode *ip);
void iput(struct inode *ip);
void iunlock(struct inode *ip);
void iunlockput(struct inode *ip);
void iupdate(struct inode *ip);
int namecmp(const char *s, const char *t);
struct inode *namei(char *path);
struct inode *nameiparent(char *path, char *name);
int readi(struct inode *ip, int user_dst, uint64 dst, uint off, uint n);
void stati(struct inode *ip, struct stat *st);
int writei(struct inode *ip, int user_src, uint64 src, uint off, uint n);
void itrunc(struct inode *ip);

// ramdisk.c
void ramdiskinit(void);
void ramdiskintr(void);
void ramdiskrw(struct buf *b);

// kalloc.c
void *kalloc(void);
void kfree(void *pa);
void kinit(void);

// log.c
void initlog(int dev, struct superblock *sb);
void log_write(struct buf *b);
void begin_op(void);
void end_op(void);

// pipe.c
int pipealloc(struct file **f0, struct file **f1);
void pipeclose(struct pipe *pi, int writable);
int piperead(struct pipe *pi, uint64 addr, int n);
int pipewrite(struct pipe *pi, uint64 addr, int n);

// printf.c
void printf(char *fmt, ...);
void panic(char *s) __attribute__((noreturn));
void printfinit(void);

// proc.c
int cpuid(void);
void exit(int);
int fork(void);
int growproc(int n);
void proc_mapstacks(pagetable_t kpgtbl);
pagetable_t proc_pagetable(struct proc *p);
void proc_freepagetable(pagetable_t pagetable, uint64 sz);
int kill(int pid);
int killed(struct proc *p);
void setkilled(struct proc *p);
struct cpu *mycpu(void);
struct cpu *getmycpu(void);
struct proc *myproc();
void procinit(void);
void scheduler(void) __attribute__((noreturn));
void sched(void);
void sleep(void *chan, struct spinlock *lk);
void userinit(void);
int wait(uint64 addr);
void wakeup(void *chan);
void yield(void);
int either_copyout(int user_dst, uint64 dst, void *src, uint64 len);
int either_copyin(void *dst, int user_src, uint64 src, uint64 len);
void procdump(void);

// swtch.S
void swtch(struct context *old, struct context *new);

// spinlock.c
void acquire(struct spinlock *lk);
int holding(struct spinlock *lk);
void initlock(struct spinlock *lk, char *name);
void release(struct spinlock *lk);
uint64          lockfree_read8(uint64 *addr);
int             lockfree_read4(int *addr);
void push_off(void);
void pop_off(void);

// sleeplock.c
void acquiresleep(struct sleeplock *lk);
void releasesleep(struct sleeplock *lk);
int holdingsleep(struct sleeplock *lk);
void initsleeplock(struct sleeplock *lk, char *name);

// string.c
int memcmp(const void *v1, const void *v2, uint n);
void *memmove(void *dst, const void *src, uint n);
void *memset(void *dst, int c, uint n);
char *safestrcpy(char *s, const char *t, int n);
int strlen(const char *s);
int strncmp(const char *p, const char *q, uint n);
char *strncpy(char *s, const char *t, int n);

// syscall.c
void argint(int n, int *ip);
int argstr(int n, char *buf, int max);
void argaddr(int n, uint64 *ip);
int fetchstr(uint64 addr, char *buf, int max);
int fetchaddr(uint64 addr, uint64 *ip);
void syscall();

// trap.c
extern uint ticks;
void trapinit(void);
void trapinithart(void);
extern struct spinlock tickslock;
void usertrapret(void);

// uart.c
void uartinit(void);
void uartintr(void);
void uartputc(int);
void uartputc_sync(int c);
int uartgetc(void);

// vm.c
void kvminit(void);
void kvminithart(void);
void kvmmap(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, int perm);
int mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa,
             int perm);
pagetable_t uvmcreate(void);
void uvmfirst(pagetable_t pagetable, uchar *src, uint sz);
uint64 uvmalloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz, int xperm);
uint64 uvmdealloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz);
int uvmcopy(pagetable_t old, pagetable_t new, uint64 sz);
void uvmfree(pagetable_t pagetable, uint64 sz);
void uvmunmap(pagetable_t pagetable, uint64 va, uint64 npages, int do_free);
void uvmclear(pagetable_t pagetable, uint64 va);
pte_t *walk(pagetable_t pagetable, uint64 va, int alloc);
uint64 walkaddr(pagetable_t pagetable, uint64 va);
int copyout(pagetable_t pagetable, uint64 dstva, char *src, uint64 len);
int copyin(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len);
int copyinstr(pagetable_t pagetable, char *dst, uint64 srcva, uint64 max);
void add_ref(void *pa);
void dec_ref(void *pa);

// plic.c
void plicinit(void);
void plicinithart(void);
int plic_claim(void);
void plic_complete(int);

// virtio_disk.c
void virtio_disk_init(void);
void virtio_disk_rw(struct buf *b, int write);
void virtio_disk_intr(void);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

// stats.c
void            statsinit(void);
void            statsinc(void);
// sprintf.c
int             snprintf(char*, int, char*, ...);
#ifdef LAB_NET
// pci.c
void            pci_init();
// e1000.c
void            e1000_init(uint32 *);
void            e1000_intr(void);
int             e1000_transmit(struct mbuf*);
// net.c
void            net_rx(struct mbuf*);
void            net_tx_udp(struct mbuf*, uint32, uint16, uint16);
// sysnet.c
void            sockinit(void);
int             sockalloc(struct file **, uint32, uint16, uint16);
void            sockclose(struct sock *);
int             sockread(struct sock *, uint64, int);
int             sockwrite(struct sock *, uint64, int);
void            sockrecvudp(struct mbuf*, uint32, uint16, uint16);
#endif
