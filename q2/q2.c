#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <error.h>
#include <errno.h>
// #include <qp0z1170.h>
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"
#define reset "\e[0m"

void enqueue();
void dequeue();
void show();

int *order_pizza_queue;
int *order_pizza_id_queue;
int Rear = -1;
int Front = -1;
int SIZE = 100;

void enqueue(int x, int y)
{
    if (Rear == SIZE - 1)
        //    printf("Overflow \n");
        return;
    else
    {
        if (Front == -1)

            Front = 0;
        // printf("Element to be inserted in the Queue\n : ");
        Rear = Rear + 1;
        order_pizza_queue[Rear] = x;
        order_pizza_id_queue[Rear] = y;
    }
}

void dequeue()
{
    if (Front == -1 || Front > Rear)
    {
        // printf("Underflow \n");
        return;
    }
    else
    {
        Front = Front + 1;
        if (Front > Rear)
            Front = Rear = -1;
    }
}
void display()
{
    if (Rear == -1)
        printf("\nQueue is Empty!!!");
    else
    {
        int i;
        printf("\nQueue elements are:\n");
        for (i = Front; i <= Rear; i++)
            printf("%d  %d \n", order_pizza_queue[i], order_pizza_id_queue[i]);
    }
    //   printf("\n");
}

typedef struct
{
    int id;
    int arrivalTime;
    int exitTime;
    char status; // Cooking(C),Sleeping(S)
    int *rejectArrPizzId;
    int rejectIndex;
} chef;
typedef struct
{
    int id;
    int quantity;
} ingredient;
typedef struct
{
    int id;
    int chefId;
    int *ingredients;
    int numIngredients;
    int prepTime;
} pizza;
int driveThroughPeople;

typedef struct
{
    int id;
    int chefId;
    int orderId;
    char status; //(I) Initialised, assigned to Chef(A), (C) for completed,rejected for (R)
} assignedPizza;

typedef struct
{
    int id;
    int custId;
    int arrivalTime;
    char status; // Initialised(I),Queue(Q),accepted(A),rejected(R), Partially made(P),Cooked(C)
    int numOfPizzas;
    int accepted; // -1 if rejected
    int pizzaIdMade;
    int newPizza;
    assignedPizza *pizzas;
    int pizzasProcessed;
    int pizzasMade;
} order;

int chefsWorking, activeChefs;
pthread_mutex_t restaurantCheck;
int globalstart; // initialise after restaurant has started
int driveThroughCap, numOfIngredients, totChefs, totPizzaVariety, totIngredients, totCustomers, totOvens, timeToReachPickup;
int *ovenArr;
int *orderQueue;
pthread_t *chefThreads;
pthread_t *orderThreads;

pthread_mutex_t *orderLocks;
pthread_mutex_t *writePizzaLocks;
pthread_mutex_t *chefLocks;
pthread_mutex_t *chefRejects;
pthread_mutex_t pizzaQueue = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ingredientLock = PTHREAD_MUTEX_INITIALIZER;
// printf("\n#######\n%d %d %d %d\n",id,orderArr[id].numOfPizzas,orderArr[id].pizzasProcessed,orderArr[id].pizzasMade);

pthread_mutex_t ovenThread = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t driveQueueThread = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t driveNext = PTHREAD_COND_INITIALIZER;
pthread_cond_t *orderCond;
pthread_cond_t chefCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t chefCondforOven = PTHREAD_COND_INITIALIZER;
pizza *pizzaArr;
chef *chefArr;
int *ingredientLimit;
order *orderArr;

