#ifndef THREAD_H
#define THREAD_H

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define SECOND 100000
#define STACK_SIZE 4096


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


#endif
