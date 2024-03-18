#ifndef __REQUEST_H__
#define __REQUEST_H__ // <!<!<!<!<!<!<!<!<!<!<!<!<!<!<!<! ADDED THIS LINE (#define __REQUEST_H__)

// <@<@<@<@<@<@<@<@<@<@<@<@<@<@<@<@<@<@<@ ADDED THIS STRUCT
/** 
*    ThreadInfo -
*        Holds statistics of a single thread.
**/
typedef struct {
   /* 
    *   id -
    *       The id assigned to this thread by the
    *       server's main thread.
    */
	int id;
    /* 
    *   num_static_requests_served -
    *       Counts the number of static requests
    *       successfully served by this thread.
    */
	int num_static_requests_served;
    /* 
    *   num_dynamic_requests_served -
    *       Counts the number of dynamic requests
    *       successfully served by this thread.
    */
	int num_dynamic_requests_served;
    /* 
    *   num_total_requests_recieved -
    *       Counts the total number of requests recieved by this thread
    *       (not necessarily successfully served). 
    */
	int num_total_requests_recieved;
} ThreadInfo;


void requestHandle(int fd, struct timeval arrival_time, struct timeval dispatch_time, ThreadInfo *p_thread_info);


#endif
