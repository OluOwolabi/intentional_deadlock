#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <pthread.h> 
#include <stdbool.h> 
#include <stdarg.h>

// The variable to add/subtract
int deadLockSum = 0;

// The mutex locks for adding/subtracting
pthread_mutex_t add_lock;
pthread_mutex_t sub_lock;

// Prints from a thread with a header
void thread_print(int threadNum, const char *format, ...)
{
    va_list vargs;

    va_start(vargs, format);

    printf("[THREAD %d]: ", threadNum);

    vprintf(format, vargs);

    printf("\n");

    va_end(vargs);
}

// Thread 1's function
// 1. Gets the add_lock and then sleeps
// 2. When awakened, attempts to get sub_lock,
//    which thread 2 already has
// 3. Deadlock!
void *thread_1_deadlock(void *vargp)
{
    thread_print(1, "Executing!");

    pthread_mutex_lock(&add_lock);
    thread_print(1, "Have add_lock, sleeping...");
    sleep(1);
    pthread_mutex_lock(&sub_lock);
    thread_print(1, "Have sub_lock!");
    
    deadLockSum += 1;
    thread_print(1, "deadLockSum %d", deadLockSum);

    pthread_mutex_unlock(&sub_lock);
    thread_print(1, "Released sub_lock...");
    pthread_mutex_unlock(&add_lock);
    thread_print(1, "Released add_lock...");

    return NULL;
}

// Thread 2's function
// 1. Gets the sub_lock and then sleeps
// 2. When awakened, attempts to get add_lock,
//    which thread 1 already has
// 3. Deadlock!
void *thread_2_deadlock(void *vargp)
{ 
    thread_print(2, "Executing!");

    pthread_mutex_lock(&sub_lock);
    thread_print(2, "Have sub_lock, sleeping...");
    sleep(1);
    pthread_mutex_lock(&add_lock);
    thread_print(2, "Have add_lock!");
    
    deadLockSum -= 2;
    thread_print(2, "deadLockSum %d", deadLockSum);

    pthread_mutex_unlock(&add_lock);
    thread_print(2, "Released add_lock...");
    pthread_mutex_unlock(&sub_lock);
    thread_print(2, "Released sub_lock...");

    return NULL; 
} 
   
int main() 
{ 

    int ret;

    // Thread IDs
    pthread_t thread_1_id, thread_2_id; 
 
    // Initialize the addition mutex
    ret = pthread_mutex_init(&add_lock, NULL);
    if (ret)
    {
        perror("Failed to init mutex add_lock:");
    }

    // Initialize the subtraction mutex
    ret = pthread_mutex_init(&sub_lock, NULL);
    if (ret)
    {
        perror("Failed to init mutex sub_lock:");
    }

    // Create the first thread
    ret = pthread_create(&thread_1_id, NULL, thread_1_deadlock, NULL); 
    if (ret)
    {
        perror("Failed to create thread 1:");
    }

    // Create the second threads
    ret = pthread_create(&thread_2_id, NULL, thread_2_deadlock, NULL); 
    if (ret)
    {
        perror("Failed to create thread 2:");
    }

    // Join the threads with the main thread
    pthread_join(thread_1_id, NULL); 
    pthread_join(thread_2_id, NULL); 
 
    // Destroy mutex locks
    pthread_mutex_destroy(&add_lock);
    pthread_mutex_destroy(&sub_lock);

    return 0;
}