void input()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    globalstart = t.tv_sec;
    numOfIngredients = 0;
    driveThroughPeople = 0;
    driveThroughCap = 5; // check this on moodle
    scanf("%d%d%d%d%d%d", &totChefs, &totPizzaVariety, &totIngredients, &totCustomers, &totOvens, &timeToReachPickup);
    chefsWorking = totChefs;
    activeChefs = 0;
    chefThreads = (pthread_t *)malloc(sizeof(pthread_t) * totChefs);
    chefLocks = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * totChefs);
    // chefRejects = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * totChefs);
    orderThreads = (pthread_t *)malloc(sizeof(pthread_t) * totCustomers);
    orderLocks = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * totCustomers);
    writePizzaLocks = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * totCustomers);
    orderCond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t) * totCustomers);
    pizzaArr = (pizza *)malloc(sizeof(pizza) * totPizzaVariety);
    chefArr = (chef *)malloc(sizeof(chef) * totChefs);
    // customerArr = (customer *)malloc(sizeof(customer) * totCustomers);
    orderArr = (order *)malloc(sizeof(order) * totCustomers);
    ovenArr = (int *)malloc(sizeof(int) * totOvens);
    int pizzaTotalOrdered = 0;
    for (int i = 0; i < totOvens; i++)
    {
        ovenArr[i] = 0;
    }
    for (int i = 0; i < totPizzaVariety; i++)
    {
        int ingnum;
        scanf("%d%d", &pizzaArr[i].id, &pizzaArr[i].prepTime);
        scanf("%d", &ingnum);
        pizzaArr[i].ingredients = (int *)malloc(sizeof(int) * ingnum);
        numOfIngredients = MAX(numOfIngredients, ingnum);
        pizzaArr[i].numIngredients = ingnum;
        for (int j = 0; j < ingnum; j++)
        {
            scanf("%d", &pizzaArr[i].ingredients[j]);
        }
        pizzaArr[i].chefId = -1;
    }
    numOfIngredients = totIngredients;
    ingredientLimit = (int *)malloc(sizeof(int) * totIngredients);
    for (int i = 0; i < totIngredients; i++)
    {
        scanf("%d", &ingredientLimit[i]);
    }
    for (int i = 0; i < totChefs; i++)
    {
        chefArr[i].id = i + 1;
        chefArr[i].status = 'S';
        scanf("%d%d", &chefArr[i].arrivalTime, &chefArr[i].exitTime);
        if (chefArr[i].arrivalTime == 0)
        {
            activeChefs++;
        }
        chefArr[i].rejectArrPizzId = (int *)malloc(sizeof(int) * (totPizzaVariety * totCustomers));
        chefArr[i].rejectIndex = 0;
    }

    for (int i = 0; i < totCustomers; i++)
    {
        int numOfPizzas;
        scanf("%d%d", &orderArr[i].arrivalTime, &numOfPizzas);
        orderArr[i].id = i;
        orderArr[i].pizzas = (assignedPizza *)malloc(sizeof(assignedPizza) * numOfPizzas);
        // orderLocks[i] = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * (numOfPizzas + 1));
        for (int j = 0; j < numOfPizzas; j++)
        {
            scanf("%d", &orderArr[i].pizzas[j].id);
            orderArr[i].pizzas[j].status = 'I';
        }
        orderArr[i].status = 'I';
        orderArr[i].numOfPizzas = numOfPizzas;
        pthread_cond_init(&orderCond[i], NULL);
        orderArr[i].accepted = 1;
        orderArr[i].newPizza = 0;
        pizzaTotalOrdered += numOfPizzas;
        orderArr[i].pizzasProcessed = 0;
        orderArr[i].pizzasMade = 0;
    }
    order_pizza_queue = (int *)malloc(sizeof(int) * pizzaTotalOrdered);
    order_pizza_id_queue = (int *)malloc(sizeof(int) * pizzaTotalOrdered);
    SIZE = pizzaTotalOrdered;
}
// 3seconds by chef, then oven

