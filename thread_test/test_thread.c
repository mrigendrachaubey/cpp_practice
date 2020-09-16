// C program to implement cond(), signal() 
// and wait() functions 
#include <pthread.h> 
#include <stdio.h> 
#include <unistd.h> 

// Declaration of thread condition variable 
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER; 

// declaring mutex 
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 

int done = 1; 



// Thread function 
void bar() 
{ 
		// Let's signal condition variable cond1 
		printf("Signaling condition variable cond1\n"); 
		pthread_cond_signal(&cond1); 
}

// Thread function 
void* foo() 
{
	// acquire a lock 
	pthread_mutex_lock(&lock); 
	printf("Waiting on condition variable cond1\n");
	pthread_cond_wait(&cond1, &lock); 
	// release lock 
	pthread_mutex_unlock(&lock); 

	printf("Returning thread\n"); 

	return NULL; 
}



// Driver code 
int main() 
{ 
	pthread_t tid1, tid2; 

	// Create thread 1 
	pthread_create(&tid1, NULL, foo, NULL); 

	// sleep for 1 sec so that thread 1 
	// would get a chance to run first 
	sleep(1); 

	bar();
	// Create thread 2 
	//pthread_create(&tid2, NULL, bar, NULL); 

	// wait for the completion of thread 2 
	pthread_join(tid1, NULL); 

	return 0; 
} 

