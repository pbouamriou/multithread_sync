// Create a program that create a thread and set the affinity of the thread to a specific core.
// Compile: gcc affinity.c -o affinity -lpthread
// Run: ./affinity

#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>

void *thread(void *arg)
{
    int i;
    cpu_set_t mask;
    cpu_set_t get;

    // Set affinity mask to include CPU 7
    CPU_ZERO(&mask);
    CPU_SET(7, &mask);

    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
    {
        printf("set thread affinity failed");
    }

    // Check the actual affinity mask assigned to the thread
    if (pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0)
    {
        printf("get thread affinity failed");
    }

    // Change priority
    int policy = SCHED_FIFO;
    int priority = sched_get_priority_max(policy);
    struct sched_param param;
    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = priority;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0)
    {
        perror("pthread_setschedparam");
    }

    printf("Set returned by pthread_getaffinity_np() contained:\n");
    for (i = 0; i < CPU_SETSIZE; i++)
        if (CPU_ISSET(i, &get))
            printf("    CPU %d\n", i);

    long int sum = 0;
    for (long int i = 0; i < 100000L; i++)
    {
        sum -= 1;
        for (long int ii = 0; ii < 100000L; ii++)
        {
            sum += ii;
        }
    }

    printf("Sum = %d\n", sum);

    // Synchronisation à faire avec __sync_synchronize();
}

int main()
{
    pthread_t id;

    pthread_create(&id, NULL, thread, NULL);
    pthread_join(id, NULL);
}