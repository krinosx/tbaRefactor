/*
Removed headers... not used...
Keep it here until the testing script is finished... 
if everything keep working, remove it.

#include "../src/conf.h"
#include "../src/sysdep.h"
#include "../src/structs.h"

#include "../src/db.h"
#include "../src/dg_event.h"
#include "../src/constants.h"
#include "../src/comm.h" /* For access to the game pulse 
#include "../src/mud_event.h"
*/


#include "../src/structs.h"
#include "../src/utils.h"
#include "../src/dg_event.h"
#include "../src/constants.h"
#include "../src/mud_event.h"

#include "CuTest.h"

/*-------------------------------------------------------------------------*
 * CuString Test
 *-------------------------------------------------------------------------*/
 /** Function prototypes for code being tested */
struct dg_queue *queue_init(void);
struct event *event_create_local(struct dg_queue * q, unsigned long current_pulse, EVENTFUNC(*func), void *event_obj, long when);
long queue_key_local(struct dg_queue *q, unsigned long p);
void *queue_head_local(struct dg_queue *q, unsigned long current_pulse);
struct q_element *queue_enq(struct dg_queue *q, void *data, long key);
void queue_deq(struct dg_queue *q, struct q_element *qe);
void event_cancel_local(struct dg_queue * queue, struct event *event);
void cleanup_event_obj(struct event *event);
void event_process_local(const unsigned long current_pulse, struct dg_queue * queue);
long event_time_local(struct event *event, unsigned long pulse);
void queue_free_2(struct dg_queue ** q2);
void queue_free(struct dg_queue *q);


/**
* simple function to create events
*/
static EVENTFUNC(simple_func) {
	int i = 1 + 1;
	return 0l;
}

/**
* Testing code
*/
void test_queue_init(CuTest* tc)
{	
	static struct dg_queue *event_q;
	event_q = queue_init();

	CuAssertPtrNotNullMsg(tc, "Fail to create a queue!", event_q);
	
	// This algorithm must allocate NUM_EVENT_QUEUES elements for the head and tail. So, the head memory size must be "NUM_EVENT_QUEUES x sizeof(event_q->head)"
	int queueHeadSize = (int) (sizeof(event_q->head) / sizeof(event_q->head[0]));
	CuAssertIntEquals_Msg(tc, "Error to allocate dg_queue head elements memory.", NUM_EVENT_QUEUES, queueHeadSize);

	int queueTailSize = (int)(sizeof(event_q->tail) / sizeof(event_q->tail[0]));
	CuAssertIntEquals_Msg(tc, "Error to allocate dg_queue tails elements memory.", NUM_EVENT_QUEUES, queueTailSize);

	// check if all elements are NULL
	for (int i = 0; i < NUM_EVENT_QUEUES; i++) {
		CuAssertPtrEquals_Msg(tc, "Not null bucket allocated for dg_queue->head.", NULL, event_q->head[i]);
		CuAssertPtrEquals_Msg(tc, "Not null bucket allocated for dg_queue->tail.", NULL, event_q->tail[i]);
	}

	/*
	* Release resources
	*/
	queue_free_2(&event_q);
}

