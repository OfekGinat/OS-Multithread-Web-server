#include "segel.h"
#include "request.h"

#include "queue.h"

#define SCHEDALG_STR_MAX_LEN 10 // <@<@<@<@ is actually 6 + 1 = 7 for "random"


// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too
void getargs(int *port, int *num_threads, int *queue_size,
             char schedalg[SCHEDALG_STR_MAX_LEN], int argc, char *argv[]) // <@<@<@<@ added arguments
{
    if (argc < 5) {
	    fprintf(stderr, "Usage: %s <port> <number of threads> <queue size> <scheduling algorithm>\n", argv[0]); 
	    exit(1);
    }
    *port = atoi(argv[1]);
    /*@>@>@>@>~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    *num_threads = atoi(argv[2]);
    if (*num_threads < 0) {
        fprintf(stderr, "Number of threads must be greater than zero\n"); 
	    exit(1);
    }
    *queue_size = atoi(argv[3]);
    if (*queue_size < 0) {
        fprintf(stderr, "Requests queue size must be greater than zero\n"); 
	    exit(1);
    }
    strcpy(schedalg, argv[4]);
    /*<@<@<@<@~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
}


/*@>@>@>@>~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void* thread_handle_request(void* arg) // <@<@<@<@ change "arg" according to actual argument when needed
{
    while (1) {
        RequestInfo request_info = queue_pop_front();

        requestHandle(request_info.connfd);

        queue_dec_num_requests();

	    Close(request_info.connfd); 

        //queue_dec_num_requests();
    }
}
/*<@<@<@<@~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, num_threads, queue_size; // <@<@<@<@ added "num_threads, queue_size"
    char schedalg[SCHEDALG_STR_MAX_LEN]; // <@<@<@<@ added schedalg
    struct sockaddr_in clientaddr;
    RequestInfo request_info; // <@<@<@<@ added request_info

    getargs(&port, &num_threads, &queue_size, schedalg, argc, argv); // <@<@<@<@ added "&num_threads, &queue_size, schedalg,"

    // 
    // HW3: Create some threads...
    //
    /*@>@>@>@>~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    queue_init(queue_size, schedalg);

    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    if (threads == NULL) {
        unix_error("malloc error");
    }

    int _pthread_error_code = 0;
    for (int i = 0; i < num_threads; ++i) {
        _pthread_error_code = pthread_create(&threads[i], NULL, thread_handle_request, NULL /* <- will change! */);
        if(_pthread_error_code != 0) {
            posix_error(_pthread_error_code, "pthread_create error");
        }
    }
    /*<@<@<@<@~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    listenfd = Open_listenfd(port);
    while (1) {
	    clientlen = sizeof(clientaddr);
	    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
	// 
	// HW3: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. 
	// 
        /*@>@>@>@>~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        request_info.connfd = connfd; // <!<!<!<!<!<! NOTE: DON'T FORGET TO SET *ALL* OF request_info'S FIELDS EVERY ITERATION!
        /* 
            more info like arrival_time (check gettimeofday() for failure!)... 
        */ 
        queue_push_back(request_info);
        /*<@<@<@<@~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	    //requestHandle(connfd); // <@<@<@<@ put in comment
	    //Close(connfd); // <@<@<@<@ put in comment
    }
}


    


 
