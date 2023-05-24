#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>      /* header file for the POSIX API */
#include <string.h>      /* string handling functions */
#include <errno.h>       /* for perror() call */
#include <pthread.h>     /* POSIX threads */ 
#include <sys/ipc.h>     /* SysV IPC header */
#include <sys/sem.h>     /* SysV semaphore header */
#include <sys/syscall.h> /* Make a syscall() to retrieve our TID */

#define BUFZ		   8  /* Character limit */
#define PSEM		   0  /* Index for semaphores*/
#define CSEM		   1  /* Index for semaphores*/
#define PRODUCERS      1  /* Number of producers */
#define CONSUMERS      1  /* Number of consumers */
#define DEBUG          0 

char textData[BUFZ];
int count;
int limit;
int semid;

struct sembuf grabPsem[1];     /* Used to emulate semWait() */
struct sembuf grabCsem[1];     /* Used to emulate semWait() */

struct sembuf releasePsem[1];  /* Used to emulate semSignal() */
struct sembuf releaseCsem[1];  /* Used to emulate semSignal() */

/* you pass this structure to modify the values of a semaphore */
union semun {
  int  val;    
  struct semid_ds *buf;   
  unsigned short  *array; 
  struct seminfo  *__buf; 
} my_semun; 

/* Prototypes for functions */
void* consumerFunc(void *dummy); 
void* producerFunc(void *dummy);
int fib(int);

int main(int argc, char *argv[]) {
	int curval;
	int ret;
	int nsems;
	char pathname[200];
		
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
	
	nsems = 2;
	
	semid = semget(ipckey, nsems, 0666 | IPC_CREAT);
	if (semid < 0) {
        perror("semget: ");
        exit(1);
   }
	
	printf("Created semaphore with ID: %d\n", semid);
	
	/* Set the initial value of the semaphore */
	my_semun.val = 1;  /* Value of index 0 */
	semctl(semid, PSEM, SETVAL, my_semun); /* Index 0 */
	
	my_semun.val = 0;  /* Value of index 1 */
	semctl(semid, CSEM, SETVAL, my_semun); /* Index 1 */
	
	/* Check current value of Semaphores */
	curval = semctl(semid, PSEM, GETVAL, 0);
	printf("PSEM initial val: %d\n", curval);
	
	curval = semctl(semid, CSEM, GETVAL, 0);
	printf("CSEM initial val: %d\n", curval);

	/* initialize sembuf struct for semaphore operations */
	grabPsem[0].sem_num = PSEM;        
	grabPsem[0].sem_flg = SEM_UNDO;    
	grabPsem[0].sem_op =  -1;          
	
	releasePsem[0].sem_num = PSEM;     
	releasePsem[0].sem_flg = SEM_UNDO;
	releasePsem[0].sem_op = +1;        
	
	grabCsem[0].sem_num = CSEM;        
	grabCsem[0].sem_flg = SEM_UNDO;    
	grabCsem[0].sem_op = -1;       	   
	
	releaseCsem[0].sem_num = CSEM;     
	releaseCsem[0].sem_flg = SEM_UNDO; 
	releaseCsem[0].sem_op = +1; 

	/* Emulate semWait(Psem) & semSignal(Psem) 
	ret = semop(semid, grabPsem, 1);
	ret = semop(semid, releasePsem, 1);
	
	Emulate semWait(Csem) & semSignal(Csem);
	ret = semop(semid, grabCsem, 1);
	ret = semop(semid, releaseCsem, 1); 
	
	/* Create an array of threads */
	pthread_t produceThread;
	pthread_t consumerThread;
	void *ProducerExit;
	int ConsumerExit;
	int rc;
	int ptr;
	int dummy;
	
	/* Create producer & consumer thread */
	rc = pthread_create(&produceThread, NULL, producerFunc, (void *)&dummy);
	if(rc){
		printf("ERROR; return code from pthread_create() is %d\n",rc);
		exit(-1);
	}
	
	rc = pthread_create(&consumerThread, NULL, consumerFunc, (void *)&dummy);
	if(rc){
		printf("ERROR; return code from pthread_create() is %d\n",rc);
		exit(-1);
	}
	
	/* Block parent thread until each thread exits */
	if(pthread_join(produceThread, &ProducerExit) < 0) {
		perror("pthread_join: ");
	}
	
	if(pthread_join(consumerThread, (void*)&ConsumerExit) < 0) {
		perror("pthread_join: ");
	}
	
	printf("Consumer thread exit code: %d\n", ConsumerExit);
	printf("Producer thread exit code: %d\n", ProducerExit);
	
	printf("Deleting semaphores with ID: %d\n", semid);
	semctl(semid, 0, IPC_RMID);
	
	exit(EXIT_SUCCESS);
}

/* PRODUCER FUNCTION */
void* producerFunc(void *dummy){
    pid_t tid = syscall(SYS_gettid);  
	pid_t pid = getpid();
	FILE *infile;
	int i;
	int ret;
	char tmp;
	
	infile = fopen("poem", "r");
	if (infile == NULL ) {
		fprintf(stderr, "error opening input file.\n");
		exit(1);
	}
	
	printf("Producer thread pid: (%lu) tid: (%lu)\n", pid, tid);
	count = 0;
	for (i = 0; i < limit; i++){
		printf("Producer waiting on PSEM...\n");
		semop(semid, grabPsem, 1);
		
		printf("Producer reading from input file.\n");
		count = 0;
		do {
			ret = fscanf(infile, "%c", &tmp);	/* read infile, look for int, store it */
			if(ret > 0) {
				textData[count++] = tmp; /* Got a character! */
			}
		}
		
		while(ret != EOF && count < BUFZ); /*  EOF: end of file */

		printf("Producer put %d characters on buffer.\n", count);
		printf("Producer signaling CSEM.\n");
		fib(40);
		semop(semid, releaseCsem, 1);
		
	}
	
	fclose(infile); /* close the file stream */
	
	pthread_exit(0);
}

/* CONSUMER FUNCTION */
void* consumerFunc(void *dummy){
    pid_t tid = syscall(SYS_gettid);  
	pid_t pid = getpid();
	FILE *outfile; 			/* Creates filestream */
	int ret;
	int i;
	int charNum;
	
	outfile = fopen("log", "w");
	if (outfile == NULL ) {
		fprintf(stderr, "error opening input file.\n");
		exit(1);
	}
	printf("Conusmers thread pid: (%lu) tid: (%lu)\n", pid, tid);
	
	for (i = 0; i < limit; i++){
		printf("Consumer waiting on CSEM...\n");
		semop(semid, grabCsem, 1);
		
		charNum = 0;
		while (charNum < count) {
			fputc(textData[charNum], outfile);
			charNum++;
		}
		
		memset(textData, 0, BUFZ);
		count = 0;
		
		fib(30);
		printf("Consumer signaling PSEM.\n");
		semop(semid, releasePsem, 1);
	}
	fputc('\n', outfile);
	fclose(outfile); /* close the file stream */
	
	pthread_exit(0);
}


int fib(int n) {
	if (n < 2) return n;
	return fib(n-1) + fib(n-2);
}