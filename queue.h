#ifndef _QUEUE_H_
#define _QUEUE_H_

//#include "segel.h"


/** 
*    RequestInfo -
*        An element of the queue.
*        Holds client's request information.
**/
typedef struct {
    int connfd;
    // >>>>>>>>>> NOTE: WE'LL HAVE TO ADD FIELDS FOR INFO 
} RequestInfo;



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                         Queue interface functions 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
*   Initialize the queue with desired maximum number
*   of simultanious requests and overload handling policy.
*   Call only after checking that all command line 
*   arguments are valid!
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