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

Thread_id Thread::uthread_create(void *(*start_routine)(void *), void *arg){
	cout<<"create thread"<<endl;
        Thread* thread = new Thread();
        TCB* tcb = new TCB();
        tcb->stack = new char[STACK_SIZE];
        thread->tcb = tcb;
        tcb->stack_size = STACK_SIZE;
        tcb->sp = (address_t)tcb->stack + STACK_SIZE - sizeof(int);
        tcb->pc = (address_t)(start_routine);


        //stack setup
        //int temp_arg = *((int*)arg);
        //(tcb->sp) = temp_arg;
        //tcb->sp -= sizeof(int);
        //tcb->sp = (address_t)(start_routine);
        //tcb->sp -= sizeof(int);


        sigsetjmp(tcb->sjbuf,1);
        (tcb->sjbuf->__jmpbuf)[JB_SP] = translate_address(tcb->sp);
        (tcb->sjbuf->__jmpbuf)[JB_PC] = translate_address(tcb->pc);
        sigemptyset(&tcb->sjbuf->__saved_mask);
        std::cout<<"hello\n";
        siglongjmp(tcb->sjbuf,1);
}

void Thread::test(){
	cout<<"hello\n";
}
