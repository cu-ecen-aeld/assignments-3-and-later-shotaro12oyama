#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    // Cast the thread argument to the correct type
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    // Wait for the specified time to obtain the mutex
    usleep(thread_func_args->wait_to_obtain_ms*1000);

    // Obtain the mutex
    pthread_mutex_lock(thread_func_args->mutex);
    usleep(thread_func_args->wait_to_release_ms*1000);

    // Release the mutex
    pthread_mutex_unlock(thread_func_args->mutex);

    thread_func_args->thread_complete_success = true;
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    // Allocate memory for thread_data and populate its fields
    struct thread_data* thread_func_args = (struct thread_data*) malloc(sizeof(struct thread_data));
    thread_func_args->mutex = mutex;
    thread_func_args->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_func_args->wait_to_release_ms = wait_to_release_ms;
    thread_func_args->thread_complete_success = false;

    // Create the thread and pass thread_data to it
    int result = pthread_create(thread, NULL, threadfunc, (void*) thread_func_args);
    if (result != 0) {
        ERROR_LOG("Failed to create thread: %d", result);
        free(thread_func_args);  // Cleanup allocated memory
        return false;
    }
    return true;
}

