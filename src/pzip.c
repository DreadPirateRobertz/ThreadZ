#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pzip.h"


struct threadInfo *master; 
pthread_mutex_t fLock[26] = {0}; 
pthread_barrier_t theWall; 
int *returny = NULL;

/**
 * pzip() - zip an array of characters in parallel
 *
 * Inputs:
 * @n_threads:		   The number of threads to use in pzip
 * @input_chars:		   The input characters (a-z) to be zipped
 * @input_chars_size:	   The number of characaters in the input file
 *
 * Outputs:
 * @zipped_chars:       The array of zipped_char structs
 * @zipped_chars_count:   The total count of inserted elements into the zippedChars array.
 * @char_frequency[26]: Total number of occurences
 *
 * NOTE: All outputs are already allocated. DO NOT MALLOC or REASSIGN THEM !!!
 *
 */
void pzip(int n_threads, char *input_chars, int input_chars_size,
	  struct zipped_char *zipped_chars, int *zipped_chars_count,
	  int *char_frequency)
{
	long j = 0; 

	for (int i = 0; i < 26; i++) 
		pthread_mutex_init(&fLock[i], NULL);
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_barrier_init(&theWall, NULL, n_threads);  
	pthread_t threadZ[n_threads];
	int rc;
	void *status;
	returny = malloc(sizeof(int) * n_threads);  
	struct threadInfo tInfo = { n_threads, input_chars, input_chars_size, zipped_chars, zipped_chars_count, char_frequency };
	master = &tInfo;	
	for (j = 0; j < n_threads; j++) {
		rc = pthread_create (&threadZ[j], &attr, callBack, (void*) j); 							   		 
		if (rc != 0) {
			perror("Error on pthread_create");
			exit(rc);
		}
	}
	if(pthread_attr_destroy(&attr) != 0) {
	        perror(""); 
		exit(rc);
	}
	for (int i = 0; i < n_threads; i++ ) {
		rc = pthread_join(threadZ[i], &status);	
		if (rc != 0) {
			perror("Error on pthread_join");
			exit(rc);
		}
	}
	for (int i = 0; i < 26; i++){
		pthread_mutex_destroy(&fLock[i]);
	}
	free(returny);
	pthread_barrier_destroy(&theWall);
}

void *callBack(void *arg) {
	long tid = (long) arg; 
	int allotment = (master->size) / (master->n_threads);
	int start = tid * allotment; 
	int index = 0; 
	int length = allotment + start;		
	int charCount = 1; 
	struct zipped_char *local = malloc(sizeof(struct zipped_char) * allotment); 
	struct zipped_char zippity;
	for (int i = start; i < length; i++){
                char currChar = master->input_chars[i];
                char nextChar = master->input_chars[i+1];

		if (currChar==nextChar && (i < (length-1))) {
			charCount++;
		}
	       else {
			zippity.character = master->input_chars[i];
			zippity.occurence = charCount;
      		       	local[index] = zippity; 
			pthread_mutex_lock(&fLock[local[index].character - 97]);
			master->frequency[local[index].character - 97] += charCount;
			pthread_mutex_unlock(&fLock[local[index].character- 97]);
			index++;
			charCount = 1; 			
			}

	}
	returny[tid] = index;
	pthread_barrier_wait(&theWall);
	*master->count += index; 
	int spacer = 0; 
	for (int i = 0; i < tid; i++) {
		spacer += returny[i];
	}	
	for (int i = 0; i < index; i++) {
		master->zippy[i+spacer] = local[i];
	}
	free(local);
	pthread_exit(NULL); 
}