int checkRejection(int id)
{

    if (orderArr[id].pizzasProcessed == orderArr[id].numOfPizzas)
    {
        if (orderArr[id].pizzasMade == 0)
        {
            orderArr[id].status = 'R';
        }
        else if (orderArr[id].pizzasMade < orderArr[id].pizzasProcessed)
        {
            orderArr[id].status = 'P';
        }
        else
        {
            // printf("hiii\n");
            orderArr[id].status = 'C';
        }
        return 1;
    }
    return 0;
}
int rejectPizzaCheck(int id, int pizza_id)
{
    for (int i = 0; i < chefArr[id].rejectIndex; i++)

    {
        if (chefArr[id].rejectArrPizzId[i] == pizza_id)
        {

            return 1;
        }
    }
    return 0;
}
void *chefFunc(void *idx)
{
    int rejectedDueToTime = 0;

    int time;
    int id = *(int *)idx;
    sleep(chefArr[id].arrivalTime);
    printf(BLU "Chef %d arrives at time %d\n", id+1, chefArr[id].arrivalTime);
    pthread_mutex_lock(&restaurantCheck);
    activeChefs++;
    pthread_mutex_unlock(&restaurantCheck);
    // assign order;
start:
    pthread_mutex_lock(&chefLocks[id]);
start1:
    pthread_mutex_lock(&pizzaQueue);

    struct timespec end;
    // int endTime = end.tv_sec
    int checkArrivalTime = globalstart + chefArr[id].arrivalTime;
    clock_gettime(CLOCK_REALTIME, &end);
    int waiter = end.tv_sec;
    waiter -= checkArrivalTime;
    // printf("timed wait %d %d\n", id, chefArr[id].exitTime - chefArr[id].arrivalTime - waiter);
    
    clock_gettime(CLOCK_REALTIME, &end);

    end.tv_sec += (chefArr[id].exitTime - chefArr[id].arrivalTime - waiter);
    // printf("%d %d\n", id, end.tv_sec - globalstart);

    int rc;
    while ((Front == -1 || rejectPizzaCheck(id, order_pizza_queue[Front]) == 1))
    {
        // display();
        struct timespec end1;
        clock_gettime(CLOCK_REALTIME, &end1);
        if ((end1.tv_sec - globalstart >= chefArr[id].exitTime))
        {
            break;
        }
        rc = pthread_cond_timedwait(&chefCond, &pizzaQueue, &end);
    }

    clock_gettime(CLOCK_REALTIME, &end);
    // checks if time has elapsed
    if ((end.tv_sec - globalstart >= chefArr[id].exitTime))
    {
        // if time has elapsed

        pthread_mutex_lock(&restaurantCheck);
        chefsWorking--;
        activeChefs--;
        // printf("%d\n", chefsWorking);
        if (chefsWorking == 0)
        {
            for (int i = 0; i < totCustomers; i++)
            {
                pthread_cond_signal(&orderCond[i]);
            }
        }
        pthread_mutex_unlock(&restaurantCheck);
        pthread_mutex_unlock(&pizzaQueue);

        struct timespec lmao;
        clock_gettime(CLOCK_REALTIME, &lmao);

        printf(BLU "Chef %d exits at time %ld\n" reset, id+1, lmao.tv_sec - globalstart);

        return NULL;
    }
    if (Front == -1)
    {

        goto start1;
    }
    int pizza_id, order_id;

    pizza_id = order_pizza_queue[Front]; // subtract 1 for array access
    order_id = order_pizza_id_queue[Front];
    // if (rejectPizzaCheck(id, pizza_id) == 1)
    // {
    //     // pthread_mutex_unlock(&pizzaQueue);
    //     while (Front == -1 || (end.tv_sec - (chefArr[id].arrivalTime + globalstart) >= chefArr[id].exitTime))
    //     {
    //         printf("hii\n");
    //         display();
    //         clock_gettime(CLOCK_REALTIME, &end);
    //         pthread_cond_wait(&chefCond, &pizzaQueue);
    //     }
    // }
    int totalPizzas = orderArr[order_id].numOfPizzas;

    dequeue();
    pthread_mutex_unlock(&pizzaQueue);

    // check for time
    struct timespec time2;
    clock_gettime(CLOCK_REALTIME, &time2);
    if (chefArr[id].arrivalTime + pizzaArr[pizza_id].prepTime > chefArr[id].exitTime + (time2.tv_sec-globalstart)) // checkthis
    {
        printf("%ld %d %d\n", time2.tv_sec - globalstart, pizzaArr[pizza_id].prepTime, chefArr[id].exitTime);
        printf(BLU "Chef %d is rejecting the pizza %d from order %d due to lack of time.\n" reset, id+1, pizza_id+1, order_id+1);
        chefArr[id].rejectArrPizzId[chefArr[id].rejectIndex++] = pizza_id;
        rejectedDueToTime = 1;
        goto end;
    }
    pthread_mutex_lock(&ingredientLock);
    // for (int i = 0; i < orderArr[order_id].numOfPizzas; i++)
    // {
        // if (pizza_id == orderArr[order_id].pizzas[i].id)
        // {
            int pid = pizza_id;
            int check = 1;
            for (int k = 0; k < pizzaArr[pid].numIngredients; k++)
            {
                int ingid = pizzaArr[pid].ingredients[k] - 1;
                // printf("%d\n",ingid);
                if (ingredientLimit[ingid] != 0)
                {
                    continue;
                }
                else if (ingredientLimit[ingid] == 0)
                {
                    check = 0;
                    break;
                }
            }
            if (check == 0)
            {
                orderArr[order_id].pizzas[pid].status = 'R';
                printf(BLU "Chef %d is rejecting the pizza %d from order %d due to lack of ingredients.\n" reset, id+1 ,1+ pizza_id, order_id+1);
                pthread_mutex_unlock(&ingredientLock);
                goto end;
                // fix rejection of pizzas in personFunc, add check per pizza rejection
            }
            for (int k = 0; k < pizzaArr[pid].numIngredients; k++)
            {
                int ingid = pizzaArr[pid].ingredients[k] - 1;
                ingredientLimit[ingid]--;
            }
            // break;
        // }
    // }

    pthread_mutex_unlock(&ingredientLock);
    printf(BLU "Pizza %d in order %d assigned to chef %d\n" reset,  pizza_id+1, order_id+1,id+1);

    sleep(3); // sleep 3 time
    // check if oven is available;
    // implement yaha pe chef ka time khatam hua ki nahi
    int ovenid = 0;
    // printf("oven\n");
    pthread_mutex_lock(&ovenThread); // checkthis
    for (int i = 0; i < totOvens; i++)
    {
        if (ovenArr[i] == 0)
        {
            ovenid = i;
            ovenArr[i] = 1;
            goto chef1;
        }
    }
    printf(BLU "Chef %d is preparing the pizza %d from order %d.\n" reset, id+1, pizza_id+1, order_id+1);

    struct timespec endTime11;
    clock_gettime(CLOCK_REALTIME,&endTime11);
    endTime11.tv_sec = chefArr[id].exitTime - pizzaArr[pizza_id].prepTime;
   int rc2= pthread_cond_timedwait(&chefCondforOven, &ovenThread,&endTime11);
   if (rc2 == 110) // ETIMEDOUT
    {
        goto end;
    }
    for (int i = 0; i < totOvens; i++)
    {
        if (ovenArr[i] == 0)
        {
            ovenid = i;
        }
    }
chef1:;

    pthread_mutex_unlock(&ovenThread);
    printf(BLU "Chef %d has put the pizza %d for order %d in oven %d.\n" reset, id+1, pizza_id+1, order_id+1, ovenid+1);
    sleep(MAX(pizzaArr[pizza_id].prepTime , 0));
    printf(BLU "Chef %d has picked up the pizza %d for order %d from the oven %d.\n" reset, id+1, pizza_id+1, order_id+1, ovenid+1);
    // implement checkthis oven here

    pthread_mutex_lock(&ovenThread); // checkthis
    ovenArr[ovenid] = 0;
    // printf("oven allocated\n");
    pthread_mutex_unlock(&ovenThread);

    orderArr[order_id].newPizza = pizza_id;
    pthread_mutex_lock(&writePizzaLocks[order_id]);
    orderArr[order_id].pizzasMade += 1;
    pthread_mutex_unlock(&writePizzaLocks[order_id]);


end:;
    // pthread_mutex_unlock(&orderLocks[order_id][pizza_id + 1]);
    if (rejectedDueToTime == 0)
    {

        pthread_mutex_lock(&orderLocks[order_id]);
        orderArr[order_id].pizzasProcessed += 1;
        if (orderArr[order_id].pizzasProcessed == orderArr[order_id].numOfPizzas)
        {
            pthread_cond_signal(&orderCond[order_id]);
        }
        pthread_mutex_unlock(&orderLocks[order_id]);
    }
    else if (rejectedDueToTime == 1)
    {
        pthread_mutex_lock(&pizzaQueue);
        // printf("%d\n", orderArr[id].numOfPizzas);

        // printf("#########\n%d %d\n", orderArr[id].pizzas[i].id, id);

        enqueue(pizza_id, order_id);
        // printf("enqyey due to time\n");
        // display();
        // added
        pthread_mutex_unlock(&pizzaQueue);
        // pthread_cond_signal(&chefCondforOven);
        pthread_mutex_unlock(&chefLocks[id]);
        // pthread_mutex_lock(&restaurantCheck);
        // chefsWorking--;
        // activeChefs--;
        // if (chefsWorking == 0)
        // {
        //     for (int i = 0; i < totCustomers; i++)
        //     {
        //         pthread_cond_signal(&orderCond[i]);
        //     }
        // }
        // clock_gettime(CLOCK_REALTIME, &end);
        // int endTime = end.tv_sec;

        // printf(BLU "Chef %d exits at time %d\n" reset, id, endTime - globalstart);

        // pthread_mutex_unlock(&restaurantCheck);
        // return NULL;
    }
    pthread_cond_signal(&chefCondforOven);
    pthread_mutex_unlock(&chefLocks[id]);
    // end condition to check for time
    clock_gettime(CLOCK_REALTIME, &end);
    int endTime = end.tv_sec;
    if ((endTime - globalstart) <= chefArr[id].exitTime)
    {
        goto start;
    }

    pthread_mutex_lock(&restaurantCheck);
    chefsWorking--;
    activeChefs--;
    if (chefsWorking == 0)
    {
        // for (int i = 0; i < totCustomers; i++)
        // {
        //     pthread_cond_broadcast(&orderCond[i]);
        // }
        for (int i = 0; i < totCustomers; i++)
        {
            pthread_cond_signal(&orderCond[i]);
        }
    }
    clock_gettime(CLOCK_REALTIME, &end);
     endTime = end.tv_sec;

    printf(BLU "Chef %d exits at time %d\n" reset, id+1, chefArr[id].exitTime);
    pthread_mutex_unlock(&restaurantCheck);
}

