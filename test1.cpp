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
            printf("g: yield\n");
	    uthread_yield();
        }
        myPause();
    }
}



int main()
{
    cout<<"This test case is used to test uthread_init, uthread_create,"<<endl;
    cout<<"uthread_yeild, uthread_self, scheduler, time slcing and basic context swtich."<<endl;
    //setup();
    uthread_init(2 * MICROSEC);

    //Thread *t1 = new Thread();
    //t1->uthread_create(f,NULL);
    
    //Thread *t2 = new Thread(); 
    //t2->uthread_create(g, NULL);

    int t1 = uthread_create(f, NULL);
    int t2 = uthread_create(g, NULL);

    while(1){
    	cout<<"back to main\n";
    	myPause();
    }

    return 0;
}
