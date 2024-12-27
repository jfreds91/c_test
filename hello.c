#include <stdio.h>

#define N_ROWS 2
#define N_COLS 3


int main()
{
    int *ptr1, *ptr2, x[N_ROWS][N_COLS] = {{1,2,3},{4,5,6}};
    ptr1 = &x[0][0];
    ptr2 = &x[1][0];
    printf("Address: %p\t Value: %d\n", (void *) ptr1, *ptr1);
    printf("Address: %p\t Value: %d\n", (void *) ptr2, *ptr2);
    
    return 0;
}