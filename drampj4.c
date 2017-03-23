/* Dinesh Ramamoorthy */
/* U51857354 */




#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>

/* SHARED FLAG */
bool in_cs;

sem_t sem_shared_read;
sem_t sem_shared_write;
int sem_value;
int assignee;

struct data{
	int count;
};
struct data * counter;

/* THREAD METHODS */
/* Accesses shared counter value 250M times */
void relaxandspendtime(){
	if(in_cs){printf("TRUE\n");}else{printf("FALSE\n");}
	while(in_cs);
	if(!in_cs){
		printf("[CRITICAL] STARTED.\n");
		for(int s=0; s<250000000; s++)
			assignee = counter->count;
		printf("[CRITICAL] ENDED.\n");
	}
}

/* READER */
void * reader_thread(void *arg){
	/* TODO: READER LOGIC */
	sleep(1);
	int k = *((int *)arg);

	sem_wait(&sem_shared_read);
	sem_getvalue(&sem_shared_read, &sem_value);

	/* CRITICAL SECTION: SEMAPHORE LOGIC */
	printf("READER_THREAD%d [STARTED]\n", k);
	relaxandspendtime();
	printf("READER_THREAD%d [DONE]\n", k);
	
	sem_post(&sem_shared_read);
	sem_getvalue(&sem_shared_read, &sem_value);

	return arg;
}

/* WRITER */
void * writer_thread(void *arg){
	/* TODO: WRITER LOGIC */
	sleep(1);
	sem_wait(&sem_shared_write);

	printf("[WRITER STARTED]\n");
	in_cs = true;
	for(int i = 0; i < 25000; i++){
		counter->count = counter->count + 1;
	}	

	sem_post(&sem_shared_write);
	in_cs = false;	

	return arg;
}

/* MAIN */
int main(int argc, char *argv[]){
	/* TODO: Read Input Values */
	int max_r = 12;
	int r = 0;
	int i;
	int shmid;		//Shared Memory ID
	pthread_t readers[100];	//Process ID For Reader Thread i
	pthread_t writers[1];
	pthread_attr_t attr[1];
	
	int n = atoi(argv[1]);
	
	sem_init(&sem_shared_read, 0, 1);
	sem_init(&sem_shared_write, 0, 1);

	if(n < max_r){
		printf("[VALID CASE. COUNTER INITIATED.]\n");	
		counter = malloc(sizeof(struct data *));
		counter->count = 0;
		
		/* CREATE WRITER THREAD */
		pthread_create(&writers[0], NULL, writer_thread, NULL);

		/* CREATE 'n' READER THREADS */
		for(i=0; i < n; i++){
			printf("[CREATING THREAD %d]\n", i);
			int j = i;
			/* REGION: pointer to pass as an argument to child threads. */
			int * arg = (int *)malloc(sizeof(void *));
			if (arg == NULL){
                	        fprintf(stderr, "COULDN'T ALLOCATE MEMORY FOR THREAD ARG\n");
        	                exit(EXIT_FAILURE);
	                }
			*arg = j;
			/* ENDREGION */

			int c = pthread_create(&readers[j], NULL, reader_thread,(void *)arg);
			if(c==0){
				printf("MAIN THREAD CHILD NUMBER: %d\n", j);
			}else{
				printf("PTHREAD_CREATE TRACE RETURN: %d\n", j);
			}
		}
		
		printf("MAIN THREAD EXITING FOR LOOP.\n");

		/* Thread Termination */
		pthread_join(writers[0], NULL);
		for(int x=0; x < n; x++){
			pthread_join(readers[x], NULL);
		}
	}
	else{
		printf("MAXIMUM ALLOWED FOR NUMBER OF READERS IS: %d", max_r);
	}
	
	/* End of Program */
	free(counter);
	sem_destroy(&sem_shared_read);
	sem_destroy(&sem_shared_write);
	return 0;
}
