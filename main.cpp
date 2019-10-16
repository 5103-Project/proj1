#include<iostream>
#include"thread.h"

using namespace std;



void *f(void*)
{
    int i=0;
    while(1) {
        ++i;
        printf("in f (%d)\n",i);
        if (i % 3 == 0) {
            printf("f: switching\n");
        }
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
    Thread *t1 = new Thread();
    //setup();
    uthread::uthread_init(300);
    t1->test();
    //t1->uthread_create(f,NULL);
    //uthread_create(g, NULL);
    while(1);
    return 0;
}

