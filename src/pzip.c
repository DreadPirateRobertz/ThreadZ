#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pzip.h"


struct threadInfo *theMeat; //Global pointer to threadIn
pthread_mutex_t fLock[26] = {0}; //Creates a mutex variable for each letter
pthread_barrier_t theWall; //This will be my barrier object to make threads wait
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
	//Create a mutex variable for each character
	for (int i = 0; i < 26; i++ ) {
		pthread_mutex_init(&fLock[i], NULL);
		}	
	//Setting attribute 
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	//Constructing the wayy and pthread array
	pthread_barrier_init(&theWall, NULL, n_threads);  
	pthread_t threadZ[n_threads];

	//Intializing and making space for other variables; 
	int rc;
	void *status;
	returny = malloc(sizeof(int) * n_threads); //Memory allocation
//perror("PZIP TEST 1");
	//Creating a local struct and passing to Global 
	struct threadInfo tInfo = { n_threads, input_chars, input_chars_size, zipped_chars, zipped_chars_count, char_frequency };
	//Pass the reference to the global struct
	theMeat = &tInfo;
	//Let's make the THREADZ
	for ( j = 0; j < n_threads; j++ ) {
		rc = pthread_create ( &threadZ[j], &attr, callBack, (void*) j ); //Creates appropriate threads, sets joinable attribute, intiates callBack,
										// and cast the soon to be threadID which is to be thought  of as an index as the ar to callBack   		 
		if (rc != 0) {
			perror("Error on pthread_create");
			exit(rc);
		}
	}

//perror("PZIP TEST 2");
	if(pthread_attr_destroy(&attr) != 0) {//Destroys the attribute and I believe prevents memory leaks
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

//perror("PZIP TEST 3");
	free(returny);
	pthread_barrier_destroy(&theWall);//TEAR DOWN THE WALL!!!
}

		
void* callBack(void* arg){
	long tid = (long) arg; //Mehemet said to think of this thread ID as an index, assign the void* and cast it back to it's original form
	int allotment = (theMeat->size) / (theMeat->n_threads);//Logic for how many letters  each thread should handle

	int start = tid * allotment; //Where should each thread start is accounted for by this logic
	int index = 0; 
	int length = allotment + start;		// Add Comment
	int charCount = 1;//Set to one because if this logic triggers then one repeat has already occurred 
	struct zipped_char *local = malloc(sizeof(struct zipped_char) * allotment); //Make a local struct that will store its specific chars here
//printf("This is thread: %ld  The allotment is: %d  The start is: %d  The length is: %d\n", tid, allotment, start, length);	
//perror("CALLBACK TEST 1");
	for ( int i = start; i < length; i++ ){//Checking to see if any 2 characters are the same
                char currChar = theMeat->input_chars[i];
                char nextChar = theMeat->input_chars[i+1];

		if ( currChar==nextChar && (i < length)-1 ){
			charCount++;
		}
	       else{//Update local zipped_char struct once above logic has completed
			struct zipped_char zippity = { theMeat->input_chars[i], charCount };
			//Insert into local
			local[index] = zippity; 
		
			pthread_mutex_lock(&fLock[local[index].character - 97]);
			theMeat->frequency[local[index].character - 97] += charCount;//CRITICAL SECTION
			pthread_mutex_unlock(&fLock[local[index].character - 97]);

			index++;
			charCount = 1; //Reset			
			}

//printf("Thread: %ld  The index is: %d\n", tid, index);
	}
		

//perror("CALLBACK TEST 2");
	returny[tid] = index;

//printf("This is thread: %ld  This is local at character 0:%c\n", tid, local[0].character);
//printf("This is thread: %ld  This is local at character 1:%c\n", tid, local[1].character);
//printf("This is thread: %ld  This is local at character 2:%c\n", tid, local[2].character);
//printf("This is thread: %ld  This is local at character 3:%c\n", tid, local[3].character);
//printf("This is thread: %ld  This is local at character 4:%c\n", tid, local[4].character);
//printf("This is thread: %ld  This is local at character 5:%c\n", tid, local[5].character);
	pthread_barrier_wait(&theWall);
	pthread_mutex_lock(&fLock[0]);
	*theMeat->count += index; 
	pthread_mutex_unlock(&fLock[0]);
	int spacer = 0; //Have to know where to store local results into the zipped_chars
	for ( int i = 0; i < tid; i++) {
		spacer += returny[i];
	}
	for ( int i = 0; i < index; i++){ //Insert the local into the global struct
		theMeat->zippy[i+spacer] = local[i];
	}

//printf("This is thread: %ld  This is zippy at character 0:%c\n", tid, theMeat->zippy[0].character);
//printf("This is thread: %ld  This is zippy at character 1:%c\n", tid, theMeat->zippy[1].character);
//printf("This is thread: %ld  This is zippy at character 2:%c\n", tid, theMeat->zippy[2].character);
//printf("This is thread: %ld  This is zippy at character 3:%c\n", tid, theMeat->zippy[3].character);
//printf("This is thread: %ld  This is zippy at character 4:%c\n", tid, theMeat->zippy[4].character);
//printf("This is thread: %ld  This is zippy at character 5:%c\n", tid, theMeat->zippy[5].character);
//printf("This is thread: %ld  This is zippy at character 6:%c\n", tid, theMeat->zippy[6].character);
//printf("This is thread: %ld  This is zippy at character 7:%c\n", tid, theMeat->zippy[7].character);
//printf("This is thread: %ld  This is zippy at character 8:%c\n", tid, theMeat->zippy[8].character);
//printf("This is thread: %ld  This is zippy at character 9:%c\n", tid, theMeat->zippy[9].character);
//printf("This is thread: %ld  This is zippy at character 10:%c\n", tid, theMeat->zippy[10].character);
//printf("This is thread: %ld  This is zippy at character 11:%c\n", tid, theMeat->zippy[11].character);
//printf("This is thread: %ld  This is zippy at character 12:%c\n", tid, theMeat->zippy[12].character);
//printf("This is thread: %ld  This is zippy at character 13:%c\n", tid, theMeat->zippy[13].character);
//printf("This is thread: %ld  This is zippy at character 14:%c\n", tid, theMeat->zippy[14].character);
//printf("This is thread: %ld  This is zippy at character 15:%c\n", tid, theMeat->zippy[15].character);





//perror("CALLBACK TEST 3");
	free(local);//Free it!
	pthread_exit(NULL); //Main waits
}
