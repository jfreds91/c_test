// compile with:  gcc thread_test.c -o thread_test -Wall -pthread
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>

typedef struct
{
    int sides;
} myargs_t;

typedef struct
{
    int value;
} myret_t;


void *roll_dice(void *args) {
    myargs_t *myargs = (myargs_t *) args;  // cast args back to myargs struct
    
    myret_t *res_ptr = malloc(sizeof(myret_t));  // reserve space on heap for return because stack values will be wiped when thread returns
    res_ptr->value = (rand() % myargs->sides) + 1;  // -> syntax dereferences for us

    printf("Thread: return stored at %p\n", res_ptr);

    return (void *) res_ptr;  // cast to void pointer in order to return
}

int main(int argc, char* argv[]) {
    srand(time(NULL));  // set random seed

    pthread_t th;
    myret_t *result_ptr;
    int sides = 6;

    if (pthread_create(&th, NULL, &roll_dice, &sides) != 0) {
        // failed to create thread
        return 1;
    }
    if (pthread_join(th, (void **) &result_ptr) != 0) {
        // failed to join thread
        return 2;
    }

    printf("Main: return stored at %p\n", result_ptr);
    printf("Main: diceroll was %d\n", result_ptr->value);

    return 0;
}