void test_event_create(CuTest* tc) {
	static struct dg_queue *event_q;
	event_q = queue_init();
	struct event * mud_event;

	unsigned long currentPulse = 0;

	mud_event = event_create_local(event_q, currentPulse,  simple_func, NULL, 10);

	CuAssertPtrNotNullMsg(tc, "Fail to create event!", mud_event);

	CuAssertPtrNotNullMsg(tc, "Fail to create an event_queue", event_q);
	CuAssertPtrNotNullMsg(tc, "event_queue head was not allocated", event_q->head);
	CuAssertPtrNotNullMsg(tc, "event_queue tail was not allocated", event_q->tail);
	CuAssertPtrNotNullMsg(tc, "event was not set to event_queue->head", event_q->head[0]);
	CuAssertPtrNotNullMsg(tc, "event was not set to event_queue->tail", event_q->tail[0]);

	CuAssertPtrEquals_Msg(tc, "event_queue->head and event_queue->tail is not pointing to same object.", event_q->head[0], event_q->tail[0]);
	CuAssertPtrEquals_Msg(tc, "event_queue->head[0]->data and event_queue->tail[0].data is not pointing to same object!", event_q->head[0]->data, event_q->tail[0]->data);

	CuAssertPtrEquals_Msg(tc, "event_queue->head[0]->next is not null! It must be.", NULL, event_q->head[0]->next);
	CuAssertPtrEquals_Msg(tc, "event_queue->head[0]->prev is not null! It must be.", NULL, event_q->head[0]->prev);
	
	CuAssertPtrEquals_Msg(tc, "event_queue->tail[0]->next is not null! It must be.", NULL, event_q->tail[0]->next);
	CuAssertPtrEquals_Msg(tc, "event_queue->tail[0]->next is not null! It must be.", NULL, event_q->tail[0]->prev);

	CuAssertIntEquals_Msg(tc, "event_queue->head[0]->key dont have the correct value!", 10, event_q->head[0]->key);
	CuAssertIntEquals_Msg(tc, "event_queue->tail[0]->key dont have the correct value!", 10, event_q->tail[0]->key);


	/*
	* Release resources
	*/
	queue_free_2(&event_q);
}

void test_queue_key(CuTest* tc) {

	/*
	* First use case. A empty queue
	*/

	// pulse is originally unsigned.
	unsigned long enqueue_time_pulse = 0l;
	unsigned long retrieve_time_pulse = 0l;
	static struct dg_queue *event_q;
	struct event * mud_event;

	event_q = queue_init();
	
	// MIN para unsigned long
	retrieve_time_pulse = 0l;
	long queue_key = queue_key_local(event_q, retrieve_time_pulse);

	retrieve_time_pulse = ULONG_MAX;
	long queue_key1 = queue_key_local(event_q, retrieve_time_pulse);

	CuAssertLongEquals_Msg(tc, "Wrong value returned with pulse = 0", LONG_MAX, queue_key);
	CuAssertLongEquals_Msg(tc, "Wrong value returned with pulse = ULONG_MAX", LONG_MAX, queue_key1);
	

	/**
	* Seccond use case, testing with a queue with only 1 event with key = 10
	*/

	mud_event = event_create_local(event_q, enqueue_time_pulse, simple_func, NULL, 10);

	retrieve_time_pulse = 0l;
	long queue_key2 = queue_key_local(event_q, retrieve_time_pulse);

	retrieve_time_pulse = 1l;
	long queue_key3 = queue_key_local(event_q, retrieve_time_pulse);

	retrieve_time_pulse = 2l;
	long queue_key4 = queue_key_local(event_q, retrieve_time_pulse);

	CuAssertLongEquals_Msg(tc, "Wrong value returned with pulse = 0", 10, queue_key2);
	CuAssertLongEquals_Msg(tc, "Wrong value returned with pulse = 0", LONG_MAX, queue_key3);
	CuAssertLongEquals_Msg(tc, "Wrong value returned with pulse = 0", LONG_MAX, queue_key4);


	/**
	* Third use case, testing with a queue full of events
	*
	* The setup provide a queue with the key and the index with same values.
	* Do not get confused because  'pulse % 10' have the same value as the 
	* returned queue_head_key. The returned key can by any number within  
	*  (10 + (pulse % 10')) range.
	* Note that the members are ordered in ascending order, so the queue_key 
	* must always return the smallest 'key' value from a event in each bucket
	*/
	mud_event = event_create_local(event_q, enqueue_time_pulse, simple_func, NULL, 1);
	mud_event = event_create_local(event_q, enqueue_time_pulse, simple_func, NULL, 2);
	mud_event = event_create_local(event_q, enqueue_time_pulse, simple_func, NULL, 3);
	mud_event = event_create_local(event_q, enqueue_time_pulse, simple_func, NULL, 4);
	mud_event = event_create_local(event_q, enqueue_time_pulse, simple_func, NULL, 5);
	mud_event = event_create_local(event_q, enqueue_time_pulse, simple_func, NULL, 6);
	mud_event = event_create_local(event_q, enqueue_time_pulse, simple_func, NULL, 7);
	mud_event = event_create_local(event_q, enqueue_time_pulse, simple_func, NULL, 8);
	mud_event = event_create_local(event_q, enqueue_time_pulse, simple_func, NULL, 9);
	mud_event = event_create_local(event_q, enqueue_time_pulse, simple_func, NULL, 11);

	retrieve_time_pulse = 10l;
	long queue_head_key10 = queue_key_local(event_q, retrieve_time_pulse);
	retrieve_time_pulse = 21l;
	long queue_head_key1 = queue_key_local(event_q, retrieve_time_pulse);
	retrieve_time_pulse = 52l;
	long queue_head_key2 = queue_key_local(event_q, retrieve_time_pulse);
	retrieve_time_pulse = 1203l;
	long queue_head_key3 = queue_key_local(event_q, retrieve_time_pulse);
	retrieve_time_pulse = 1000004l;
	long queue_head_key4 = queue_key_local(event_q, retrieve_time_pulse);
	retrieve_time_pulse = ULONG_MAX; // (4294967295 % 10 = 5)
	long queue_head_key5 = queue_key_local(event_q, retrieve_time_pulse);


	CuAssertLongEquals(tc, 10, queue_head_key10);
	CuAssertLongEquals(tc, 1, queue_head_key1);
	CuAssertLongEquals(tc, 2, queue_head_key2);
	CuAssertLongEquals(tc, 3, queue_head_key3);
	CuAssertLongEquals(tc, 4, queue_head_key4);
	CuAssertLongEquals(tc, 5, queue_head_key5);
	/*
	* Release resources
	*/
	queue_free_2(&event_q);
}

