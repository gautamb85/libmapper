#include "../src/mapper_internal.h"
#include <mapper/mapper.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>

#ifdef WIN32
#define usleep(x) Sleep(x/1000)
#endif

//changes made in the branch
//structure defining a signal queue
typedef struct mqueue
{
	int capacity;
	int size;
	int front;
	int rear;
	mapper_signal *elements;
}mqueue;

//function to create a queue of signals

mqueue * createQueue(int max)
{
	mqueue *q;
	q = (mqueue *)malloc(sizeof(mqueue));
	q->elements = (mapper_signal *)malloc(sizeof(mapper_signal)*max);
	q->size=0;
	q->capacity = max;
	q->front = 0;
	q->rear = -1;
	
	return q;
}

void enqueue(mqueue *q,mapper_signal sig)
{
	/* If the Queue is full, we cannot push an element into it as there is no space for it.*/
        if(q->size == q->capacity)
        {
                printf("Queue is Full\n");
        }
        else
        {
                q->size++;
                q->rear = q->rear + 1;
                /* As we fill the queue in circular fashion */
                if(q->rear == q->capacity)
                {
                        q->rear = 0;
                }
                /* Insert the element in its rear side */ 
                q->elements[q->rear] = sig;
        }
}

void dequeue(mqueue *q)
{
	if (q->size==0)
	{
		printf("Queue is empty\n");
		return;
	}
	
	else
	{
		q->size++;
		q->front++;
		
		//As we fill elements in a circular fashion
		
		if(q->front == q->capacity)
		q->front=0;

	}
	return;
}

mapper_signal front(mqueue *q)
{
		if (q->size==0)
	{
		printf("Queue is empty\n");
		exit(0);
	}
	
	return q->elements[q->front];
}

//function to update a mapper queue for floating point values
void queue_update(mqueue *q, float value)
{
	int s = q->size;

	for(int i=0;i<s;i++)
	{
		mapper_signal sig = 0;
		sig = front(q);
		dequeue(q);
    	memcpy(sig->value,&value, msig_vector_bytes(sig));
    	sig->props.has_value = 1;
   		
		if (sig->props.is_output)
		mdev_route_signal(sig->device, sig, (mapper_signal_value_t*)&value);
		

	}

}

