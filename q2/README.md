- First input is used.
- Used pthread create to create all threads at once 
- A thread was made for each chef.
- When using the chef thread feature:
  Chef rests in preparation for his arrival; uses pthread cond timedwait() to determine whether a pizza needs to be handled; and exits if no pizza is assigned and the chef has reached his departure.
  _time frame
- give the chef the pizza; - leave if the time required to prepare the pizza will take longer than the chef's exit time
- begin making the pizza; - begin gathering the ingredients; - stop the function if there are not enough ingredients; - designate an oven for the pizza. Pthread cond timedwait() is utilised if oven is not immediately available.
- Stop the function if an oven is not allocated - Start making the pizza in the oven
- snooze while the pizza cooks

-Now, if the pizza order was turned down for lack of time, we re-add it to the queue so that another chef might be able to prepare it. If there is still time before the chef leaves, we return to the beginning of the loop (where pizza assignment takes place).
extinguish the function

- A thread was made for every customer.
- snooze until the customer arrives; if the drive-thru limit is exceeded, we wait until someone leaves (impossible due to the latitude offered by TCS); the customer leaves if there are no chefs present at the time of arrival; otherwise, the order is placed and printed on terminal; a queue is maintained for the orders; and once more, the customer enters.
  exits if there are no chefs working
  - otherwise, a signal is broadcast to all the chef threads to start preparing the order
  - the customer conditionally waits till the order prepares
  - check for order status of the customer and print it on the terminal
  - exit the function

- Joined all threads using pthread_join()

Followup Questions

1. Semaphores are a possibility. They are introduced to the principal storage benefit, namely, the ability to store pizzas. We employ the *sem trywait()* method each time the chef want to place the pizza. If the semaphore value is greater than 0, this method will allow entry; otherwise, it will block until the value turns positive.

2. In the restaurant, orders may be cancelled in the middle of processing for the following reasons:
3. The pizza cannot be made by the chef since there is not enough time.
4. Insufficient supply of components

5. Drive thru acceptance increases because they can get more ingredients.

The actions listed below can be taken:

Use the idea of a buffer time, let's say x, along with the real pizza preparation time. Then, we determine whether or not we wish to reject an order using the total time (prep time + x). As a result, the restaurant's ratings will rise because more orders will be turned down at the beginning rather than in the middle.

Please take note that after the chef is allocated the pizza, we verify that they have enough ingredients on hand to make the pizza.
We won't eat the pizza if they don't.
Cond timedwait() can be used to wait for a predetermined period of time to see if the ingredients become available, though, if we are permitted to replenish our supplies.
It should be noted that this time, let's say x, multiplied by preparation time, p, cannot exceed the chef's departure time (x + p = exit time). 