void test_queue_head(CuTest *tc) {

	/*
	* First use case. A empty queue
	*/

	// pulse is originally unsigned.
	unsigned long enqueue_time_pulse = 0l;
	unsigned long retrieve_time_pulse = 0l;
	int event_delay = 1;

	static struct dg_queue *event_q;
	struct event * mud_event;

	event_q = queue_init();

	// Check against an empty queue
	void * event_data = queue_head_local(event_q, retrieve_time_pulse);
	CuAssertPtrEquals(tc, NULL, event_data);
	
	// Put an event on the queue
	mud_event = event_create_local(event_q, enqueue_time_pulse, simple_func, NULL, event_delay);
	
	// Create the correct pulse to check the queue
	retrieve_time_pulse = enqueue_time_pulse + event_delay;

	// If we pass the wrong pulse, it must return nothing
	void * event_data0 = queue_head_local(event_q, retrieve_time_pulse+2);
	CuAssertPtrEquals(tc, NULL, event_data0);

	// if we pass the correct pulse it must return the event
	void * event_data1 = queue_head_local(event_q, retrieve_time_pulse);
	CuAssertPtrNotNull(tc, event_data1);
	// TODO: Check if its possible to change the pointer from *void to 'event' type

	// The routine must remove the element from queue. 
	// If we call it again, nothing must be returned
	void * event_data2 = queue_head_local(event_q, retrieve_time_pulse);
	CuAssertPtrEquals(tc, NULL, event_data2);
}

