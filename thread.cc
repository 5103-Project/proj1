#include "thread.h"
#include <iostream>

using namespace std;

#ifdef __x86_64__


/* code for 64 bit Intel arch */


#define JB_SP 6
#define JB_PC 7


/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
        address_t ret;
        asm volatile("xor    %%fs:0x30,%0\n"
                        "rol    $0x11,%0\n"
                        : "=g" (ret)
                          : "0" (addr));
        return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
        address_t ret;
        asm volatile("xor    %%gs:0x18,%0\n"
                        "rol    $0x9,%0\n"
                        : "=g" (ret)
                          : "0" (addr));
        return ret;
}

#endif

int TAS(volatile int *addr, int newval){
    int result = newval;
    asm volatile("lock; xchg %0, %1"
                 : "+m" (*addr), "=r" (result)
                 : "1" (newval)
                 : "cc");
    return result;

}


/* code start here*/

#define SUCCESS 0
#define FAIL -1

struct itimerval timer;

struct sigevent evp;

struct sigaction sa = {0};

sigset_t sigsetBlock;

Thread_id uthread::Thread_ID;

std::map<Thread_id, Thread*> uthread::Threads;

std::list<TCB*> uthread::WaitingList;

std::list<TCB*> uthread::ReadyList;

std::list<TCB*> uthread::RunningList;

std::list<TCB*> uthread::FinishedList;

static void contextSwitch(Thread t1, Thread t2);

void block(){
        if(sigprocmask(SIG_BLOCK, &sigsetBlock, NULL) < 0){
                std::cerr<<"block signal failed\n";
                exit(1);
        }
}

void unblock(){
        if(sigprocmask(SIG_UNBLOCK, &sigsetBlock, NULL) < 0){
                std::cerr<<"unblock signal failed\n";
                exit(1);

        }
}


Thread::Thread(){
	this->tcb = new TCB();
	this->tcb->stack = new char[STACK_SIZE];
	this->tcb->stack_size = STACK_SIZE;
}

Thread_id Thread::uthread_create(void *(*start_routine)(void *), void *arg){
	//block();
	cout<<"create thread"<<endl;
        this->tcb->sp = (address_t)tcb->stack + STACK_SIZE - sizeof(int);
        this->tcb->pc = (address_t)(start_routine);


        //stack setup
        //int temp_arg = *((int*)arg);
        //(tcb->sp) = temp_arg;
        //tcb->sp -= sizeof(int);
        //tcb->sp = (address_t)(start_routine);
        //tcb->sp -= sizeof(int);


        sigsetjmp(tcb->jbuf,1);
        //(tcb->jbuf->__jmpbuf)[JB_SP] = translate_address(tcb->sp);
        //(tcb->jbuf->__jmpbuf)[JB_PC] = translate_address(tcb->pc);
        //sigemptyset(&tcb->jbuf->__saved_mask);
        //siglongjmp(tcb->sjbuf,1);
	
	uthread::ReadyList.push_back(this->tcb);
	uthread::Thread_ID++;
	uthread::Threads[uthread::Thread_ID] = this;
	this->tcb->id = uthread::Thread_ID;
	//unblock();
	return uthread::Thread_ID;
	//return 0;
}


void fool(){
	cout<<"fool\n";
}

// uthread class functions' defination here

void handler(int sig){
	
	Thread* T_from, *T_to;
	//block();
	cout<<"there is handler\n";
	
	//jmp_buf env;
	//sigsetjmp(env,1);
	
	TCB* running_thread_TCB = uthread::RunningList.front();	
	Thread* running_thread = uthread::Threads[running_thread_TCB->id];
	cout<<"yield from a running thread "<<running_thread_TCB->id<<"\n";
	
	// used for debugging
	cout<<"There are the current threads we have:"<<endl;
        for (int i=0;i<uthread::Threads.size();i++){
                cout<<uthread::Threads[i]->tcb->id<<endl;
        }
	// 

	running_thread->uthread_yield();
	
	//siglongjmp(env, 1);	
	//sigaddset(&sigsetBlock, SIGALRM);
	//unblock();
	for (int i=0;i<uthread::Threads.size();i++){
		cout<<uthread::Threads[i]<<endl;
	}
}

