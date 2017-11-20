#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include "queue.h"

#define NUM_CLERKS 2
#define NUM_QUEUES 4

typedef struct {
  int id;
  bool busy;
} clerk;

typedef struct {
  int id; // id of customer 
  int idx; // customers[idx] = this customer
  int atime, stime; // arrival, service time in microseconds
  int clerk; // clerk that serviced customer, -1 if not set
  double start_wait_time, end_wait_time; 
} customer;

customer* customers;

queue queues[NUM_QUEUES];
pthread_cond_t queue_convars[NUM_QUEUES];
pthread_mutex_t queue_mutex;

clerk clerks[NUM_CLERKS];
pthread_mutex_t clerk_mutex[NUM_CLERKS];
pthread_cond_t clerk_busy[NUM_CLERKS];

struct timeval start_time;


double get_curr_time() {
  struct timeval curr_time;
	double curr_secs, init_secs;
	init_secs = start_time.tv_sec + (double) start_time.tv_usec/1000000;
	gettimeofday(&curr_time, NULL);
	curr_secs = curr_time.tv_sec + (double) curr_time.tv_usec/1000000;
	return curr_secs - init_secs;
}

int pick_queue(bool min) {
  int ties[NUM_QUEUES];
  int i, j = 0;
  for (i = 0; i < NUM_QUEUES; i++) 
    if (min && queues[i].size < queues[j].size)
      j = i;
    else if (!min && queues[i].size > queues[j].size)
      j = i;
  
  int num_ties = 0;
  for (i = 0; i < NUM_QUEUES; i++)
    if (queues[i].size == queues[j].size)
      ties[num_ties++] = i;

  return ties[rand() % num_ties];
}

void* customer_start(void* args) {
  int cust_idx = *((int*) args);
  customer* cust = &customers[cust_idx];

  usleep(cust->atime);
  printf("A customer arrives: customer ID %2d. \n", cust->id);
  
  pthread_mutex_lock(&queue_mutex); 
  {
    int qmin = pick_queue(true);
        
    queue_push(&queues[qmin], cust_idx);

    printf("A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", 
        qmin, queues[qmin].size);
    cust->start_wait_time = get_curr_time();

    while (cust->clerk == -1) 
      pthread_cond_wait(&queue_convars[qmin], &queue_mutex);
  
    cust->end_wait_time = get_curr_time();
     printf("A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n",
        cust->end_wait_time, cust->id, cust->clerk);
  }
  pthread_mutex_unlock(&queue_mutex);
  
  usleep(cust->stime);
  printf("A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n",
      get_curr_time(), cust->id, cust->clerk);

  int clerk_id = cust->clerk;
  pthread_mutex_lock(&clerk_mutex[clerk_id]);
  {
    clerks[clerk_id].busy = false;
    pthread_cond_signal(&clerk_busy[clerk_id]);
  }
  pthread_mutex_unlock(&clerk_mutex[clerk_id]);

  return NULL;
}

void* clerk_start(void* args) {
  int clerk_id = *((int*) args);
  
  while (true) {
    pthread_mutex_lock(&queue_mutex);
    int imax = pick_queue(false);

    if (queues[imax].size > 0) {
      int cust_idx = queue_pop(&queues[imax]);
      customers[cust_idx].clerk = clerk_id;
      clerks[clerk_id].busy = true;
      pthread_cond_broadcast(&queue_convars[imax]);
      pthread_mutex_unlock(&queue_mutex);
    } else {
      pthread_mutex_unlock(&queue_mutex);
      usleep(250); // idle 1/4 ms
    }

    pthread_mutex_lock(&clerk_mutex[clerk_id]);
    {
      if (clerks[clerk_id].busy)
        pthread_cond_wait(&clerk_busy[clerk_id], &clerk_mutex[clerk_id]);
    }
    pthread_mutex_unlock(&clerk_mutex[clerk_id]);
  }

  return NULL;
}


void init_globals() {
  int i;
  gettimeofday(&start_time, NULL); 
  srand(time(0)); // seed random numbers

  // init queues
  pthread_mutex_init(&queue_mutex, NULL);
  for (i = 0; i < NUM_QUEUES; i++) {
    queue_init(&queues[i]);
    pthread_cond_init(&queue_convars[i], NULL);
  }

  // initialize clerk globals
  for (i = 0; i < NUM_CLERKS; i++) {
    clerks[i].id = i;
    pthread_mutex_init(&clerk_mutex[i], NULL);
    pthread_cond_init(&clerk_busy[i], NULL);
  }
}

void destroy_globals() {
  int i;
  
  // destroy queues
  pthread_mutex_destroy(&queue_mutex);
  for (i = 0; i < NUM_QUEUES; i++) {
    queue_end(&queues[i]);
    pthread_cond_destroy(&queue_convars[i]);
  }

  // destroy clerk globals
  for (i = 0; i < NUM_CLERKS; i++) {
    pthread_mutex_destroy(&clerk_mutex[i]);
    pthread_cond_destroy(&clerk_busy[i]);
  }

  free(customers);
}

int main(int argc, char *argv[]) {
  int i, num_customers, valid_customers;
  
  if (argc != 2) {
    fprintf(stderr, "usage: ACS <file>\n");
    return EXIT_FAILURE;
  }

  FILE* fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "Cannot open file %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  init_globals();

  fscanf(fp, "%d", &num_customers);
  if (num_customers <= 0) {
    fprintf(stderr, "Error. Number of customers must be positive\n");
    return EXIT_FAILURE;
  }

  customers = (customer*) malloc(sizeof (customer) * num_customers);
  customer customer;
  for (i = 0, valid_customers = 0; i < num_customers; i++) {
    fscanf(fp, "%d:%d,%d", &customer.id, &customer.atime, &customer.stime);
    if (customer.id < 0 || customer.atime < 0 || customer.stime < 0) {
      fprintf(stderr, "Error. Skipping customer %d with negative value\n", customer.id);
    } else {
      customer.stime *= 100000; // convert 10th of a second to microseconds
      customer.atime *= 100000;
      customer.clerk = -1;
      customer.idx = valid_customers;
      customers[valid_customers++] = customer;
    }
  }
  num_customers = valid_customers;

  // spawn clerk threads
  for (i = 0; i < NUM_CLERKS; i++) {
    pthread_t thread;
    if (pthread_create(&thread, NULL, clerk_start, (void*) &clerks[i].id) != 0) {
      fprintf(stderr, "Error with pthread_create\n");
      return EXIT_FAILURE;
    }
  }

  // spawn customer threads
  pthread_t customer_threads[num_customers];
  for (i = 0; i < num_customers; i++) {
    if (pthread_create(&customer_threads[i], NULL, customer_start, (void*) &customers[i].idx) != 0) {
      fprintf(stderr, "Error with pthread_create\n");
      return EXIT_FAILURE;
    }
  }
  
  // wait for threads to terminate
  for (i = 0; i < num_customers; i++) {
    pthread_join(customer_threads[i], NULL);
  }

  double avg_wait = 0.0;
  for (i = 0; i < num_customers; i++) {
    avg_wait += customers[i].end_wait_time - customers[i].start_wait_time;
  }
  avg_wait /= num_customers;
  printf("The average waiting time for all customers in the system is: %.2f seconds. \n", avg_wait);

  destroy_globals();
  fclose(fp);

  return EXIT_SUCCESS;
} 