void test_queue_enq(CuTest *tc) {

	struct dg_queue * event_q;
	unsigned long current_pulse = 0;
	void *event_obj = NULL;
	long bucket_number = 1;
	long when = bucket_number;
	struct event *new_event_1;

	// create the queue
	event_q = queue_init();


	// Allocate memory for the event object
	CREATE(new_event_1, struct event, 1);
	CuAssertPtrNotNullMsg(tc, "Error to allocate memory to a new_event_1 object.", new_event_1);
	new_event_1->func = simple_func;
	new_event_1->event_obj = event_obj;
	new_event_1->isMudEvent = FALSE;

	// Add an single element to bucket determied by 'when' (when % 10)
	new_event_1->q_el = queue_enq(event_q, new_event_1, when + current_pulse);
	CuAssertPtrNotNullMsg(tc, "Event (new_event_1) not pointing to a valid queue bucket", new_event_1->q_el);
	CuAssertPtrEquals_Msg(tc, "", event_q->head[when%10], new_event_1->q_el);
	CuAssertPtrEquals_Msg(tc, "", event_q->tail[when%10], new_event_1->q_el);


	/**
	 * Enqueue a seccond element, in the same bucket and check if the order is OK
	 */
	// Allocate memory for the event object
	struct event *new_event_2;
	CREATE(new_event_2, struct event, 1);
	CuAssertPtrNotNullMsg(tc, "Error to allocate memory to a new_event_2 object.", new_event_2);
	new_event_2->func = simple_func;
	new_event_2->event_obj = event_obj;
	new_event_2->isMudEvent = FALSE;

	// Change time to a multiple of 10 + event_bucket
	when = when + 50; // it must fit in the same bucket

	// Add an single element to bucket determied by 'when' (when % 10)
	new_event_2->q_el = queue_enq(event_q, new_event_2, when + current_pulse);
	CuAssertPtrNotNullMsg(tc, "Event (new_event_2) not pointing to a valid queue bucket", new_event_2->q_el);
	// Check if the first element still on head
	CuAssertPtrEquals_Msg(tc, "", event_q->head[when % 10], new_event_1->q_el);
	//and check if the new element is on the tail
	CuAssertPtrEquals_Msg(tc, "", event_q->tail[when % 10], new_event_2->q_el);
	// Check references (next/previous)
	CuAssertPtrEquals(tc, NULL, new_event_1->q_el->prev); /* Head element must have prev = NULL */
	CuAssertPtrEquals(tc, new_event_2->q_el, new_event_1->q_el->next); /* the new element must be the ->next */
	CuAssertPtrEquals(tc, new_event_1->q_el, new_event_2->q_el->prev);
	CuAssertPtrEquals(tc, NULL, new_event_2->q_el->next);

	/**
	 * Finally, add a third element between the first two ones
	 */
	// Allocate memory for the event object
	struct event *new_event_3;
	CREATE(new_event_3, struct event, 1);
	CuAssertPtrNotNullMsg(tc, "Error to allocate memory to a new_event_3 object.", new_event_3);
	new_event_3->func = simple_func;
	new_event_3->event_obj = event_obj;
	new_event_3->isMudEvent = FALSE;

	// Change time to a multiple of 10 + event_bucket
	when = when - 20; // it must fit in middle of two previous elements

	// Add an single element to bucket determied by 'when' (when % 10)
	new_event_3->q_el = queue_enq(event_q, new_event_3, when + current_pulse);
	CuAssertPtrNotNullMsg(tc, "Event not pointing to a valid queue bucket", new_event_3->q_el);
	// Check if the first element (new_event_1) still on head
	CuAssertPtrEquals_Msg(tc, "", event_q->head[when % 10], new_event_1->q_el);
	//and check if the seccond element (new_event_2) still on tail 
	CuAssertPtrEquals_Msg(tc, "", event_q->tail[when % 10], new_event_2->q_el);
	
	// Check references (next/previous) for the middle element
	CuAssertPtrEquals(tc, new_event_1->q_el, new_event_3->q_el->prev); /* must point to head element*/
	CuAssertPtrEquals(tc, new_event_2->q_el, new_event_3->q_el->next); /* must point to tail element*/
	
	/*
	* Release resources
	*/
	queue_free_2(&event_q);
}