void *personFunc(void *idx)
{

    int id = *(int *)idx;
    sleep(orderArr[id].arrivalTime);
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);
    int time = end.tv_sec;

    printf(YEL "Customer %d arrived at time %d\n", id+1, orderArr[id].arrivalTime);
    pthread_mutex_lock(&driveQueueThread);

    if (driveThroughPeople < driveThroughCap)
    {
        driveThroughPeople++;
        pthread_mutex_unlock(&driveQueueThread);
    }
    else
    {
        while (driveThroughPeople >= driveThroughCap) // wait until drive through queue clears
            pthread_cond_wait(&driveNext, &driveQueueThread);
        pthread_mutex_unlock(&driveQueueThread);
    }
    pthread_mutex_lock(&restaurantCheck);
    if (chefsWorking == 0)
    {
        printf(RED "Customer %d rejected.\nCustomer %d exits the drive-thru zone.\n" reset, id+1, id+1);
        pthread_mutex_unlock(&restaurantCheck);

        return NULL;
    }
    pthread_mutex_unlock(&restaurantCheck);

    clock_gettime(CLOCK_REALTIME, &end);
    time = end.tv_sec;
    printf(YEL "Customer %d in queue at time %d\n", id+1, time - globalstart-chefArr[id].arrivalTime);
    printf(RED "Order %d placed by customer %d has pizzas ", id+1, id+1);
    pthread_mutex_lock(&orderLocks[id]);
    for (int i = 0; i < orderArr[id].numOfPizzas; i++)
    {
        printf("%d ", orderArr[id].pizzas[i].id+1);
        orderArr[id].status = 'A';
    }
    printf("\n" reset);
    pthread_mutex_lock(&pizzaQueue);
    // printf("%d\n", orderArr[id].numOfPizzas);
    for (int i = 0; i < orderArr[id].numOfPizzas; i++)
    {
        enqueue(orderArr[id].pizzas[i].id - 1, id);
        // printf("enqyey\n");
        // display();
    }
    pthread_mutex_unlock(&pizzaQueue);
    // pthread_mutex_unlock(&orderLocks[id][0]);
    printf(RED "Order %d placed by customer %d awaits processing\n" reset, id+1, id+1);

    sleep(timeToReachPickup);

    // now driver Queue has cleared and hence we will add this code here
    pthread_mutex_lock(&driveQueueThread);
    driveThroughPeople--;
    if (driveThroughPeople < driveThroughCap)
    {
        pthread_cond_signal(&driveNext);
    }
    pthread_mutex_unlock(&driveQueueThread);

