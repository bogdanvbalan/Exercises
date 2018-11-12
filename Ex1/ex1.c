#include "linked_list.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define NUM_THREADS 5   // The number of threads that are created
#define MAX_CALLS 10 	   // The maximum number of calls to a function of linked_list performed by each thread

pthread_barrier_t barrier; // The barrier used to sync all threads
pthread_mutex_t mutex;     // Mutex used to protect the head of the list

int result_check[2000]; //A vector used to check the results
int result_index = 0;   //The index of the result vector
int result_length = 0;
void add_to_result(int val);
void remove_from_result(int val);
void sort_vector();
	
void* thread_routine(void* data) { 
	int thd_id = *((int *)data);
	int rc;

	rc = pthread_barrier_wait(&barrier);

	if (rc == PTHREAD_BARRIER_SERIAL_THREAD) {
		printf("All threads are ready.\n");
	}

	srand(time(NULL) * pthread_self());
	int no_of_operations = rand() % MAX_CALLS + 1;

	while (no_of_operations) {
		int type_of_call = rand() % 6 + 1;   // The call to add() has more chances to appear
		int parameter = rand() % 100 + 1;

		if (type_of_call == 1) {
			pthread_mutex_lock(&mutex);
			printf("Thread %d executes sort()\n", thd_id);
			sort();
			sort_vector();                      // test function
			pthread_mutex_unlock(&mutex);
		}
		else if (type_of_call == 2) {
			pthread_mutex_lock(&mutex);
			printf("Thread %d executes delete(%d)\n", thd_id,parameter);
			delete(parameter);
			remove_from_result(parameter);   // test function
			pthread_mutex_unlock(&mutex);
		}
		else if (type_of_call == 3) {
			printf("Thread %d executes print()\n", thd_id);
			print();
		}
		else {
			printf("Thread %d executes add(%d)\n", thd_id,parameter);
			add(parameter);
			add_to_result(parameter);  //test function
		}

		no_of_operations--;
	}
}

/*Functions used to manage the result vector*/
void add_to_result(int val) {
	result_check[result_index] = val;
	result_index++;
	result_length++;
}
void remove_from_result(int val) {
   /*int i, j; 
   for (i = 0; i < result_length; i++) 
      if (result_check[i] == val) 
        break; 
   	if (i < result_length) {	 
        result_length--;
    	for (j = i; j < result_length; j++) 
        	result_check[j] = result_check[j+1]; 
   	} */
   	int i, j;
   	for (i = 0; i < result_length; i++) {
   		if (result_check[i] == val) {
   			for (j = i - 1; j < result_length - 1; j++) {
   				result_check[j] = result_check[j+1];
   			}
   			result_length--;
   			result_index--;
   		}
   	}
}
void sort_vector(){
	int j, k, temp;
	for (j = 0; j < result_index-1; j++) {
		for (k = j + 1; k < result_index; k++) {
			if (result_check[j] > result_check[k]) {
				temp = result_check[j];
				result_check[j] = result_check[k];
				result_check[k] = temp;
			}
		}
	}
}

int main() {
	int i,j;

	sort();

	pthread_t tids[NUM_THREADS];
	int th_id[NUM_THREADS];

	pthread_barrier_init(&barrier, NULL, NUM_THREADS);
	pthread_mutex_init(&mutex, NULL);

	for (i = 0;i < NUM_THREADS; i++) {
		th_id[i] = i;
		pthread_create(&tids[i], NULL, thread_routine, &(th_id[i]));
	}

	for (i = 0;i < NUM_THREADS; i++) {
		pthread_join(tids[i], NULL);
	}

	pthread_barrier_destroy(&barrier);
	pthread_mutex_destroy(&mutex);

	printf("The list before flushing.\n");
	print();
	flush();
	
	printf("The result vector.\n");
	for (j = 0; j < result_length; j++) {
		printf("%d\n", result_check[j]);
	}

	printf("result_length: %d\n",result_length);
	printf("result_index: %d\n",result_index);

	return 0;
}