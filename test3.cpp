#include<iostream>
#include"uthread.h"

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
        printf("running thread is %d,in f (%d)\n",uthread_self(),k);
		if(k==3){
			cout<<"suspend t1"<<endl;
            uthread_suspend(uthread_self());
        }
        if(k == 15){			
			cout<<"Thread of f() is terminated!"<<endl;
			uthread_terminate(uthread_self());
		}
        myPause();
    }
}

void *g(void*)
{
    static int j=0;
    while(1){
        ++j;
        printf("running thread is %d,in g (%d)\n",uthread_self(),j);
		if(j == 30){			
			cout<<"Thread of g() is terminated!"<<endl;
			uthread_terminate(uthread_self());
		}
        myPause();
    }
}



int main()
{
    cout<<"This test case is used to test uthread_suspend, uthread_resume"<<endl;
    //setup();
    uthread_init(2 * MICROSEC);
/*
    Thread *t1 = new Thread();
    t1->uthread_create(f,NULL);
    
    Thread *t2 = new Thread(); 
    t2->uthread_create(g, NULL);
*/
    int t1 = uthread_create(f, NULL);
    int t2 = uthread_create(g, NULL);
    //printForDebug();
    cout<<"suspend t2"<<endl;
    uthread_suspend(t2);
   
    for(int i = 0; i <= 10; i++){
        myPause();
    }
    cout<<"free t1"<<endl;
    uthread_resume(t1);
    cout<<"free t2"<<endl;
    uthread_resume(t2);
    
    
	uthread_join(t1,NULL);
	cout<<"T1 finished! Back to main!"<<endl;
   
	uthread_join(t2,NULL);
	cout<<"T2 finished! Back to main!"<<endl;
    
	while(1){
    	cout<<"back to main\n";
    	myPause();
    }
    return 0;
}
