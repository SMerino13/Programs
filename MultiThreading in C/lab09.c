#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>      /* header file for the POSIX API */
#include <string.h>      /* string handling functions */
#include <errno.h>       /* for perror() call */
#include <pthread.h>     /* POSIX threads */ 
#include <sys/ipc.h>     /* SysV IPC header */
#include <sys/sem.h>     /* SysV semaphore header */
#include <sys/syscall.h> /* Make a syscall() to retrieve our TID */

#define BUFZ			200
#define NUM_THREADS		5

int limit;
int semid;

struct sembuf grab_fork[5][1];     /* Used to emulate semWait() */
struct sembuf release_fork[5][1];  /* Used to emulate semSignal() */

typedef struct{
	int philosopherID;
	int Fork1;
	int Fork2;
} threadInfo;

union semun {
  int  val;    
  struct semid_ds *buf;   
  unsigned short  *array; 
  struct seminfo  *__buf; 
} my_semun;

/* Prototype for philosopher() & fib() function*/
void *philosopherFunc (void *);
int fib(int);

int main(int argc, char *argv[]){
	int ret;
	int nsems;
	int i;
	char pathname[BUFZ];
	
	/* Defualt limit if no limit is specified */
	if(argc < 2) {
		limit = 50;
	}
	else if (argc > 2){
		printf("Usage: %s < set limit (optional) > \n", argv[0]);
		exit(1);
	}
	else {
		limit = atoi(argv[1]);
	}	
	
	printf("Limit set to %d\n", limit);

	/* key_t variable to hold the IPC key */
	key_t ipckey;
	getcwd(pathname,BUFZ);
	strcat(pathname, "/foo");
	ipckey = ftok(pathname, 7);
	
	nsems = 5;
	
	semid = semget(ipckey, nsems, 0666 | IPC_CREAT);
	if (semid < 0) {
        perror("semget: ");
        exit(1);
   }
   
   	printf("Created semaphore with ID: %d\n", semid);
	
//	------UNKNOWN IF INITIALIZE VAL OF SEMA'S-------
	for(i = 0; i < NUM_THREADS; i++){
		my_semun.val = 1;
		semctl(semid, i, SETVAL, my_semun);
	}


	for(i = 0; i < NUM_THREADS; i++){
		grab_fork[i][0].sem_num = i;        
		grab_fork[i][0].sem_flg = SEM_UNDO;    
		grab_fork[i][0].sem_op =  -1;          
	
		release_fork[i][0].sem_num = i;     
		release_fork[i][0].sem_flg = SEM_UNDO;
		release_fork[i][0].sem_op = +1;    
	}
	
	threadInfo thread[NUM_THREADS];
	/* Dijsktra's solution */
	for(i = 0; i < NUM_THREADS; i++){
		thread[i].philosopherID = i;
		thread[i].Fork1 = i % 4;
		thread[i].Fork2 = i + 1;
		
		thread[NUM_THREADS-1].Fork2 = 4;
		
		printf("Philospher %d will pick up fork %d, then fork %d\n", thread[i].philosopherID,
			thread[i].Fork1, thread[i].Fork2);
	}
	
	/* Creating 5 threads for philosophers */
	pthread_t threads[NUM_THREADS];
	int rc;
	long t;
	for(t = 0; t < NUM_THREADS; t++){
		rc = pthread_create(&threads[t], NULL, philosopherFunc, (void *)&(thread[t]));
		if(rc){
			printf("ERROR; return code from pthread_create() is %d\n",rc);
			exit(-1);
		}
		printf("Spawning thread for philosopher %d\n", t);
	}
	
	/* Block parent thread until each child thread exits */
	for(t = 0; t < NUM_THREADS; t++){
		if (pthread_join(threads[t], NULL) < 0) {
			 perror("pthread_join: ");
		}
		printf("Calling pthread_join for Philosopher %d. . .\n", t);
	}
	
	printf("Deleting semaphores with ID: %d\n", semid);
	semctl(semid, 0, IPC_RMID);
	
	pthread_exit(NULL);
}

void *philosopherFunc (void * ptr){
    pid_t tid = syscall(SYS_gettid);  
	pid_t pid = getpid();
	/* do this since a void ptr cannot be dereferenced */
	threadInfo *tmp = ptr;
	
	printf("Philosopher %d thread pid: (%lu) tid: (%lu)\n", (*tmp).philosopherID, pid, tid);
	
	for(int i = 0; i < limit; i++){
		printf("Philosopher %d is thinking...\n", (*tmp).philosopherID);
		fib(30);
		
		semop(semid, grab_fork[(*tmp).Fork1], 1);
		semop(semid, grab_fork[(*tmp).Fork2], 1);
		
		fib(20);
		printf("Philosopher %d is hungry...\n", (*tmp).philosopherID);
		printf("Philosopher %d is eating...\n", (*tmp).philosopherID);
		
		printf("Philosopher %d has put down both forks...\n", (*tmp).philosopherID);
		semop(semid, release_fork[(*tmp).Fork2], 1);
		semop(semid, release_fork[(*tmp).Fork1], 1);	
		
		printf("Philosopher %d has finished...\n", (*tmp).philosopherID);
	}
	
	pthread_exit(0);
}

int fib(int n) {
	if (n < 2) return n;
	return fib(n-1) + fib(n-2);
}