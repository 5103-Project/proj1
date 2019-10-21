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
typedef int lock_t;


struct itimerval timer;

struct sigevent evp;

struct sigaction sa = {0};

struct sigaction sa_suspend = {0};

sigset_t sigsetBlock;

Thread_id uthread::Thread_ID;

std::map<Thread_id, Thread*> uthread::Threads;

//std::list<TCB*> uthread::WaitingList;

std::map<TCB*, int> uthread::WaitingList;

std::list<TCB*> uthread::ReadyList;

std::list<TCB*> uthread::RunningList;

std::list<TCB*> uthread::FinishedList;

static void contextSwitch(Thread t1, Thread t2);

void printForDebug();

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
	block();
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
        (tcb->jbuf->__jmpbuf)[JB_SP] = translate_address(tcb->sp);
        (tcb->jbuf->__jmpbuf)[JB_PC] = translate_address(tcb->pc);
        sigemptyset(&tcb->jbuf->__saved_mask);
        //siglongjmp(tcb->sjbuf,1);
	
	uthread::ReadyList.push_back(this->tcb);
	uthread::Thread_ID++;
	uthread::Threads[uthread::Thread_ID] = this;
	this->tcb->id = uthread::Thread_ID;
	unblock();
	return uthread::Thread_ID;
	//return 0;
}


void fool(int sig){
	cout<<"fool\n";
}

// uthread class functions' defination here

void handler(int sig){
	
	Thread* T_from, *T_to;
	block();
	cout<<"there is handler\n";
	
	TCB* running_thread_TCB = uthread::RunningList.front();	
	Thread* running_thread = uthread::Threads[running_thread_TCB->id];
	cout<<"yield from a running thread\n";
	running_thread->uthread_yield();
	
	//sigaddset(&sigsetBlock, SIGALRM);
	unblock();
	
}

int uthread::uthread_init(int time_slice){
        cout<<"initialize user level thread:\n";
        uthread::Thread_ID = 0;
        if(time_slice < 0){
                std::cerr<<"time slice should be a postive value!\n";
                return FAIL;
        }
/*
        // main thread
        Thread* main_thread = new Thread();
        sigsetjmp(main_thread->tcb->jbuf,1);

        main_thread->S = RUNNING;
        uthread::RunningList.push_back(main_thread->tcb);
        uthread::Threads[0] = main_thread;

        main_thread->tcb->id = 0;
*/


        timer.it_value.tv_sec = time_slice / MICROSEC;
        timer.it_value.tv_usec = time_slice % MICROSEC;
        timer.it_interval.tv_sec = time_slice / MICROSEC;
        timer.it_interval.tv_usec = time_slice % MICROSEC;


        if(setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1){
                std::cerr<<"timer set up failed!\n";
                exit(-1);
        }



        sa.sa_handler = &handler;
        //sa_suspend.sa_handler = &handler;

/*
        if (sigaction(SIGINT, &sa_suspend, nullptr) < 0) {
                std::cerr << "system error: sigaction error."<< std::endl;
                exit(1);
        }
*/
        if (sigaction(SIGVTALRM, &sa, nullptr) < 0) {
        //if (sigaction(SIGINT, &sa, nullptr) < 0) {
                 std::cerr << "system error: sigaction error."<< std::endl;
                exit(1);
        }

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
		this->S = WAITING;
		uthread::WaitingList[this->tcb] = tid;
		//this->uthread_yield();
		// find the next thread and do context switch
		if (uthread::ReadyList.size()>0){
			TCB* next_TCB = uthread::ReadyList.front();
			Thread* next_thread = uthread::Threads[next_TCB->id];
			uthread::ReadyList.pop_front();
			uthread::RunningList.push_back(next_TCB);
			uthread::Threads[next_TCB->id]->S = RUNNING;
			uthread::context_switch(target, next_thread);	
		}
		else {
			cerr<<"this is last thread!"<<endl;
			return -1;
		}

	}
	if (retval != NULL){
		*retval = *target->tcb->retval;
	}

	return 0;


}


void uthread::context_switch(Thread* t1, Thread* t2){
	block();
	cout<<"switch from "<<t1->tcb->id<<" to "<<t2->tcb->id;
	if(t1 != NULL){
		cout<<" save context\n";
		
		if(sigsetjmp(t1->tcb->jbuf,1)){
			return;	
		}
		
		//sigsetjmp(t1->tcb->jbuf, 1);
	}
	unblock();
	siglongjmp(t2->tcb->jbuf,1);
	
}

