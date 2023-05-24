#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>      /* header file for the POSIX API */
#include <string.h>      /* string handling functions */
#include <errno.h>       /* for perror() call */
#include <pthread.h>     /* POSIX threads */ 
#include <sys/ipc.h>     /* SysV IPC header */
#include <sys/sem.h>     /* SysV semaphore header */
#include <stdint.h>      /* For 64 bit integers */

#define MAX_SIZE  100000  /* Maximum elements in array */
#define MAX_NAME     256  /* Most filesystems only support 255-char filenames */
#define NUM_THREADS	   6  /* Number of threads */
#define BUFZ 		 200

int array[MAX_SIZE]; /* In global so child threads have access */
int count; 			 /* number of elements in 'array' */
int semid;
int64_t globalSum;

struct sembuf grab[2];     /* Used to emulate semWait() */
struct sembuf release[1];  /* Used to emulate semSignal() */

typedef struct{
	int threadNum;
	int startIndex;
	int endIndex;
} threadInfo;

/* you pass this structure to modify the values of a semaphore */
union semun {
  int  val;    
  struct semid_ds *buf;   
  unsigned short  *array; 
  struct seminfo  *__buf; 
} my_semun; 

void *threadFunc (void *);

int main(int argc, char *argv[]) {
	char filein[MAX_NAME];
	char fileout[MAX_NAME];
	FILE *infile; 			/* Creates filestream */
	char pathname[BUFZ];
	int chunk;
	int nsems;
	int ret;
	int tmp;
	
	if(argc < 2) {
		printf("Usage: %s <input_file> \n", argv[0]);
		exit(1);
	}
	
	/* key_t variable to hold the IPC key */
	key_t ipckey;
	getcwd(pathname,BUFZ);
	strcat(pathname, "/foo");
	ipckey = ftok(pathname, 7);
	
	globalSum = 0;
	nsems = 1;
	
	semid = semget(ipckey, nsems, 0666 | IPC_CREAT);
	if (semid < 0) {
        perror("semget: ");
        exit(1);
   }
   
	count = 0;/* Number of elements in array */
	strncpy(filein, argv[1], MAX_NAME);
	
	/* Open the file stream for read-only */
	infile = fopen(filein, "r"); 		/* "r" for read */
	if(infile == NULL) {
		perror("fopen: ");
		exit(EXIT_FAILURE);
	}
	
	/* Input loop */
	do{
		ret = fscanf(infile, "%d", &tmp);	/* read infile, look for int, store it */
		if(ret > 0) array[count++] = tmp; /* Got a number! */
	} while(ret != EOF && count < MAX_SIZE); /*  EOF: end of file */
	
	printf("Read in %d integers from file %s.\n", count, filein);
	fclose(infile); /* close the file stream */
   
	/* Set the initial value of the semaphore */
	 my_semun.val = 0;
	 semctl(semid, 0, SETVAL, my_semun);
	 
	/* initialize sembuf struct for semaphore operations */
	grab[0].sem_num = 0;           /* Associate this with first (only) semaphore */
	grab[0].sem_flg = SEM_UNDO;    /* Release semaphore if premature exit */
	grab[0].sem_op =  0;           /* 1st action: wait until sem value is zero */
	
	grab[1].sem_num = 0;           /* Also associated with first semaphore */
	grab[1].sem_flg = SEM_UNDO;    /* Release semaphore if premature exit */
	grab[1].sem_op = +1;       	   /* 2nd action: increment semaphore value by 1 */
	
	release[0].sem_num = 0;        /* Also associated with first semaphore */
	release[0].sem_flg = SEM_UNDO; /* Release semaphore if premature exit */
	release[0].sem_op = -1;        /* 1st action: decrement semaphore value by 1 */
	
	chunk = count / NUM_THREADS;
	printf("Passing %d elements to each child thread\n", chunk);
	
	threadInfo thread[NUM_THREADS];
	
	int i;
	for(i = 0; i < NUM_THREADS; i++){
		thread[i].threadNum = i;
		thread[i].startIndex = (i * chunk);
		thread[i].endIndex = (i * chunk) + chunk;
	}
	thread[NUM_THREADS-1].endIndex = count;
	 
	/* Create an array of threads */
	pthread_t threads[NUM_THREADS];
	int rc;
	long t;
	for(t = 0; t < NUM_THREADS; t++){
		rc = pthread_create( &threads[t], NULL, threadFunc, (void *)&(thread[t]) );
		printf("Calling pthread_create for thread %d. . .\n", t);
		if(rc){
			printf("ERROR; return code from pthread_create() is %d\n",rc);
			exit(-1);
		}
	}
	
	/* Block parent thread until each child thread exits */
	for(t = 0; t < NUM_THREADS; t++){
		if (pthread_join(threads[t], NULL) < 0) {
			 perror("pthread_join: ");
		}
		printf("Calling pthread_join for thread %d. . .\n", t);
	}
	
	printf("Overall Sum is %ld\n", globalSum);
	
	ret = semctl(semid, 0, IPC_RMID);
	
	pthread_exit(NULL);
}

void *threadFunc(void *ptr){
	int64_t localSum;
	int ret;
	
	threadInfo *tmp = ptr;	/* do this since a void ptr cannot be dereferenced */
	
	printf("Thread data: id = %d  start = %d  end = %d \n",(*tmp).threadNum,
		(*tmp).startIndex, (*tmp).endIndex);
	
	localSum = 0;
	
	int i;
	for(i = (*tmp).startIndex; i < (*tmp).endIndex; i++){
		localSum += array[i];
	}
	
	printf("Local sum is %ld\n", localSum);
	
	printf("Thread %d requesting semaphore access. . .\n",(*tmp).threadNum);
	
	/* GRAB */
	/* these two semaphore operations are be executed atomically */
	ret = semop(semid, grab, 2); /* perform the two operations */
	
	 /* CRITICAL SECTION ENTRY */
	globalSum += localSum;
	
	printf("Thread %d is updating sum. . .\n", (*tmp).threadNum);
	printf("Thread %d is releasing semaphore. . .\n", (*tmp).threadNum);
	
	ret = semop(semid, release, 1);
	printf("Thread %d done with task\n", (*tmp).threadNum);
	
	pthread_exit((void*)EXIT_SUCCESS);
}