L1:
    pthread_mutex_lock(&restaurantCheck);

    if (chefsWorking == 0)
    {
        pthread_mutex_unlock(&orderLocks[id]);
        printf(RED "Customer %d rejected.\nCustomer %d exits the drive-thru zone.\n" reset, id+1, id+1);

        pthread_mutex_unlock(&restaurantCheck);

        return NULL;
    }
    pthread_mutex_unlock(&restaurantCheck);

    // check this, signal here.
    while (orderArr[id].pizzasProcessed != orderArr[id].numOfPizzas && chefsWorking != 0)
    {
        pthread_cond_broadcast(&chefCond);
        pthread_cond_wait(&orderCond[id], &orderLocks[id]); // check if chef available or not where this?
    }
    pthread_mutex_lock(&restaurantCheck);
    if (chefsWorking == 0)
    {
        printf(YEL "Customer %d exits the drive-thru.\n" reset, id+1);

        pthread_mutex_unlock(&orderLocks[id]);

        pthread_mutex_unlock(&restaurantCheck);

        return NULL;
    }
    pthread_mutex_unlock(&restaurantCheck);

    if (checkRejection(id) == 1)
    {
        if (orderArr[id].status == 'P')
        {
            printf(RED "Order %d placed by customer %d partially processed and remaining couldn't be.\n", id+1, id+1);
        }
        else if (orderArr[id].status == 'C')
        {
            printf(RED "Order %d placed by customer %d has been processed.\n" reset, id+1, id+1);
        }
        else if (orderArr[id].status == 'R')
        {
            pthread_mutex_unlock(&orderLocks[id]);
            printf(YEL "Customer %d rejected.\nCustomer %d exits the drive-thru zone.\n" reset, id+1, id+1);

            return NULL;
        }
    }
    printf(YEL "Customer %d exits the drive-thru.\n" reset, id)+1;

    pthread_mutex_unlock(&orderLocks[id]);

    return NULL;
}

