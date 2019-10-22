#include<iostream>
#include"uthread.h"

using namespace std;

/*
 *This test sample used to test lock and unlock
 *In this test case we create two function: f, g
 *function f will increase the variable milk, the maximun number of milk is 1
 * Thus it will stop increase if milk is already 1
 *function f decrease the variable milk, the minimum number of milk is 0
 *
 * we create two thread running f and g seperately.
 * And to see if f will increase when g has not taken milk(milk = 1)
 * and if g will take milk if milk is empty(milk = 0)
*/

void myPause(){
        int t = 1;
        for(int i = 0; i < 500000000; i++){
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

        printf("running thread is in f\n");
        acquire(lock);
        if(milk == 0){
                cout<<"add milk\n";
                milk++;
        }
        printMilkStatus();
        myPause();
        release(lock);
        cout<<"finish add milk\n";
        myPause();
    }
}

void *g(void*)
{
    while(1){
        printf("running thread is in g \n");
        acquire(lock);
        if(milk == 1){
                cout<<"remove milk\n";
                milk--;
        }
        printMilkStatus();
        myPause();
        release(lock);
        cout<<"finish remove milk\n";
        myPause();
    }
}



int main()
{
    cout<<"This test case is used to test lock and unlock"<<endl;
    //setup();
    uthread_init(0.5 * MICROSEC);
/*
    Thread *t1 = new Thread();
    t1->uthread_create(f,NULL);

    Thread *t2 = new Thread();
    t2->uthread_create(g, NULL);
*/

    int t1 = uthread_create(f, NULL);
    int t2 = uthread_create(g, NULL);

    lock = new lock_t();
    lock_init(lock);

    while(1){
            cout<<"back to main thread\n";
            myPause();
    }

    return 0;
}