//function to send a OSC - bundled mapper queue
//the function could have a timetag as an optional argument
//otherwise the bundle is given the current system time
void send_mapper_queue(mqueue *q)
{
	lo_timetag now;
	//set the now timetag to the current system time (sampling time??)
	lo_timetag_now(&now);
	//create an osc bundle to send the mapper queue
	//Each signal in the mapper queue is given the 
	lo_bundle mb = lo_bundle_new(now);	

mapper_device source = 0;
mapper_device destination = 0;
mapper_router router = 0;
mapper_signal sendsig = 0;
mapper_signal sendsig1 = 0;
mapper_signal sendsig2 = 0;
mapper_signal recvsig = 0;
mapper_signal recvsig1 = 0;
mapper_signal recvsig2 = 0;

mqueue *n;
mapper_signal fr;
int port = 9000;

int sent = 0;
int received = 0;

int setup_source()
{
    source = mdev_new("testsend", port, 0);
    if (!source)
        goto error;
    printf("source created.\n");
	lo_timetag tt;
	tt.sec = 0;
	tt.frac = 2;	

    float mn=0, mx=1;
	
	sendsig = mdev_add_output(source, "/outsig", 1, 'f', 0, &mn, &mx);
	sendsig1 = mdev_add_output(source, "/outsig1", 1, 'f', 0, &mn, &mx);
	sendsig2 = mdev_add_output(source, "/outsig2", 1, 'f', 0, &mn, &mx);

	//create a signal queue of 3 elements
	n = createQueue(3);
	
	//enqueue the 3 signals assosiated with the device - testsend
	enqueue(n,sendsig);
	enqueue(n,sendsig1);
	enqueue(n,sendsig2);

	source->outputs[0]->value_tt = tt;
	lo_arg_pp(LO_TIMETAG, &(source->outputs[0]->value_tt));
    printf("\n");
	printf("Output signal /outsig registered.\n");
	
    printf("Number of outputs: %d\n", mdev_num_outputs(source));
    return 0;

  error:
    return 1;
}

void cleanup_source()
{
    if (source) {
        if (router) {
            printf("Removing router.. ");
            fflush(stdout);
            mdev_remove_router(source, router);
            printf("ok\n");
        }
        printf("Freeing source.. ");
        fflush(stdout);
        mdev_free(source);
        printf("ok\n");
    }
}

void insig_handler(mapper_signal sig, mapper_db_signal props,
                   mapper_timetag_t *timetag, void *value)
{
    if (value) {
        lo_arg_pp(LO_TIMETAG, timetag);
		printf("  This is the time\n");
		printf("handler: Got %f\n", (*(float*)value));
	}
    received++;
}

int setup_destination()
{
    destination = mdev_new("testrecv", port, 0);
    if (!destination)
        goto error;
    printf("destination created.\n");

    float mn=0, mx=1;

    recvsig = mdev_add_input(destination, "/insig", 1, 'f', 0,
                             &mn, &mx, insig_handler, 0);
	recvsig1 = mdev_add_input(destination, "/insig1", 1, 'f', 0,
                             &mn, &mx, insig_handler, 0);
	recvsig2 = mdev_add_input(destination, "/insig2", 1, 'f', 0,
                             &mn, &mx, insig_handler, 0);

    printf("Input signal /insig registered.\n");
    printf("Number of inputs: %d\n", mdev_num_inputs(destination));
    return 0;

  error:
    return 1;
}

void cleanup_destination()
{
    if (destination) {
        printf("Freeing destination.. ");
        fflush(stdout);
        mdev_free(destination);
        printf("ok\n");
    }
}

int setup_router()
{
    const char *host = "localhost";
    router = mapper_router_new(source, host, destination->admin->port.value,
                               mdev_name(destination));
    mdev_add_router(source, router);
    printf("Router to %s:%d added.\n", host, port);

    char signame_in1[1024];
    if (!msig_full_name(recvsig, signame_in1, 1024)) {
        printf("Could not get destination signal name.\n");
        return 1;
    }

    char signame_out1[1024];
    if (!msig_full_name(sendsig, signame_out1, 1024)) {
        printf("Could not get source signal name.\n");
        return 1;
    }
	
	 char signame_in2[1024];
    if (!msig_full_name(recvsig1, signame_in2, 1024)) {
        printf("Could not get destination signal name.\n");
        return 1;
    }

    char signame_out2[1024];
    if (!msig_full_name(sendsig1, signame_out2, 1024)) {
        printf("Could not get source signal name.\n");
        return 1;
    }
	 char signame_in3[1024];
     if (!msig_full_name(recvsig2, signame_in3, 1024)) {
        printf("Could not get destination signal name.\n");
        return 1;
    }

    char signame_out3[1024];
    if (!msig_full_name(sendsig2, signame_out3, 1024)) {
        printf("Could not get source signal name.\n");
        return 1;
    }

    printf("Connecting signal %s -> %s\n", signame_out1, signame_in1);
    mapper_connection c = mapper_router_add_connection(router, sendsig,
                                                       recvsig->props.name,
                                                       'f', 1);
    
    printf("Connecting signal %s -> %s\n", signame_out2, signame_in2);
    mapper_connection c1 = mapper_router_add_connection(router, sendsig1,
                                                       recvsig1->props.name,
                                                       'f', 1);
	
    printf("Connecting signal %s -> %s\n", signame_out3, signame_in3);
    mapper_connection c2 = mapper_router_add_connection(router, sendsig2,
                                                       recvsig2->props.name,
                                                       'f', 1);
	mapper_connection_range_t range;
    range.src_min = 0;
    range.src_max = 1;
    range.dest_min = -10;
    range.dest_max = 10;
    range.known = CONNECTION_RANGE_KNOWN;
    
    mapper_connection_set_linear_range(c, sendsig, &range);
	mapper_connection_set_linear_range(c1, sendsig1, &range);
	mapper_connection_set_linear_range(c2, sendsig2, &range);

    return 0;
}

void wait_ready()
{
    while (!(mdev_ready(source) && mdev_ready(destination))) {
        mdev_poll(source, 0);
        mdev_poll(destination, 0);
        usleep(500 * 1000);
    }
}

void loop()
{
    printf("Polling device..\n");
    int i;
    for (i = 0; i < 10; i++) {
        mdev_poll(source, 0);
        printf("Updating signal %s to %f\n",
               sendsig->props.name, (i * 1.0f));
	    printf("Updating signal %s to %f\n",
               sendsig1->props.name, (i * 1.0f));
	    printf("Updating signal %s to %f\n",
               sendsig2->props.name, (i * 1.0f));

        queue_update(n, (i*1.0f));
        sent++;
        usleep(250 * 1000);
        mdev_poll(destination, 0);
    }
}

int main()
{
    int result = 0;

    if (setup_destination()) {
        printf("Error initializing destination.\n");
        result = 1;
        goto done;
    }

    if (setup_source()) {
        printf("Done initializing source.\n");
        result = 1;
        goto done;
    }

    wait_ready();

    if (setup_router()) {
        printf("Error initializing router.\n");
        result = 1;
        goto done;
    }

    loop();

    if (sent != received) {
        printf("Not all sent messages were received.\n");
        printf("Updated value %d time%s, but received %d of them.\n",
               sent, sent == 1 ? "" : "s", received);
        result = 1;
    }

  done:
    cleanup_destination();
    cleanup_source();
    printf("Test %s.\n", result ? "FAILED" : "PASSED");
    return result;
}