void test_queue_deq(CuTest *tc) {
	//void queue_deq(struct dg_queue *q, struct q_element *qe)
	struct dg_queue * event_q;
	unsigned long current_pulse = 0;
	void *event_obj = NULL;
	long bucket_number = 1;
	long when = bucket_number;
	struct event *new_event_1;

	// create the queue
	event_q = queue_init();


	// Allocate memory for the event object
	CREATE(new_event_1, struct event, 1);
	CuAssertPtrNotNullMsg(tc, "Error to allocate memory to a new_event_1 object.", new_event_1);
	new_event_1->func = simple_func;
	new_event_1->event_obj = event_obj;
	new_event_1->isMudEvent = FALSE;

	// Allocate memory for the event object
	struct event *new_event_2;
	CREATE(new_event_2, struct event, 1);
	CuAssertPtrNotNullMsg(tc, "Error to allocate memory to a new_event_2 object.", new_event_2);
	new_event_2->func = simple_func;
	new_event_2->event_obj = event_obj;
	new_event_2->isMudEvent = FALSE;
		
	// Allocate memory for the event object
	struct event *new_event_3;
	CREATE(new_event_3, struct event, 1);
	CuAssertPtrNotNullMsg(tc, "Error to allocate memory to a new_event_3 object.", new_event_3);
	new_event_3->func = simple_func;
	new_event_3->event_obj = event_obj;
	new_event_3->isMudEvent = FALSE;

	
	// Enqueue
	new_event_1->q_el = queue_enq(event_q, new_event_1, when + current_pulse);
	
	when = when + 50;
	new_event_2->q_el = queue_enq(event_q, new_event_2, when + current_pulse);

	when = when - 20; // it must fit in middle of two previous elements
	new_event_3->q_el = queue_enq(event_q, new_event_3, when + current_pulse);

	// Check if it was allocated
	CuAssertPtrNotNullMsg(tc, "Event (new_event_1) not pointing to a valid queue bucket", new_event_1->q_el);
	CuAssertPtrNotNullMsg(tc, "Event (new_event_2) not pointing to a valid queue bucket", new_event_2->q_el);
	CuAssertPtrNotNullMsg(tc, "Event (new_event_3) not pointing to a valid queue bucket", new_event_3->q_el);


	/*
	* Check the order
	*/
	CuAssertPtrEquals_Msg(tc, "Object new_event_1->q_el must be the head.", event_q->head[when % 10], new_event_1->q_el);
	CuAssertPtrEquals_Msg(tc, "Object new_event_2->q_el must be the tail.", event_q->tail[when % 10], new_event_2->q_el);


	// Check references (next/previous)
	CuAssertPtrEquals(tc, NULL, new_event_1->q_el->prev); /* Head element must have prev = NULL */
	CuAssertPtrEquals(tc, new_event_3->q_el, new_event_1->q_el->next); /* must point to middle (new_event_3)*/
	CuAssertPtrEquals(tc, NULL, new_event_2->q_el->next); /* Tail element must have NEXT = NULL */
	CuAssertPtrEquals(tc, new_event_3->q_el, new_event_2->q_el->prev); /* must point to middle (new_event_3)*/
															  
	// Check references (next/previous) for the middle element
	CuAssertPtrEquals(tc, new_event_1->q_el, new_event_3->q_el->prev); /* must point to head element*/
	CuAssertPtrEquals(tc, new_event_2->q_el, new_event_3->q_el->next); /* must point to tail element*/
	
	// OK, queue sanity checked
	
	// Remove first element
	queue_deq(event_q, new_event_1->q_el);

	// Middle element must become the HEAD
	CuAssertPtrEquals_Msg(tc, "Object new_event_3->q_el must be the head.", event_q->head[when % 10], new_event_3->q_el);
	CuAssertPtrEquals_Msg(tc, "Object new_event_2->q_el must be the tail.", event_q->tail[when % 10], new_event_2->q_el);
	CuAssertPtrEquals(tc, NULL, new_event_3->q_el->prev); /* Head element must have prev = NULL */
	CuAssertPtrEquals(tc, new_event_2->q_el, new_event_3->q_el->next); /* Head element must have prev = NULL */


	queue_deq(event_q, new_event_2->q_el);
	// new_event_3 must be the only one (head and tail)
	CuAssertPtrEquals_Msg(tc, "Object new_event_3->q_el must be the head.", event_q->head[when % 10], new_event_3->q_el);
	CuAssertPtrEquals_Msg(tc, "Object new_event_3->q_el must be the tail.", event_q->tail[when % 10], new_event_3->q_el);
	CuAssertPtrEquals(tc, NULL, new_event_3->q_el->prev); /* Head element must have prev = NULL */
	CuAssertPtrEquals(tc, NULL, new_event_3->q_el->next); /* Head element must have prev = NULL */

	queue_deq(event_q, new_event_3->q_el);
	CuAssertPtrEquals_Msg(tc, "Object event_q->head must NULL.", NULL, event_q->head[when % 10]);
	CuAssertPtrEquals_Msg(tc, "Object event_q->tail must NULL.", NULL, event_q->tail[when % 10]);


	/*
	* Release resources
	*/
	queue_free_2(&event_q);
}