int uthread::uthread_init(int time_slice){
	cout<<"initialize user level thread:\n";
	uthread::Thread_ID = 0;
	if(time_slice < 0){
		std::cerr<<"time slice should be a postive value!\n";
		return FAIL;
	}

	// main thread
	
	Thread* main_thread = new Thread();
	main_thread->S = RUNNING;
	uthread::RunningList.push_back(main_thread->tcb);
	uthread::Threads[0] = main_thread;

    	//(jbuf[0]->__jmpbuf)[JB_SP] = translate_address(sp);
    	//(jbuf[0]->__jmpbuf)[JB_PC] = translate_address(pc);
   	//sigemptyset(&jbuf[0]->__saved_mask);     
        //cout<<"create thread"<<endl;
        //main_thread->tcb->sp = (address_t)tcb->stack + STACK_SIZE - sizeof(int);
        //this->tcb->pc = (address_t)(start_routine);


	main_thread->tcb->id = 0;

	timer.it_value.tv_sec = time_slice / MICROSEC;
	timer.it_value.tv_usec = time_slice % MICROSEC;
	timer.it_interval.tv_sec = time_slice / MICROSEC;
	timer.it_interval.tv_usec = time_slice % MICROSEC;
        
	
	if(setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1){
                std::cerr<<"timer set up failed!\n";
                exit(-1);
        }
	// The setitimer function counts down towards zero if the
	// process is executed and delivers SIGVTALRM upon expiration.



	sa.sa_handler = &handler;
	
	if (sigaction(SIGVTALRM, &sa, nullptr) < 0) {
        	std::cerr << "system error: sigaction error."<< std::endl;
        	exit(1);
	}
	// Receive SIGVTALRM and call the handler function of sa.

	if(sigemptyset(&sigsetBlock) < 0){
		std::cerr << "sigset error.\n";
	}

	if (sigaddset(&sigsetBlock, SIGALRM) < 0) {
        	std::cerr << "system error: sigset error."<< std::endl;
        	exit(1);
	}

	return SUCCESS;

}

int Thread::uthread_yield(){
	TCB* current_TCB, *next_TCB;
	current_TCB = this->tcb;

	if(uthread::ReadyList.size() > 0){
		next_TCB = uthread::ReadyList.front();

		cout<<current_TCB->id<<'\t'<<next_TCB->id<<endl;

		Thread* current_thread = uthread::Threads[current_TCB->id];
		Thread* next_thread = uthread::Threads[next_TCB->id]; 
		this->S = READY;
		
		uthread::RunningList.pop_front();
		uthread::ReadyList.push_back(current_TCB);
		uthread::ReadyList.pop_front();
		uthread::RunningList.push_back(next_TCB);
		
		uthread::Threads[next_TCB->id]->S = RUNNING;
		uthread::context_switch(current_thread, next_thread);

		
	}else{
		// resume the running thread

		this->S = RUNNING;
	}
	
	//delete all finished thread
	/*
	for(auto finished_t : uthread::FinishedList){
		;
	}
	*/	
}
int Thread::uthread_self(void){

	return this->tcb->id;

}

int Thread::uthread_join(int tid, void **retval){

	Thread* target = uthread::Threads[tid];
	while (target->S != FINISHED){
		this->uthread_yield();
	}
	if (retval != NULL){
		// UNFINISHED
	}
	return 0;


}
	
void uthread::context_switch(Thread* t1, Thread* t2){
	//block();
	//t1 = uthread::Threads[1];
	cout<<"switch from "<<t1->tcb->id<<'\t'<<"to \t"<<t2->tcb->id;
	if(t1 != NULL){
		cout<<" save context\n";
		sigsetjmp(t1->tcb->jbuf,1);
	}
	//unblock();
	siglongjmp(t2->tcb->jbuf,1);
}

int uthread::uthread_terminate(int tid){

	Thread* target = uthread::Threads[tid];
	//TCB* tcb = target->tcb;
	//delete [] tcb->stack;
	//delete tcb;
	target->S = FINISHED;
	uthread::FinishedList.push_back(target->tcb);
	return 0;
}




