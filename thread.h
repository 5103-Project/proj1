#ifndef THREAD_H
#define THREAD_H

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <list>

#define SECOND 100000
#define STACK_SIZE 4096
#define MICROSEC 1000000

typedef unsigned long address_t;

typedef int Thread_id;

enum STATE{INIT, READY, WAITING, RUNNING, FINISHED};


class TCB{
public:
     	Thread_id id;
        char* stack;
	int stack_size;
        sigjmp_buf jbuf;
        address_t sp;
        address_t pc;
	sigjmp_buf sjbuf;	
};

class Thread{
public:
        TCB* tcb;

	/*function define here
	 * int uthread_create(void *(*start_routine)(void *), void *arg); // returns tid
	 * int uthread_yield(void); // return value not meaningful
	 * int uthread_self(void); // returns tid
	 * int uthread_join(int tid, void **retval);
	*/
	Thread_id uthread_create(void *(*start_routine)(void *), void *arg);
	
	void test();
};

class uthread{

public:
	static std::list<TCB*> WaitingList;

	static std::list<TCB*> ReadyList;

	static std::list<TCB*> RunningList;

	//static struct itimerval timer;

	//static struct sigevent evp;

	static int uthread_init(int time_slice);



};


#endif
