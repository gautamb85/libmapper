
#include "../src/mapper_internal.h"
#include <mapper/mapper.h>
#include <stdio.h>
#include <math.h>
#include <lo/lo.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#ifdef WIN32
#define usleep(x) Sleep(x/1000)
#endif

int automate = 1;

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

mapper_device source = 0;
mapper_device destination = 0;
mapper_signal sendsig_1 = 0;
mapper_signal recvsig_1 = 0;
mapper_signal sendsig_2 = 0;
mapper_signal recvsig_2 = 0;
mapper_signal sendsig_3 = 0;
mapper_signal recvsig_3 = 0;
mapper_signal sendsig_4 = 0;
mapper_signal recvsig_4 = 0;
mqueue *n;
int port = 9000;

int sent = 0;
int received = 0;
int done = 0;

/*! Creation of a local source. */
int setup_source()
{
    source = mdev_new("testsend", port, 0);
    if (!source)
        goto error;
    printf("source created.\n");

    float mn=0, mx=10;

    sendsig_1 = mdev_add_output(source, "/outsig_1", 1, 'f', "Hz", &mn, &mx);
    sendsig_2 = mdev_add_output(source, "/outsig_2", 1, 'f', "mm", &mn, &mx);
    sendsig_3 = mdev_add_output(source, "/outsig_3", 3, 'f', 0, &mn, &mx);
    sendsig_4 = mdev_add_output(source, "/outsig_4", 1, 'f', 0, &mn, &mx);

	n = createQueue(4);
	enqueue(n,sendsig_1);
    enqueue(n,sendsig_2);
	enqueue(n,sendsig_3);
	enqueue(n,sendsig_4);

	printf("Output signal /outsig registered.\n");


    // Make sure we can add and remove outputs without crashing.
    mdev_remove_output(source, mdev_add_output(source, "/outsig_5", 1,
                                               'f', 0, &mn, &mx));

    printf("Number of outputs: %d\n", mdev_num_outputs(source));

    return 0;

  error:
    return 1;
}

void cleanup_source()
{
    if (source) {
        if (source->routers) {
            printf("Removing router.. ");
            fflush(stdout);
            mdev_remove_router(source, source->routers);
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
        printf("--> destination got %s", props->name);
        float *v = value;
		 for (int i = 0; i < props->length; i++) {
            printf(" %f", v[i]);
	    	
        }
        printf("\n");
    }
    received++;
}

/*! Creation of a local destination. */
int setup_destination()
{
    destination = mdev_new("testrecv", port, 0);
    if (!destination)
        goto error;
    printf("destination created.\n");

    float mn=0, mx=1;

    recvsig_1 = mdev_add_input(destination, "/insig_1", 1, 'f',
                               0, &mn, &mx, insig_handler, 0);
    recvsig_2 = mdev_add_input(destination, "/insig_2", 1, 'f',
                               0, &mn, &mx, insig_handler, 0);
    recvsig_3 = mdev_add_input(destination, "/insig_3", 3, 'f',
                               0, &mn, &mx, insig_handler, 0);
    recvsig_4 = mdev_add_input(destination, "/insig_4", 1, 'f',
                               0, &mn, &mx, insig_handler, 0);

    printf("Input signal /insig registered.\n");

    // Make sure we can add and remove inputs and inputs within crashing.
    mdev_remove_input(destination,
                      mdev_add_input(destination, "/insig_5", 1,
                                     'f', 0, &mn, &mx, 0, 0));

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



void wait_local_devices()
{
    while (!(mdev_ready(source) && mdev_ready(destination))) {
        mdev_poll(source, 0);
        mdev_poll(destination, 0);

        usleep(50 * 1000);
    }
}

void loop()
{
    printf("-------------------- GO ! --------------------\n");
    int i = 0;

    if (automate) {
        char source_name_1[1024], destination_name_1[1024];
        char source_name_2[1024], destination_name_2[1024];
		char source_name_3[1024], destination_name_3[1024];

		
        printf("%s\n", mdev_name(source));
        printf("%s\n", mdev_name(destination));

        lo_address a = lo_address_new_from_url("osc.udp://224.0.1.3:7570");
        lo_address_set_ttl(a, 1);

        lo_send(a, "/link", "ss", mdev_name(source), mdev_name(destination));

        msig_full_name(sendsig_1, source_name_1, 1024);
        msig_full_name(recvsig_1, destination_name_1, 1024);

        lo_send(a, "/connect", "ss", source_name_1, destination_name_1);

        msig_full_name(sendsig_2, source_name_2, 1024);
        msig_full_name(recvsig_2, destination_name_2, 1024);

        lo_send(a, "/connect", "ss", source_name_2, destination_name_2);

	    msig_full_name(sendsig_3, source_name_3, 1024);
        msig_full_name(recvsig_3, destination_name_3, 1024);

        lo_send(a, "/connect", "ss", source_name_3, destination_name_3);

        lo_address_free(a);
    }

    while (i >= 0 && !done) {
        mdev_poll(source, 0);
        queue_update(n, ((i % 10) * 1.0f));
        printf("source value updated to %d -->\n", i % 10);

        printf("Received %i messages.\n\n", mdev_poll(destination, 100));
        i++;
    }
}

void ctrlc(int sig)
{
    done = 1;
}

int main()
{
    int result = 0;

    signal(SIGINT, ctrlc);

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

    wait_local_devices();

    loop();

  done:
    cleanup_destination();
    cleanup_source();
    return result;
}
