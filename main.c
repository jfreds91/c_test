#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

// https://stackoverflow.com/questions/37538/how-do-i-determine-the-size-of-my-array-in-c
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))
#define NUM_READERS 5
#define NUM_WRITERS 5
#define RUN_X_TIMES 10

int resource_counter = 0;  // convention is positive=readers, negative=writer(s)
int reader_queue = 0;
pthread_mutex_t lock;  // protects access to resource_counter, reader_queue
pthread_cond_t *read_phase;  // condition signalling ok for reader to get lock
pthread_cond_t *write_phase;  // condition signalling ok for writer to get lock

/*
Prompt requests a global variable to read/write.
This is written to the data part of the program address space, and is outside of the
execution context of any thread. We could have instead written this to the heap, if
we wanted a variable we could actually dynamically write to
*/
char X = 'X';

typedef void *(*function_t)(void *);

typedef struct {
    int n;
    function_t func;
} function_runner_t;


void *do_n_times_with_delay(void *args) {
    /*
    Executes a function n times with a delay before each one
    Args:
        (function_runner_t *) args
            ->n (int): number of times to loop
            ->func (callable): pointer to a function which takes a void pointer and returns a void pointer
    Returns:
        (void *) NULL
    */
    function_runner_t *input = (function_runner_t *) args;  // inform compiler this is a pointer to a struct
    int seconds;
    for (int i = 0; i < input->n; i++) {
        seconds = rand() % 5 + 1;
        sleep(seconds);
        input->func(NULL);
    }
    return NULL;
}

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

int main(int argc, char* argv[]) {
    srand(time(NULL));  // set random seed

    // reserve heap space for conditions since they were defined as pointers
    read_phase = malloc(sizeof(pthread_cond_t));
    write_phase = malloc(sizeof(pthread_cond_t));
    
    // init mutex + conditions
    if (pthread_mutex_init(&lock, NULL) != 0) {
        return 5;
    }
    if (pthread_cond_init(read_phase, NULL) != 0) {
        return 6;
    }
    if (pthread_cond_init(write_phase, NULL) != 0) {
        return 7;
    }

    // create reader threads
    pthread_t reader_threads[NUM_READERS];
    function_runner_t reader_args;
    reader_args.n = RUN_X_TIMES;
    reader_args.func = &read_func;

    for (int i=0; i < NELEMS(reader_threads); i++) {
        // pthread_create takes a pthread_t ADDRESS
        if (pthread_create(&reader_threads[i], NULL, &do_n_times_with_delay, &reader_args) != 0) {
            return 1;
        }
    }
    printf("Done spawning reader threads\n");
    
    // create writer threads
    pthread_t writer_threads[NUM_WRITERS];
    function_runner_t writer_args;
    writer_args.n = RUN_X_TIMES;
    writer_args.func = &write_func;
    for (int i = 0; i < NELEMS(writer_threads); i++) {
        if (pthread_create(&writer_threads[i], NULL, &do_n_times_with_delay, &writer_args) != 0) {
            return 2;
        }
    }

    // Join reader threads
    for (int i = 0; i < NELEMS(reader_threads); i++) {
        // TODO: upgrade to nonblocking join
        // pthread_join takes a pthread_t VALUE
        if (pthread_join(reader_threads[i], NULL) != 0) {
            return 3;
        }
    }
    printf("Done joining reader threads\n");

    // Join writer threads
    for (int i = 0; i < NELEMS(writer_threads); i++) {
        // TODO: upgrade to nonblocking join
        if (pthread_join(writer_threads[i], NULL) != 0) {
            return 4;
        }
    }
    printf("Done joining writer threads\n");

    free(read_phase);
    free(write_phase);

    return 0;
}