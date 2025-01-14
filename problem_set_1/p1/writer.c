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

void *write_func(void *args) {
    pthread_t mythread = pthread_self();

    pthread_mutex_lock(&lock);
    while (resource_counter != 0) {
        pthread_cond_wait(write_phase, &lock);
    }
    resource_counter--;
    pthread_mutex_unlock(&lock);
    
    // ---- enter critical section
    printf("Thread %lu: WROTE X: %c\n", mythread, X);
    printf("Thread %lu: there are %d total readers\n", mythread, resource_counter);
    // ---- exit critical section

    pthread_mutex_lock(&lock);
    resource_counter++;
    // we must check the queue, which is a protected resource, so cannot unlock here
    // we can't actually signal here because it would be a spurious wakeup
    bool signal_readers = true;
    if (reader_queue == 0) {
        signal_readers = false;
    }
    pthread_mutex_unlock(&lock);

    // always broadcast to readers
    pthread_cond_broadcast(read_phase);
    if (signal_readers == 0) {
        // we don't think there are any readers waiting, safe to signal a writer
        pthread_cond_signal(write_phase);
    }

    return NULL;
}