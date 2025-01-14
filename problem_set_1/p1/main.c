#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include "reader.h"
#include "writer.h"

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