#include "info.h"

/*typedef struct {
	long mtype =1;
	char mtext[100];
} mymsg_t;
*/

int main (int argc, char * argv[]){
	int userIndex = atoi(argv[1]);
	char userLogfile[10];
	strcpy(userLogfile, argv[2]);
	int maxIndex = atoi(argv[3]);
	//maxIndex = maxIndex -1;
	//printf("maxIndex = %d\n", maxIndex);
	//printf("%s\n", userLogfile);
	//printf("my index: %d", userIndex); 
	char buffer[100];
	mymsg_t messageToSend;
	messageToSend.mtype = 1;
	//printf("establish mtype: %d\n", messageToSend.mtype);

	//printf("in user process: %d\n", getpid());
/*	
	if(userIndex == 0){
		logfile = fopen(userLogfile, "a");
		fprintf(logfile, "Child %d with index %d writing to logfile\n", getpid(), userIndex);
		fclose(logfile); 
	}
*/

	//get and attach to shared memory for maxproc
	int getProc = shmget(shmkey3, sizeof(int), 0666);
	if(getProc < 0){
		perror("user shmget fail");
		exit(-1);
	} 
	int * usermaxproc = (int*)shmat(getProc, NULL,0);
	
	//get and attach to shared memory for the OSS Clock
	int getClock = shmget(shmkey1, sizeof(sysClock), 0666);

	sysClock * userClock = (sysClock *) shmat(getClock, NULL, 0);
	//printf("print clock from user %d\n", userClock->nanoSec);
	userClock->nanoSec = userClock->nanoSec +10;
	//printf("after add on clock: %d\n", userClock->nanoSec); 
	//printf("maxproc %d \n", *usermaxproc);
	//sleep(2);

	//get message queue and send message
	int msgQID = msgget(msgKey, 0666);
	if(msgQID < 0){
		perror("message queue fail in user");
		exit(-1);
	}

	//get and attach to shared memory for turn var
	int getTurnVar = shmget(turnKey, sizeof(int), 0666);
	if (getTurnVar < 0){
		perror("user turn var shmget fail");
		exit(-1);
	}
	int * userTurn = (int*) shmat(getTurnVar, NULL, 0);
	int localTurn = *userTurn;
	//printf("userTurn = %d\n", *userTurn);
	//printf("localTurn = %d\n", localTurn);

	//get and attach to shared memory for flagarray
	int getFlagArray = shmget(pidKey, 18 * sizeof(strState), 0666);
	if(getFlagArray < 0){
		perror("user flagarray shmget fail");
		exit(-1);
	}
	
	strState * userFlagArray = (strState*) shmat(getFlagArray, NULL, 0);
	/*
	userFlagArray[userIndex].flag = want_in;
	if(userFlagArray[userIndex].flag == want_in){
		userFlagArray[userIndex].flag = in_cs;
	}
	*/

	//*usermaxproc = *usermaxproc +1;
	//printf("after add: %d\n", *usermaxproc);
	messageToSend.proc = *usermaxproc; 
	//printf("test transfer to struct: %d\n", messageToSend.proc);
	messageToSend.userPID = getpid();
	//printf("test pid in struct: %d\n", messageToSend.userPID);
	//printf("before message\n");
	if(msgsnd(msgQID, (void*)&messageToSend, sizeof(mymsg_t), 0) == -1){
		perror("user messaged failed to send\n");
		exit(-1);
	}
	int randomSleepTime;	
	//multiple process solution code
	do{
		randomSleepTime = getRand();	
		sleep(randomSleepTime);
		do{
			userFlagArray[userIndex].flag =want_in;
			localTurn = *userTurn;
			
			//wait for my turn
			while(localTurn != userIndex){
				localTurn = (userFlagArray[localTurn].flag != idle) ? *userTurn : (localTurn + 1) %maxIndex;
			}
			
			//declare intention to enter critical section
			userFlagArray[userIndex].flag = in_cs;

			//check that no one else is in critical section
			for(localTurn = 0; localTurn<maxIndex; localTurn++){
				if((localTurn != userIndex) && (userFlagArray[localTurn].flag == in_cs)){
					break;
				}
			}
		} while (( localTurn < maxIndex) || (*userTurn != userIndex && userFlagArray[*userTurn].flag != idle));
		
		//Assign turn to self and enter critical section
		*userTurn = userIndex;
//***************************Critical Section*******************************************************
		logfile = fopen(userLogfile, "a");
		fprintf(logfile, "Child %d with index %d slept for %d seconds. Now  writing to logfile\n", getpid(), userIndex, randomSleepTime);
		fclose(logfile); 
//**************************End Critical Section**************************************************
		
		localTurn = (*userTurn + 1) % maxIndex;
		while (userFlagArray[localTurn].flag == idle){
			localTurn = (localTurn + 1) % maxIndex;
		}
		
		*userTurn = localTurn;
		userFlagArray[userIndex].flag = idle;
		
			
	
	}while(1);

	
	
	printf("user process %d finished\n", getpid());
	return 0;

}

int getRand(){
	return ((rand() % 5) + 1);
}
