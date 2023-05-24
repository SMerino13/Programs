#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>        /* to write time */
#include <sys/types.h>   /* standard data types for systems programming */
#include <sys/file.h>    /* open() system call */
#include <unistd.h>      /* header file for the POSIX API */
#include <sys/wait.h>    /* for wait() system call */ 
#include <signal.h>      /* signal handling */ 
#include <errno.h>       /* for perror() call */ 
#include <pthread.h>
#include <sys/msg.h>     /* message queue calls */ 
#include <sys/sem.h>     /* SysV semaphore header */
#include <sys/ipc.h>     /* SysV IPC header */
#include <sys/syscall.h> /* Make a syscall() to retrieve our TID */

#define MAX_NAME     256  /* Most filesystems only support 255-char filenames */
#define BUFFER 		 100
#define PRODUCERS      1  /* Number of producers */
#define CONSUMERS      5  /* Number of consumers */
#define PSEM		   5  /* Index for semaphore */

int mqid;	/* message queue ID variable */
int semid;	/* semaphore ID variable */
int limit;
int globalEnd = 0; 

/* Creating message data structure */
typedef struct{ 
	long type;			/* COME BACK */
	char text [BUFFER]; /* DECLARE mq_msg mymsg VARIABLES IN PRODUCER AND CONSUMER FUNCTION ) */
} mymsg;

/* you pass this structure to modify the values of a semaphore */
union semun {
  int  val;    
  struct semid_ds *buf;   
  unsigned short  *array; 
  struct seminfo  *__buf; 
} my_semun; 

typedef struct{
	int threadID;
	int data;
} threadInfo;

/* Producer: Grab and Release */
struct sembuf producer_grab[1];     /* Used to emulate semWait() */
struct sembuf producer_release[1];  /* Used to emulate semSignal() */

/* Consumers: Grab and Release */ 
struct sembuf consumer_grab[5][1];   /* Used to emulate semWait() */
struct sembuf consumer_release[5][1];  /* Used to emulate semSignal() */

int fib(int n) {
	if (n < 2) return n;
	return fib(n-1) + fib(n-2);
}

void ctrlc_handler(int sig){
	int rc;
	
	printf("\nCleaning up before terminating...\n");
	if(mqid > 0){
		rc = msgctl(mqid, IPC_RMID, NULL);
		if(rc < 0){
			perror("msgctl: ");
			exit(1);
		}
	}
	
	if(semid > 0){
		rc = semctl(semid, 0, IPC_RMID);
		if(rc < 0){
			perror("semctl: ");
			exit(1);
		}
	}
	
	exit(1);
}

void* producerFunc(void *dummy){
	int localEnd = 0;
	FILE *infile;
	int count, i, j, ret;
	char localbuf[BUFFER];
	mymsg proMessage;
	pid_t tid = syscall(SYS_gettid);  
	pid_t pid = getpid();
	threadInfo *tmp = dummy;
	
	printf("Producer pid: (%lu) tid: (%lu)\n", pid, tid);
	
	infile = fopen("poem", "r");
	printf("Producer opening poem for reading.\n");
	
	int Lmt = 5 * limit;
	for(i = 0; i < Lmt; i++){
		if(localEnd == 1){
			globalEnd = 1;
			
			for(j = 0; j < CONSUMERS; j++){
				ret = semop(semid, consumer_release[j], 1);
				if(ret < 0) {
					perror("semop release: ");
					exit(EXIT_FAILURE);
				}
				
				printf("Signaling Consumer %d to exit.\n", j);
			}
			
			printf("End of file detected in loop %d\n", i);
			break;
		} 
		
		count = 0;
		memset(localbuf, 0, sizeof(localbuf));
		
		printf("Producer waiting on PSEM...\n");
		ret = semop(semid, producer_grab, 1);
		if(ret < 0) {
			perror("semop grab: ");
			exit(EXIT_FAILURE);
		}
		
		while(count < BUFFER - 1){
			localbuf[count] = fgetc(infile);
			if(feof(infile)){
				localEnd = 1;
				break;
			}
			if(localbuf[count] == '\n'){
				break;
			}
			count++;
		}
		
		printf("Producer read %d characters from line %d.\n", count, i);
		
		if(count > 1){
			proMessage.type = 1;
			strncpy(proMessage.text, localbuf, BUFFER);
			if( msgsnd(mqid, &proMessage, sizeof(proMessage), 0) == -1){
				perror("IPC msgsnd: ");
			}
			
			fib(30);
			
			printf("Producer signaling Consumer %d to read line.\n", i%5);
			semop(semid, consumer_release[i % 5], 1);
			if(ret < 0) {
				perror("semop release: ");
				exit(EXIT_FAILURE);
			}
		}
	}
	
	printf("Ending Producer loop.\n");
	fclose(infile);
	
	pthread_exit(0);
}

