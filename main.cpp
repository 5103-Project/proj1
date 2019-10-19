#include<iostream>
#include"thread.h"

using namespace std;


void myPause(){
	int t = 1;
	for(int i = 0; i < 100000000; i++){
		t *= i;
	}
}

void *f(void*)
{
    int i=0;
    while(1) {
        ++i;
        printf("in f (%d)\n",i);
        myPause();
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
        myPause();
    }
}



int main()
{
    Thread *t1 = new Thread();
    //setup();
    uthread::uthread_init(2 * MICROSEC);
    t1->uthread_create(f,NULL);
    
    Thread *t2 = new Thread(); 
    t2->uthread_create(g, NULL);

    // main thread
    Thread* main_thread = new Thread();

    main_thread->S = RUNNING;
    uthread::RunningList.push_back(main_thread->tcb);
    uthread::Threads[0] = main_thread;

    main_thread->tcb->id = 0;
    sigsetjmp(main_thread->tcb->jbuf,1);
 

    while(1){
    	cout<<"back to main\n";
    	myPause();
    }

    return 0;
}

