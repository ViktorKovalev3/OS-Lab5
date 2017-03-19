#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>          /* For ftruncate  */
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For file mode constants */
using namespace std;

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

struct thread_arg{
    bool thread_ended = 0; //isn't end
    sem_t* sem[2];         //0 - read sem, 1 - write sem;
    int* shared_var;
};


static void * func1_thread(void *vp_arg){
    thread_arg* arg = (thread_arg*) vp_arg;
    while(!(arg->thread_ended)){
        //reader
        sem_wait(arg->sem[1]); //Wait write
        printf("%d", *(arg->shared_var));
        fflush(stdout);
        sem_post(arg->sem[0]);
    }
    pthread_exit((void*) "Reader thread ended");
}

int main(void)
{
    printf("Reader:\n\n");
    pthread_t thread1; thread_arg thread1_arg;
    int* shared_var;
    //Shared memory section
    int shm_d = shm_open("TestSharedMemory",
                         O_CREAT | O_RDWR,
                         S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH
                         );
    if (shm_d == -1) handle_error("shm_open");
    if (ftruncate(shm_d, sizeof(int))) handle_error("ftruncate");
    shared_var = (int*) mmap(NULL,
                             sizeof(int),
                             PROT_WRITE,
                             MAP_SHARED,
                             shm_d,
                             0
                             );
    if (shared_var == MAP_FAILED) handle_error("mmap");
    thread1_arg.shared_var = shared_var;

    //Named semaphore section
    sem_t* sem[2];
    for (int i = 0; i < 2; ++i){
        sem[i] = sem_open( (i) ? "write_sem.sem" : "read_sem.sem",
                           O_CREAT,
                           S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH,
                           0);
        if (sem[i] == SEM_FAILED) handle_error("sem_open");
        thread1_arg.sem[i] = sem[i];
    }

    //Thread section
    if ( pthread_create( &thread1, NULL, func1_thread, &thread1_arg ) )
        handle_error("pthread_create");

    getchar();
    thread1_arg.thread_ended = 1;

    //Exit section
    char* exit_thread1_code;
    if ( pthread_join( thread1, (void**) &exit_thread1_code ) )
            return 1;
    printf("\n%s\n", exit_thread1_code);

    for (int i = 0; i < 2; ++i) sem_close(sem[i]);
    if( sem_unlink("read_sem.sem")  == -1 )  handle_error("sem_unlink");
    if( sem_unlink("write_sem.sem") == -1 )  handle_error("sem_unlink");
    if( munmap(shared_var, sizeof(int)) == -1) handle_error("munmap");
    if( shm_unlink("TestSharedMemory") ) handle_error("shm_unlink");
    return EXIT_SUCCESS;
}