void* consumerFunc(void *ptr){
	pid_t tid = syscall(SYS_gettid);  
	pid_t pid = getpid();
	threadInfo *tmp = ptr;
	mymsg conMessage;
	FILE *outfile;
	char fileout[MAX_NAME];
	int ret, i;
	
	sprintf(fileout,"log_offset%d", (*tmp).threadID);
	printf("Consumer %d opening log_offset%d for writing.\n", (*tmp).threadID, (*tmp).threadID);
	outfile = fopen(fileout, "w");
	
	printf("Consumer %d thread pid: (%lu) tid: (%lu)\n", (*tmp).threadID, pid, tid);

	for(i = 0; i < limit; i++){
		printf("Consumer %d waiting on CSEM %d...\n", (*tmp).threadID, (*tmp).threadID);
		ret = semop(semid, consumer_grab[(*tmp).threadID], 1);
		if(ret < 0) {
			perror("semop grab: ");
			exit(EXIT_FAILURE);
		}
		
		if(globalEnd == 1){
			printf("Consumer %d detected end of file signal from producer\n", (*tmp).threadID);
			break;
		}
		
		ret = msgrcv(mqid, &conMessage, sizeof(conMessage), 0 , 0);
		if(ret < 0) {
			perror("IPC msgrcv: ");
			exit(1);
		}
		
		printf("Consumer %d retrieving message from Producer.\n", (*tmp).threadID);
		fib(20);
		
		printf("Consumer %d signaling Producer.\n", (*tmp).threadID);
		ret = semop(semid, producer_release, 1);
		if(ret < 0) {
			perror("semop release: ");
			exit(EXIT_FAILURE);
		}
		
		fprintf(outfile, "%s", conMessage.text);
	}
	
	fclose(outfile); /* close the file stream */
	
	pthread_exit(0);
}

int main (int argc, char *argv[]){
	char pathname[BUFFER];
	char buf[BUFFER];
	int nsems;
	int i, ret, rc, dummy;
	long t;
	
	sigset_t mask;
	sigfillset(&mask); 			/* create a full mask */
	sigdelset(&mask, SIGINT);   /* remove SIGINT from mask */
	sigprocmask(SIG_BLOCK, &mask, NULL); 	/* block all signals execpt SIGNINT */
	
	struct sigaction ctrlc;		/* Strucutre for signal */
	ctrlc.sa_handler = ctrlc_handler;	/* assign handler */
	ctrlc.sa_flags = 0; 		/* hold flag restart */
	sigaction(SIGINT,  &ctrlc, NULL);	/* register handler */
	
	/* Defualt limit if no limit is specified */
	if(argc < 2) {
		limit = 5;
	}
	else {
		limit = atoi(argv[1]);
	}
	
	getcwd(pathname, BUFFER);	/* Returns string of the current directory's pathname. NULL terminated */
	strcat(pathname, "/foo");
	
	int ipckey = ftok(pathname, 7);
	mqid = msgget (ipckey, IPC_CREAT | 0666); /* Creating message queue */
	if (mqid < 0) {
        perror("msgget: ");
        exit(1);
	}
	
	printf("Created message queue with ID: %d\n", mqid);
	
	nsems = 6;
	semid = semget(ipckey, nsems, 0666 | IPC_CREAT);
	if (semid < 0) {
        perror("semget: ");
        exit(1);
	}
	
	printf("Created semaphore with ID: %d\n", semid);
	
	/* consumer semaphores (indices 0-4) */
	for(i = 0; i < CONSUMERS; i++){
		my_semun.val = 0;  /* Values of index 0-4 */
		semctl(semid, i, SETVAL, my_semun); /* Index i */
		printf("Consumer %d using semaphore %d\n",i, i);
	}

	my_semun.val = 1;  /* Value of index 5 */
	semctl(semid, PSEM, SETVAL, my_semun); /* Index 5 */
	printf("Producer using semaphore %d\n", PSEM);
	
	for(i = 0; i < CONSUMERS; i++){
		consumer_grab[i][0].sem_num = i;        
		consumer_grab[i][0].sem_flg = SEM_UNDO;    
		consumer_grab[i][0].sem_op =  -1;          
	
		consumer_release[i][0].sem_num = i;     
		consumer_release[i][0].sem_flg = SEM_UNDO;
		consumer_release[i][0].sem_op = +1;    
	}

	producer_grab[0].sem_num = PSEM;        
	producer_grab[0].sem_flg = SEM_UNDO;    
	producer_grab[0].sem_op =  -1;          
	
	producer_release[0].sem_num = PSEM;     
	producer_release[0].sem_flg = SEM_UNDO;
	producer_release[0].sem_op = +1;     
	
	threadInfo thread[CONSUMERS];
	for(i = 0; i < CONSUMERS; i++){
		thread[i].threadID = i;
	}
	
	/* Creating 5 threads for Consumers */
	pthread_t consumerThread[CONSUMERS];
	for(t = 0; t < CONSUMERS; t++){
		rc = pthread_create(&consumerThread[t], NULL, consumerFunc, (void *)&(thread[t]));
        printf("Calling pthread_create for consumer thread %d. . .\n", t);
		if(rc){
			printf("ERROR; return code from pthread_create() is %d\n",rc);
			exit(-1);
		}
	}
	
	pthread_t produceThread[PRODUCERS];
	for(t = 0; t < PRODUCERS; t++){
		rc = pthread_create(&produceThread[t], NULL, producerFunc, (void *)&dummy);
        printf("Calling pthread_create for producer thread. . .\n");
		if(rc){
			printf("ERROR; return code from pthread_create() is %d\n",rc);
			exit(-1);
		}
	}

	/* Block parent thread until each child thread exits */
	for(t = 0; t < CONSUMERS; t++){
		if (pthread_join(consumerThread[t], NULL) < 0) {
			 perror("pthread_join: ");
		}
		printf("Calling pthread_join for consumer thread %d. . .\n", t);
	}	
	
	for(t = 0; t < PRODUCERS; t++){
		if (pthread_join(produceThread[t], NULL) < 0) {
			 perror("pthread_join: ");
		}
		printf("Calling pthread_join for producer thread. . . .\n");
	}	
	
	ret = msgctl(mqid, IPC_RMID, NULL);
	if(ret < 0){
		perror("msgctl: ");
		exit(1);
	}
	
	rc = semctl(semid, 0, IPC_RMID);
	if(ret < 0){
		perror("semctl: ");
		exit(1);
	}	
	
	return 0;
}