void test_event_cancel(CuTest *tc) {
	//void event_cancel_local(struct event *event, struct dg_queue * queue);
	//void queue_deq(struct dg_queue *q, struct q_element *qe)
	struct dg_queue * event_q;
	unsigned long current_pulse = 0;
	void *event_obj = NULL;
	long bucket_number = 1;
	long when = bucket_number;
	struct event *new_event_1;

	// create the queue
	event_q = queue_init();


	// Allocate memory for the event object
	CREATE(new_event_1, struct event, 1);
	CuAssertPtrNotNullMsg(tc, "Error to allocate memory to a new_event_1 object.", new_event_1);
	new_event_1->func = simple_func;
	new_event_1->event_obj = event_obj;
	new_event_1->isMudEvent = FALSE;

	// Allocate memory for the event object
	struct event *new_event_2;
	CREATE(new_event_2, struct event, 1);
	CuAssertPtrNotNullMsg(tc, "Error to allocate memory to a new_event_2 object.", new_event_2);
	new_event_2->func = simple_func;
	new_event_2->event_obj = event_obj;
	new_event_2->isMudEvent = FALSE;

	// Allocate memory for the event object
	struct event *new_event_3;
	CREATE(new_event_3, struct event, 1);
	CuAssertPtrNotNullMsg(tc, "Error to allocate memory to a new_event_3 object.", new_event_3);
	new_event_3->func = simple_func;
	new_event_3->event_obj = event_obj;
	new_event_3->isMudEvent = FALSE;


	// Enqueue
	new_event_1->q_el = queue_enq(event_q, new_event_1, when + current_pulse);

	when = when + 50;
	new_event_2->q_el = queue_enq(event_q, new_event_2, when + current_pulse);

	when = when - 20; // it must fit in middle of two previous elements
	new_event_3->q_el = queue_enq(event_q, new_event_3, when + current_pulse);

	// Check if it was allocated
	CuAssertPtrNotNullMsg(tc, "Event (new_event_1) not pointing to a valid queue bucket", new_event_1->q_el);
	CuAssertPtrNotNullMsg(tc, "Event (new_event_2) not pointing to a valid queue bucket", new_event_2->q_el);
	CuAssertPtrNotNullMsg(tc, "Event (new_event_3) not pointing to a valid queue bucket", new_event_3->q_el);


	/*
	* Check the order
	*/
	CuAssertPtrEquals_Msg(tc, "Object new_event_1->q_el must be the head.", event_q->head[when % 10], new_event_1->q_el);
	CuAssertPtrEquals_Msg(tc, "Object new_event_2->q_el must be the tail.", event_q->tail[when % 10], new_event_2->q_el);


	// Check references (next/previous)
	CuAssertPtrEquals(tc, NULL, new_event_1->q_el->prev); /* Head element must have prev = NULL */
	CuAssertPtrEquals(tc, new_event_3->q_el, new_event_1->q_el->next); /* must point to middle (new_event_3)*/
	CuAssertPtrEquals(tc, NULL, new_event_2->q_el->next); /* Tail element must have NEXT = NULL */
	CuAssertPtrEquals(tc, new_event_3->q_el, new_event_2->q_el->prev); /* must point to middle (new_event_3)*/

	// Check references (next/previous) for the middle element
	CuAssertPtrEquals(tc, new_event_1->q_el, new_event_3->q_el->prev); /* must point to head element*/
	CuAssertPtrEquals(tc, new_event_2->q_el, new_event_3->q_el->next); /* must point to tail element*/

	// OK, queue sanity checked

	// Remove first element
	event_cancel_local(event_q, new_event_1);

	// Middle element must become the HEAD
	CuAssertPtrEquals_Msg(tc, "Object new_event_3->q_el must be the head.", event_q->head[when % 10], new_event_3->q_el);
	CuAssertPtrEquals_Msg(tc, "Object new_event_2->q_el must be the tail.", event_q->tail[when % 10], new_event_2->q_el);
	CuAssertPtrEquals(tc, NULL, new_event_3->q_el->prev); /* Head element must have prev = NULL */
	CuAssertPtrEquals(tc, new_event_2->q_el, new_event_3->q_el->next); /* Head element must have prev = NULL */


	event_cancel_local(event_q, new_event_2);
	// new_event_3 must be the only one (head and tail)
	CuAssertPtrEquals_Msg(tc, "Object new_event_3->q_el must be the head.", event_q->head[when % 10], new_event_3->q_el);
	CuAssertPtrEquals_Msg(tc, "Object new_event_3->q_el must be the tail.", event_q->tail[when % 10], new_event_3->q_el);
	CuAssertPtrEquals(tc, NULL, new_event_3->q_el->prev); /* Head element must have prev = NULL */
	CuAssertPtrEquals(tc, NULL, new_event_3->q_el->next); /* Head element must have prev = NULL */

	event_cancel_local(event_q, new_event_3);
	CuAssertPtrEquals_Msg(tc, "Object event_q->head must NULL.", NULL, event_q->head[when % 10]);
	CuAssertPtrEquals_Msg(tc, "Object event_q->tail must NULL.", NULL, event_q->tail[when % 10]);


	/*
	* Release resources
	*/
	queue_free_2(&event_q);
}

