// Create a program that create a thread and set the affinity of the thread to a specific core.
// Compile: gcc affinity.c -o affinity -lpthread
// Run: ./affinity

#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>

volatile static int print_sum_required = 0;
volatile static int print_sum_done = 0;
volatile static int sum = 0;
volatile static int terminated = 0;

int compute_sum()
{
    int sum = 0;
    for (int i = 0; i < 100000L; i++)
    {
        sum -= 1;
        for (int ii = 0; ii < 100000L; ii++)
        {
            sum += ii;
        }
    }

    return sum;
}

void change_affinity(int cpuid) {
    cpu_set_t mask;
    cpu_set_t get;
    CPU_ZERO(&mask);
    CPU_SET(cpuid, &mask);

    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
        printf("set thread affinity failed");
    }

    // Check the actual affinity mask assigned to the thread
    if (pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0) {
        printf("get thread affinity failed");
    }

    for (int i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, &get) && i != cpuid) {
            printf("Bad thread affinity: %d", i);
        }
    }
}

void set_priority(int policy, int priority) {
    struct sched_param param;
    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = priority;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
        perror("pthread_setschedparam");
    }
}

void *thread_calculate(void *arg)
{
    int i;

    change_affinity(7);
    set_priority(SCHED_FIFO, sched_get_priority_max(SCHED_FIFO));

    int iteration = 0;

    while( 0 == terminated ) {
        ++iteration;
        printf("Calculate iteration = %d\n", iteration);

        clock_t start = clock();
        sum = compute_sum();
        clock_t end = clock();
        double duration = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Duration = %f\n", duration);

        printf("Print sum required\n");
        print_sum_required = 1;

        //while( 0 == print_sum_done ) __sync_synchronize();
        while( 0 == print_sum_done && 0 == terminated);
        print_sum_done = 0;
    }
}

void *thread_print(void *arg)
{
    change_affinity(8);
    set_priority(SCHED_FIFO, sched_get_priority_max(SCHED_FIFO));

    while( 0 == terminated ) {
        //while( 0 == print_sum_required ) __sync_synchronize();
        while( 0 == print_sum_required && 0 == terminated) ;

        if( terminated ) break;

        printf("Sum = %d\n", sum);

        print_sum_done = 1;
        print_sum_required = 0;
    }
}

int main()
{
    pthread_t id_calculate;
    pthread_t id_print;

    change_affinity(1);

    pthread_create(&id_calculate, NULL, thread_calculate, NULL);
    pthread_create(&id_print, NULL, thread_print, NULL);

    printf("Waiting ...\n");

    // Wait for 2 minutes
    usleep(60*2*1000*1000);

    printf("Terminating ...\n");

    terminated = 1;

    pthread_join(id_calculate, NULL);
    pthread_join(id_print, NULL);
}