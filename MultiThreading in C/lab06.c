#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>      /* header file for the POSIX API */
#include <string.h>      /* string handling functions */
#include <errno.h>       /* for perror() call */
#include <pthread.h>     /* POSIX threads */ 

#define MAX_SIZE  100000  /* Maximum elements in array */
#define MAX_NAME     256  /* Most filesystems only support 255-char filenames */
#define NUM_THREADS	   6  /* Number of threads */

int array[MAX_SIZE]; /* In global so child threads have access */
int count; 			 /* number of elements in 'array' */
typedef struct{
	int threadNum;
	int startIndex;
	int endIndex;
} threadInfo;

void *threadFunc (void *);

int main(int argc, char *argv[]){
	char filein[MAX_NAME];
	char fileout[MAX_NAME];
	FILE *infile; 			/* Creates filestream */
	FILE *outfile; 			/* Creates filestream */
	int ret;
	int i;
	int chunk;
	int tmp;
	
	/* Check if filename argument was given
	*		
	*	Example: ./file_io data.txt copy.txt
	*/
	if(argc < 3) {
		printf("Usage: %s <input_file> <output_file>\n", argv[0]);
		exit(1);
	}
	
	count = 0;/* Number of elements in array */
	strncpy(filein, argv[1], MAX_NAME);
	strncpy(fileout, argv[2], MAX_NAME);
	
	/* Open the file stream for read-only */
	infile = fopen(filein, "r"); 		/* "r" for read */
	if(infile == NULL) {
		perror("fopen: ");
		exit(EXIT_FAILURE);
	}
	
	/* Open output file stream for writing */
	outfile = fopen(fileout, "w");
	if(outfile == NULL) {
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
	
	chunk = count / NUM_THREADS;
	printf("Passing %d elements to each child thread\n", chunk);
	
	threadInfo thread[NUM_THREADS];
	
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
	
	/* Write the data to the outfile */
	for(tmp = 0; tmp < count; tmp++){
		fprintf(outfile, "%d ", array[tmp]);
		if(tmp && ((tmp % 10) == 0)) fprintf(outfile, "\n");
	}
	printf("%d doubled integers written to %s.\n", count, fileout);
	fclose(outfile); /* close the file stream */
	
	pthread_exit(NULL);
}

void *threadFunc(void *ptr){
	
	threadInfo *tmp = ptr;	/* do this since a void ptr cannot be dereferenced */
	
	printf("Thread data: id = %d  start = %d  end = %d \n",(*tmp).threadNum,
		(*tmp).startIndex, (*tmp).endIndex);
		
	int i;
	for(i = (*tmp).startIndex; i < (*tmp).endIndex + 1; i++){
		array[i] += array[i];
	}
	printf("Thread %d done with task\n", (*tmp).threadNum);
	pthread_exit((void*)EXIT_SUCCESS);
}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
