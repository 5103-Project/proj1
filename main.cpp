#include<iostream>
#include"thread.h"

using namespace std;



void *f(void*)
{
    int i=0;
    while(1) {
        ++i;
        printf("in f (%d)\n",i);
        sleep(1);
    }
}

void *g(void*)
{
    int i=0;
    while(1){
        ++i;
        printf("in g (%d)\n",i);
        if (i % 5 == 0) {
            printf("g: switching\n");
        }
        sleep(1);
    }
}



int main()
{
    //setup();
    uthread::uthread_init(2 * MICROSEC);
    
    Thread *t1 = new Thread();
    t1->uthread_create(f,NULL);
    
    // "fake" test case for termination
    // sleep(5);
    //uthread::uthread_terminate(t1->tcb->id);

    Thread *t2 = new Thread(); 
    t2->uthread_create(g, NULL);
    while(1);
    return 0;
}