/**
* I dont know a effectivfe way to thest memory freeing in C
*/
void test_cleanup_event_obj(CuTest *tc)
{
	//void cleanup_event_obj(struct event *event)
}

void test_event_proces(CuTest *tc)
{
	//void event_process_local(const unsigned long current_pulse, struct dg_queue * queue)
}

void test_event_time(CuTest *tc)
{
	struct dg_queue * event_q;
	unsigned long current_pulse = 0;
	long key = 99;
	struct event *new_event;

	// create the queue
	event_q = queue_init();

	// Allocate memory for the event object
	CREATE(new_event, struct event, 1);
	CuAssertPtrNotNullMsg(tc, "Error to allocate memory to a new_event_1 object.", new_event);
	new_event->func = simple_func;
	new_event->event_obj = NULL;
	new_event->isMudEvent = FALSE;

	// Enqueue a event with 99 pulses delay
	new_event->q_el = queue_enq(event_q, new_event, key + current_pulse);
	CuAssertPtrNotNullMsg(tc, "Event (new_event_1) not pointing to a valid queue bucket", new_event->q_el);


	// Check remaining pulses (must be 99(
	long time_remaining = event_time_local(new_event, current_pulse);
	CuAssertLongEquals(tc, 99, time_remaining);

	current_pulse = 50;
	time_remaining = event_time_local(new_event, current_pulse);
	CuAssertLongEquals(tc, 49, time_remaining);

	current_pulse = 99;
	time_remaining = event_time_local(new_event, current_pulse);
	CuAssertLongEquals(tc, 0, time_remaining);

	current_pulse = 100;
	time_remaining = event_time_local(new_event, current_pulse);
	CuAssertLongEquals(tc, -1, time_remaining);
	

	/*
	* I think this case will never happen in the MUD... so we can check
	* if its worth to refactor the method to return 0 when the time has passed.
	*/
	current_pulse = -1;
	time_remaining = event_time_local(new_event, current_pulse);
	CuAssertLongEquals(tc, 100, time_remaining);


	/**
	* Testing limits... I really dont know exactly the math here
	* but we must assure that the behavior will not change untill 
	* we find out.
	*
	* GUESS: I think the problem is that pulse is always a Unsigned Long, but the
	* element KEY is a simple 'long'. Maybe we should refactor it to become an 
	* unsigned long, also we should change the method return type.
	*/

	current_pulse = LONG_MAX; /* 2147483647L */
	long time_remaining_max = event_time_local(new_event, current_pulse);
	CuAssertLongEquals(tc, LONG_MIN+100, time_remaining_max);

	current_pulse = LONG_MIN; /* -2147483646L */
	long time_remaining_min = event_time_local(new_event, current_pulse);
	CuAssertLongEquals(tc, LONG_MAX + 100, time_remaining_min);

	current_pulse = ULONG_MAX;
	long time_remaining_umax = event_time_local(new_event, current_pulse);
	CuAssertLongEquals(tc, 100, time_remaining_umax);	

	/*
	* Release resources
	*/
	queue_free_2(&event_q);
}