int main()
{
    srand(time(0));
    // pass id with n-1
    input();
    printf("Simulation Started\n");
    for (int i = 0; i < totCustomers; i++)
    {
        int *idx = (int *)malloc(sizeof(int));
        *idx = i;
        pthread_create(&orderThreads[i], NULL, personFunc, (void *)idx);
    }
    for (int i = 0; i < totChefs; i++)
    {
        int *idx = (int *)malloc(sizeof(int));
        *idx = i;
        pthread_create(&chefThreads[i], NULL, chefFunc, (void *)idx);
    }
    for (int i = 0; i < totCustomers; i++)
    {

        pthread_join(orderThreads[i], NULL);
    }
    for (int i = 0; i < totChefs; i++)
    {

        pthread_join(chefThreads[i], NULL);
    }
        printf("Simulation Ended\n");

}
// hojaega, make a new variable pizzasDone and do it carefully...rest will be handled by makig one rejection function and reject them all
/*
3 3 3 4 5 3 1 5 3 1 2 3 2 5 2 2 3 3 5 0 10 5 2 0 50 20 60 30 120 0 2 1 2 1 1 1 2 2 1 2 4 1 3
3 3 3 4 5 3
1 5 3 1 2 3
2 5 2 2 3
3 5 0
10 5 3
0 2 20 60 30 120
0 2 1 2
1 1 1
2 2 1 2
4 1 3
*/

/*

 python3 pipeline.py  --model_name_or_path "google/mt5-base" --data_dir "../../XLSum_complete_v2.0" --output_dir "../../XLSum_output"  --lr_scheduler_type="transformer"  --learning_rate=1  --warmup_steps 5000  --weight_decay 0.01   --per_device_train_batch_size=2  --gradient_accumulation_steps=16   --max_steps 50000 --save_steps 5000  --evaluation_strategy "no"  --logging_first_step  --adafactor  --label_smoothing_factor 0.1  --upsampling_factor 0.5  --do_train
*/