#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <string.h>

FILE * logfile;

typedef enum { false, true } bool;

typedef enum {idle, want_in, in_cs} state;

#define TIMER_MSG "Alarm received, killing processes, freeing memory\n"
#define NANOSECOND 1000000000

//***********************************function prototypes***********************************************
void createShm();
void releaseMem();
void checkClock();
int getRand();

//***************************************define keys for shared memory/message queues*************************
key_t shmkey1 = 640640;
key_t shmkey2 = 464064;
key_t shmkey3 = 444767;
key_t msgKey = 122446;
key_t semKey = 550040;
key_t pidKey = 218745;
key_t turnKey = 269845;

//************************************define shared memory ids********************************************
typedef struct {
	int nanoSec;
	int sec;
}sysClock;

typedef struct {
	state flag;
}strState;

strState * flagarray;

sysClock *OSSclk;
int * clock;
int shm_clock = 0;

int maxproc;
int * procs;
int shm_maxproc = 0;

int shm_msgQueue =0;

int shm_semaphore =0;
sem_t * sem;

int printHolder;

int shm_flag = 0;
//state * flagarray;

int shm_turn = 0;
int * turn;

char * t_opt_arg;
char * l_opt_arg;
char * s_opt_arg;

int timeout = 0;
//char * logfile;
static volatile sig_atomic_t doneflag = 0;


/*ARGSUSED*/
static void setdoneflag(int signo){
	printf("Alarm Signal Caught!\n");
	doneflag = 1;
}
typedef struct{
	int frame;
	int pNum;
	pid_t userPID;
	int dirtyBit;
	char state;
	int pcb;
}pTable;
pTable table[256];

struct sigaction act;

struct Queue {
	pid_t userPID;
	int killFlag;
	};


struct PCB {
	pid_t procPID;
	long int timeQuantum;
	long int systemTime;
	int blocked;
	//struct RCB * resources[10];
};

typedef struct{
	long int mtype;
	char  mtext[100];
	int proc;
	pid_t userPID;
}mymsg_t;



