#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <error.h>
#include <errno.h>

// #define MAX(x, y) (((x) > (y)) ? (x) : (y))
// #define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"
#define reset "\e[0m"

#define size 5

#define MAX 100
pthread_mutex_t try;
pthread_cond_t machineCond = PTHREAD_COND_INITIALIZER;
int students, machines, wastedTime;
pthread_mutex_t wasteTimeMutex = PTHREAD_MUTEX_INITIALIZER;
typedef struct student
{
    int id; // for identifying the student
    // int thread_id;
    int arrTime; // time of arrival
    // int alloc_time; // time when machine gets allocated
    int washTime; // time taken to wash clothes
    int patience; // patience
    char status;  // A for not washing, B for in Washing, C for Washed
} student;
student *student_arr;

int comparator(const void *a, const void *b)
{
    struct student x = *(struct student *)a;
    struct student y = *(struct student *)b;
    if (x.arrTime == y.arrTime)
    {
        return x.id - y.id;
    }
    return (x.arrTime - y.arrTime);
}
pthread_t *student_threads;
pthread_mutex_t *student_locks;
int students_left;
pthread_mutex_t studentWriter = PTHREAD_MUTEX_INITIALIZER;
int numOfMachines;
int globalstart;
int diss;
pthread_mutex_t machineLock = PTHREAD_MUTEX_INITIALIZER;
void *student_func(void *idx)
{
    int id = *((int *)idx);
    // sleep(student_arr[id].arrTime);
    struct timespec delay = {student_arr[id].arrTime, 10000 * (id+2)};
    pselect(0, NULL, NULL, NULL, &delay, NULL);
    printf(WHT "%d: Student %d arrives\n" reset, student_arr[id].arrTime, student_arr[id].id + 1);
    struct timespec tim;
    clock_gettime(CLOCK_REALTIME, &tim);
    tim.tv_sec = globalstart + student_arr[id].patience + student_arr[id].arrTime;
    int rc=-1;
    struct timespec end;

    pthread_mutex_lock(&machineLock);

    while (numOfMachines == 0)
    {
        rc = pthread_cond_timedwait(&machineCond, &machineLock, &tim);
        if (rc == ETIMEDOUT)
        {
            break;
        }
    }
    // printf("%d\n", numOfMachines);
        clock_gettime(CLOCK_REALTIME, &end);
//         if(student_arr[id].id==1)
// printf("%ld",end.tv_sec-globalstart);
    if (numOfMachines != 0)
    {
    LL:;
        numOfMachines--;
        pthread_mutex_unlock(&machineLock);
        clock_gettime(CLOCK_REALTIME, &end);
        printf(GRN "%ld:Student %d starts washing\n" reset, end.tv_sec-globalstart-student_arr[id].arrTime,student_arr[id].id + 1);
        pthread_mutex_lock(&wasteTimeMutex);
        wastedTime += ((double)end.tv_sec - (double)student_arr[id].arrTime - (double)globalstart);
        pthread_mutex_unlock(&wasteTimeMutex);
        double x = student_arr[id].washTime;
        sleep(x);
        // struct timespec delay = {x-1, 99999999};

        // pselect(0, NULL, NULL, NULL, &delay, NULL);

        pthread_mutex_lock(&machineLock);
        numOfMachines++;
        pthread_mutex_unlock(&machineLock);
        pthread_cond_signal(&machineCond);
        clock_gettime(CLOCK_REALTIME, &end);
        // pthread_mutex_unlock(&wash_mutex2);
        printf(YEL "%ld: Student %d leaves after washing\n" reset, end.tv_sec - globalstart, student_arr[id].id + 1);
        return NULL;
    }
    // pthread_mutex_lock(&try);
    // struct timespec delay = {0, 200000};
    //     pthread_mutex_unlock(&try);

    // pselect(0, NULL, NULL, NULL, &delay, NULL);
    pthread_mutex_unlock(&machineLock);
    // clock_gettime(CLOCK_REALTIME, &end);
    // pthread_mutex_lock(&machineLock);
    // if (numOfMachines > 0)
    // {
    //     goto LL;
    // }
    // pthread_mutex_unlock(&machineLock);

    pthread_mutex_lock(&wasteTimeMutex);
    wastedTime += student_arr[id].patience;
    diss += 1;
    pthread_mutex_unlock(&wasteTimeMutex);

    clock_gettime(CLOCK_REALTIME, &end);
    printf(RED "%ld Student %d leaves without washing\n" reset, end.tv_sec - globalstart, student_arr[id].id + 1);

    return NULL;
}

void input()
{
    diss = 0;
    students_left = 0;
    wastedTime = 0;
    numOfMachines = machines;
    student_arr = (student *)malloc(sizeof(student) * students);
    student_threads = (pthread_t *)malloc(sizeof(pthread_t) * students);
    student_locks = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * students);

    for (int i = 0; i < students; i++)
    {
        student_arr[i].id = i;
        scanf("%d%d%d", &student_arr[i].arrTime, &student_arr[i].washTime, &student_arr[i].patience);
    }
}
int main()
{

    srand(time(NULL));
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);
    globalstart = end.tv_sec;
    scanf("%d%d", &students, &machines);
    input();
    qsort(student_arr, students, sizeof(student), comparator);
    // for (int i = 0; i < students; i++)
    // {
    //     printf("%d\n", student_arr[i].id);
    // }
    for (int i = 0; i < students; i++)
    {
        int *idx = (int *)malloc(sizeof(int));
        *idx = i;
        pthread_create(&student_threads[i], NULL, student_func, (void *)idx);
    }
    for (int i = 0; i < students; i++)
    {
        pthread_join(student_threads[i], NULL);
    }
    printf(reset "%d\n%d\n", diss, wastedTime);
    double x = (double)diss / (double)students;
    x *= 100;
    if (x < 25)
    {
        printf("No\n");
    }
    else
    {
        printf("Yes\n");
    }

    return 0;
}
/*
5 2
6 3 5
3 4 3
6 5 2
2 9 6
8 5 2
*/
/*
machine()
{
    p_cond_waits()
    sleep(washing_time)
machine_available++;
}
student()
{
    while(machine_availble==0)
    {
        p_cond_wait()
    }
}
*/