int uthread::uthread_terminate(int tid){

	Thread* target = uthread::Threads[tid];
	//TCB* tcb = target->tcb;
	//delete [] tcb->stack;
	//delete tcb;
	if (target->S == READY){
		uthread::ReadyList.remove(target->tcb);
	}
	else if (target->S == WAITING){
		//uthread::WaitingList.remove(target->tcb);
		std::map<TCB*,int>::iterator it;
		it = uthread::WaitingList.find(target->tcb);
		if (it != uthread::WaitingList.end()){
			uthread::WaitingList.erase(it);
		}
	}
	else if (target->S == RUNNING){
		uthread::RunningList.remove(target->tcb);
	}
	else {
		cerr<<"the thread has already been terminated!"<<endl;
		return -1;
	}
	//target->S = FINISHED;
	
	// signal to the thread waiting for it
	for (std::map<TCB*,int>::iterator it=uthread::WaitingList.begin(); it!=uthread::WaitingList.end(); ++it){
		if (it->second == tid){
			//TCB* tcb = it->first;
			Thread* thread = uthread::Threads[it->first->id];
			thread->S = READY;
			uthread::ReadyList.push_back(it->first);
			uthread::WaitingList.erase(it);
		}
	
	}
	// De-allocation may need to be done
	

	uthread::FinishedList.push_back(target->tcb);
	if (target->S == RUNNING){
		//find the next thread and do context switch
		if (uthread::ReadyList.size()>0){
			target->S = FINISHED;
			TCB* next_TCB = uthread::ReadyList.front();
			Thread* next_thread = uthread::Threads[next_TCB->id];
			uthread::ReadyList.pop_front();
			uthread::RunningList.push_back(next_TCB);
			uthread::Threads[next_TCB->id]->S = RUNNING;
			uthread::context_switch(target, next_thread);	
		}
		else {
			cerr<<"this is last thread!"<<endl;
			return -1;
		}
	
	}
	return 0;
}

int uthread::uthread_suspend(int tid){
	// put thread from running to waiting.
	Thread* target = uthread::Threads[tid];
	if (target->S == READY){
		uthread::ReadyList.remove(target->tcb);
		target->S = WAITING;
		uthread::WaitingList[target->tcb] = 0;
		return 0;
        }
	else if (target->S == RUNNING){
                uthread::RunningList.remove(target->tcb);
		//target->S = WAITING;
		uthread::WaitingList[target->tcb] = 0;

		//find the next thread and do context switch
                if (uthread::ReadyList.size()>0){
                        target->S = WAITING;
                 	TCB* next_TCB = uthread::ReadyList.front();
                        Thread* next_thread = uthread::Threads[next_TCB->id];
                        uthread::ReadyList.pop_front();
                        uthread::RunningList.push_back(next_TCB);
                        uthread::Threads[next_TCB->id]->S = RUNNING;
                        uthread::context_switch(target, next_thread);
                }
                else {
                        cerr<<"this is last thread!"<<endl;
                        return -1;
                }
        }
	else {
		cerr<<"thread could only be suspended from either RUNNING or READY state!"<<endl;
		return -1;
	}
	return 0;
}

int uthread::uthread_resume(int tid){
	Thread* target = uthread::Threads[tid];
	if (target->S == WAITING){
		if (uthread::WaitingList[target->tcb] == 0){
			uthread::WaitingList.erase(target->tcb);
			target->S = READY;
			uthread::ReadyList.push_back(target->tcb);
			return 0;
		}
		else {
			cerr<<"fail to resume because it is still waiting for some thread."<<endl;
			return -1;
		}
	}
	else {
		cerr<<"thread that has not been suspended cannot be resumed!";
		return -1;
	}


}
void printForDebug(){
	cout<<"Threads size:"<<uthread::Thread_ID<<endl;
        cout<<"Threads"<<endl;
        for (int i=0; i < uthread::Threads.size();i++){
                cout<<uthread::Threads[i]->tcb->id<<" State: "<<uthread::Threads[i]->S<<endl;
        }
        cout<<"ReadyList"<<endl;

	for (std::list<TCB*>::iterator it=uthread::ReadyList.begin(); it != uthread::ReadyList.end(); ++it){
                cout<<(*it)->id<<endl;
        }
	cout<<"RunningList"<<endl;
	for (std::list<TCB*>::iterator it=uthread::RunningList.begin(); it != uthread::RunningList.end(); ++it){
                cout<<(*it)->id<<endl;
        }

}



int uthread::getCurrentUid(){
	if(uthread::RunningList.size() == 1){
		return RunningList.front()->id;
	}
	else if(uthread::RunningList.size() > 1){
		std::cerr<<"More than one running thread!\n";
		exit(-1);
	}
	else{
		cout<<"no running thread\n";
		exit(-1);
	}
}

int lock_init(lock_t *lock){
	TAS(lock, 0);
	return 0;
}

int acquire(lock_t *lock){
	return TAS(lock, 1) == 0 ? SUCCESS : FAIL;
}

int release(lock_t *lock){
	return TAS(lock, 0) == 1 ? SUCCESS : FAIL;
}
