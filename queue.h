#ifndef __QUEUE_H__
#define __QUEUE_H__

//#include "segel.h"
#include <sys/time.h>

/** 
*    RequestInfo -
*        An element of the queue.
*        Holds client's request information.
**/
typedef struct {
    int connfd;
    struct timeval arrival_time; // <!<!<!<!<!<!<!<!<!<!<! ADDED THIS
    // >>>>>>>>>> NOTE: WE'LL HAVE TO ADD FIELDS FOR INFO 
} RequestInfo;



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                         Queue interface functions 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
*   Initialize the queue with desired maximum number
*   of simultanious requests and overload handling policy.
*   max_requests must be a positive integer, and schedalg
*   must be one of "block", "dt" or "dh".
**/
void queue_init(int max_requests, char* schedalg);

/**
*   Insert a new request at the back of the queue.
**/
void queue_push_back(RequestInfo request_info);

/**
*   Remove the oldest request in the queue and
*   mark it as being handled.
**/
RequestInfo queue_pop_front(); 

/**
*    When done handling a request, a worker thread
*    should call this function to decrease the number
*    of requests currently being handled.
**/
void queue_dec_num_requests();


#endif