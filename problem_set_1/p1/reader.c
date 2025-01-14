#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

// TODO: make these arguments instead of globals, or make this a struct
extern int resource_counter;  // convention is positive=readers, negative=writer(s)
extern int reader_queue;
extern pthread_mutex_t lock;  // protects access to resource_counter, reader_queue
extern pthread_cond_t *read_phase;  // condition signalling ok for reader to get lock
extern pthread_cond_t *write_phase;  // condition signalling ok for writer to get lock
extern char X;

void *read_func(void *args) {
    // READERS HAVE PRIO
    pthread_t mythread = pthread_self();

    // request permission to read. Once granted, increment reader counter
    pthread_mutex_lock(&lock);
    while (resource_counter < 0) {
        reader_queue++;
        pthread_cond_wait(read_phase, &lock);
        reader_queue--;
    }
    resource_counter++;
    pthread_mutex_unlock(&lock);

    //  ---- enter critical section
    printf("Thread %lu: READ X: %c\n", mythread, X);
    printf("Thread %lu: there are %d total readers\n", mythread, resource_counter);

    // hang out here a while to prove other readers seeing me
    sleep(1);
    //  ---- exit critical section

    // decrement reader counter
    pthread_mutex_lock(&lock);
    resource_counter--;
    if (resource_counter == 0 && reader_queue == 0) {
        // only signal to writers if there are NO readers active or waiting
        // no need to signal to readers
        pthread_cond_signal(write_phase);
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}