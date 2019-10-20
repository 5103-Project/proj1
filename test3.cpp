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
    static int k=0;
    while(1) {
        ++k;
        printf("running thread is %d,in f (%d)\n",uthread::RunningList.front()->id,k);
		if(k==3){
            uthread::uthread_suspend(uthread::uthread_self());
        }
        if(k == 15){			
			cout<<"Thread of f() is terminated!"<<endl;
			uthread::uthread_terminate(uthread::uthread_self());
		}
        myPause();
    }
}

void *g(void*)
{
    static int j=0;
    while(1){
        ++j;
        printf("running thread is %d,in g (%d)\n",uthread::RunningList.front()->id,j);
		if(j == 30){			
			cout<<"Thread of g() is terminated!"<<endl;
			uthread::uthread_terminate(uthread::uthread_self());
		}
        myPause();
    }
}



int main()
{
    cout<<"This test case is used to test uthread_suspend, uthread_resume"<<endl;
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
   // myPause();
    //printForDebug();
   
    uthread::uthread_suspend(t2->tcb->id);
   
    for(int i = 0; i <= 10; i++){
        myPause();
    }
    uthread::uthread_resume(t1->tcb->id);
    uthread::uthread_resume(t2->tcb->id);
    
    
	main_thread->uthread_join(t1->uthread_self(),NULL);
	cout<<"T1 finished! Back to main!"<<endl;
   
	main_thread->uthread_join(t2->uthread_self(),NULL);
	cout<<"T2 finished! Back to main!"<<endl;
    
	while(1){
    	cout<<"back to main\n";
    	myPause();
    }
    return 0;
}