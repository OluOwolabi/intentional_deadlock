#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

// The time a child will sleep after
// getting the first semaphore
#define SLEEP_TIME 3

// A mock boolean type
typedef int BOOL;
#define FALSE 0
#define TRUE 1

// Configurations for Semaphores and Shared Memory
#define SEM_CONFIG O_CREAT | O_EXCL
#define SEM_FLAGS  0644
#define SHM_CONFIG 0644 | IPC_CREAT

// On fatal error, exits with message
void die(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char **argv)
{
    int i;

    int shared_mem_id;    // Shared memory ID
    key_t shared_mem_key; // Shared memory key
    int *shared_adder;    // Shared adding variable
    BOOL *firstChildBorn; // Shared BOOL for tracking children

    // Semaphores for adding/subbing
    sem_t *add_sem;
    sem_t *sub_sem;

    // Holds PIDs from fork
    pid_t pid;

    // Get a shared memory key from /dev/null, and initialize id
    shared_mem_key = ftok("/dev/null", 5);
    shared_mem_id = shmget(shared_mem_key, sizeof(int), SHM_CONFIG); 
    if (shared_mem_id < 0)
    {
        die("shmget():");
    }

    // Place shared_adder into shared memory
    shared_adder = (int *)shmat(shared_mem_id, NULL, 0);
    *shared_adder = 0;

    // Place firstChildBorn into shared memory
    firstChildBorn = (BOOL *)shmat(shared_mem_id, NULL, 0);
    *firstChildBorn = TRUE;

    /* initialize semaphores for shared processes */
    add_sem = sem_open("add_sem", SEM_CONFIG, 0644, 1); 
    sub_sem = sem_open("sub_sem", SEM_CONFIG, 0644, 1); 

    // Fork children processes
    for (i = 0; i < 2; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            break;
        }
        // Error
        else if (pid < 0)
        {
            sem_unlink("add_sem");   
            sem_unlink("sub_sem");   

            sem_close(add_sem);  
            sem_close(sub_sem); 
            
            die("fork():");
        }    
        // Parent
        else
        {
            printf("Created child process with PID %d\n", pid);
        }
    }

    // Parent process
    if (pid != 0)
    {
        // Wait for both children to exit
        for (i = 0; i < 2; i++)
        {
            wait(NULL);
        }

        // Destroy shared memory
        shmdt(shared_adder);
        shmdt(firstChildBorn);
        shmctl(shared_mem_id, IPC_RMID, 0);

        // Unlink and close semaphores
        sem_unlink("add_sem");   
        sem_unlink("sub_sem");   
        sem_close(add_sem);  
        sem_close(sub_sem);  

        exit(0);
    }

    // First child process
    if (*firstChildBorn == TRUE)
    {
        printf("First child getting both locks...\n");

        *firstChildBorn = FALSE;
        
        // P Operation
        sem_wait(sub_sem);
        
        // By the time this process wakes, the
        // add_sem semaphore will have been grabbed
        // by the other child
        sleep(SLEEP_TIME);
        sem_wait(add_sem);

        printf("First Child has both locks.\n");

        *shared_adder += 1;

        printf("First child's adder value: %d\n", *shared_adder);
        
        // V operations
        sem_post(add_sem);
        sem_post(sub_sem);

        exit(0);
    }
    // Second child process
    else
    {
        printf("Second child getting both locks...\n");

        // P operation
        sem_wait(add_sem);

        // By the time this process wakes, the
        // sub_sem semaphore will have been grabbed
        // by the other child
        sleep(SLEEP_TIME);
        sem_wait(sub_sem);
        
        printf("Second Child has both locks.\n");
        
        *shared_adder -= 2;

        printf("Second child's adder value: %d\n", *shared_adder);

        // V operations
        sem_post(sub_sem);
        sem_post(add_sem);
        
        exit(0);
    }
}

