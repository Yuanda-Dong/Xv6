// Mutual exclusion lock.
struct spinlock {
    uint locked; // Is the lock held?, 0 if lock is available, != 0 when it is held

    // For debugging:
    char *name;      // Name of lock.
    struct cpu *cpu; // The cpu holding the lock.
};