/**
* I dont know a effectivfe way to thest memory freeing in C
*/
void test_queue_free(CuTest *tc) {

	struct dg_queue * event_q;
	unsigned long current_pulse = 0;
	long key = 99;
	struct event *new_event;

	// create the queue
	event_q = queue_init();
	CuAssertPtrNotNullMsg(tc, "Error to allocate memory to a event_q object.", event_q);
	
	// Allocate memory for the event object
	CREATE(new_event, struct event, 1);
	CuAssertPtrNotNullMsg(tc, "Error to allocate memory to a new_event_1 object.", new_event);
	new_event->func = simple_func;
	new_event->event_obj = NULL;
	new_event->isMudEvent = FALSE;

	// Enqueue a event with 99 pulses delay
	new_event->q_el = queue_enq(event_q, new_event, key + current_pulse);
	CuAssertPtrNotNullMsg(tc, "Event (new_event_1) not pointing to a valid queue bucket", new_event->q_el);

	queue_free_2(&event_q);
}

/**
 * Export function to aggregate all tests. This function must be
 * included in testrunner.c
*/
CuSuite* dg_eventGetSuite(void)
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_queue_init);
	SUITE_ADD_TEST(suite, test_event_create);
	SUITE_ADD_TEST(suite, test_event_cancel);
	SUITE_ADD_TEST(suite, test_queue_key);
	SUITE_ADD_TEST(suite, test_queue_head);
	SUITE_ADD_TEST(suite, test_queue_enq);
	SUITE_ADD_TEST(suite, test_queue_deq);
	SUITE_ADD_TEST(suite, test_cleanup_event_obj);
	SUITE_ADD_TEST(suite, test_event_proces);
	SUITE_ADD_TEST(suite, test_event_time);
	SUITE_ADD_TEST(suite, test_queue_free);

	return suite;
}