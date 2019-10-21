#include<iostream>
#include"thread.h"

using namespace std;

void myPause(){
	int t = 1;
	for(int i = 0; i < 100000000; i++){
		t *= i;
	}
}

int milk = 0;

lock_t *lock;

void printMilkStatus(){
	if(milk == 0){
		cout<<"milk is empty\n";
	}else{
		cout<<"milk is full"<<milk<<endl;
	}
}

void *f(void*)
{
    while(1) {
	
	acquire(lock);
        printf("running thread is in f\n");
	if(milk == 0){
		cout<<"add milk\n";
		milk++;
	}
	printMilkStatus();
	myPause();
  	release(lock);	
	myPause();
	myPause();
    }
}

void *g(void*)
{
    while(1){
        acquire(lock);
	printf("running thread is in g \n");
	if(milk == 1){
		cout<<"remove milk\n";
		milk--;
	}
	printMilkStatus();
        myPause();
  	release(lock);	
        myPause();
        myPause();
    }
}



int main()
{
    cout<<"This test case is used to test lock and unlock"<<endl;
    //setup();
    uthread::uthread_init(2 * MICROSEC);

    Thread *t1 = new Thread();
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

    lock = new lock_t();
    lock_init(lock);
     
    while(1){
	    cout<<"back to main thread\n";
	    myPause();
    }

    return 0;
}
