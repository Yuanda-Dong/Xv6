#define NBUCKET 5
#define NKEYS 100000

struct entry {
    int key;
    int value;
    struct entry *next;
};

struct entry *table[NBUCKET];
int keys[NKEYS];
int nthread = 1;

static void insert(int key, int value, struct entry **p, struct entry *n) {
    struct entry *e = malloc(sizeof(struct entry));
    e->key = key;
    e->value = value;
    e->next = n;
    *p = e;
}

static void put(int key, int value) {
    int i = key % NBUCKET; // decide which buckect to put

    // is the key already present?
    struct entry *e = 0;
    for (e = table[i]; e != 0; e = e->next) { // iterating over the bucket
        if (e->key == key)
            break;
    }
    if (e) {
        // update the existing key.
        e->value = value;
    } else {
        // the key is new.
        pthread_mutex_lock(&lock);
        insert(key, value, &table[i], table[i]);
        pthread_mutex_unlock(&lock);
    }
}

static struct entry *get(int key) {
    int i = key % NBUCKET;

    struct entry *e = 0;
    for (e = table[i]; e != 0; e = e->next) {
        if (e->key == key)
            break;
    }

    return e;
}
