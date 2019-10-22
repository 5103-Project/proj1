#ifndef UTHREAD_H
#define UTHREAD_H

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <list>
#include <map>

#define SECOND 100000
#define STACK_SIZE 4096
#define MICROSEC 1000000

typedef unsigned long address_t;

typedef int Thread_id;

typedef int lock_t;

enum STATE{INIT, READY, WAITING, RUNNING, FINISHED};


class TCB{
public:
     	Thread_id id;
        char* stack;
	int stack_size;
        sigjmp_buf jbuf;
        address_t sp;
        address_t pc;
	void** retval;
};

class Thread{
public:
        TCB* tcb;

	STATE S;

	Thread();

	~Thread();
};


	Thread_id uthread_create(void *(*start_routine)(void *), void *arg);
	
	int uthread_yield();

	int uthread_self(void);

	int uthread_join(int tid, void **retval);
	
	//void test();



	static int Thread_count = 0;

	static std::map<Thread_id, Thread*> Threads;

	//static std::list<TCB*> WaitingList;
	static std::map<TCB*, int> WaitingList;
	// int refers to the thread id the current thread is waiting waiting for.
	// It's set to 0 if it is suspended and waits for resume, because we assume 
	// no one should be waiting for the main thread.
	
	static std::list<TCB*> ReadyList;

	static std::list<TCB*> RunningList;

	static std::list<TCB*> FinishedList;

	//static struct itimerval timer;

	//static struct sigevent evp;

	int uthread_init(int time_slice);

	void context_switch(Thread* t1, Thread* t2);


	int uthread_terminate(int tid);

	int uthread_suspend(int tid);

	int uthread_resume(int tid);

    //static int uthread_self();
// lock and unlock function define here

int lock_init(lock_t* lock);

int acquire(lock_t* lock);

int release(lock_t* lock);

void block();

void unblock();
#endif
