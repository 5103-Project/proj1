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
};
