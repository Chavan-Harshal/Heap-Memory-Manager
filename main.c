#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stddef.h>
#include "HMM.h"

// Driver function to test codes
int main(){

    typedef struct p{
        int a;
        char c;
        float d;
        struct p *next;
    }p;

    int *arr;
    p *ptr;

    init("int", sizeof(int));
    init("p", sizeof(p));

    arr = mycalloc("int", 10);
    ptr = mycalloc("p", 1);
    print_usage();
    printf("\n\nStarting address of int array : %p\n", arr);
    ptr->a = 5;
    ptr->c = 'a';
    ptr->d = 5.5;

    printf("\nContents of structure : a = %d   c = %c  d = %f\n", ptr->a, ptr->c, ptr->d);

    myfree((void*)&arr);
    if(!arr)
        printf("\nArr deallocated\n");

    myfree((void*)&ptr);
    if(!ptr)
        printf("Ptr deallocated\n");

    print_usage();

    return 0;
}
