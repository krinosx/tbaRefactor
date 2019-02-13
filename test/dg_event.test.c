/*
Removed headers... not used...
Keep it here until the testing script is finished... 
if everything keep working, remove it.

#include "../src/conf.h"
#include "../src/sysdep.h"
#include "../src/structs.h"
#include "../src/utils.h"
#include "../src/db.h"
#include "../src/dg_event.h"
#include "../src/constants.h"
#include "../src/comm.h" /* For access to the game pulse 
#include "../src/mud_event.h"
*/


#include "../src/structs.h"
#include "../src/dg_event.h"
#include "../src/constants.h"
#include "../src/mud_event.h"

#include "CuTest.h"

/*-------------------------------------------------------------------------*
 * CuString Test
 *-------------------------------------------------------------------------*/
 /** Function prototypes for code being tested */
struct dg_queue *queue_init(void);
struct event *event_create_onqueue(struct dg_queue * q, unsigned long current_pulse, EVENTFUNC(*func), void *event_obj, long when);
long queue_key_pulse(struct dg_queue *q, unsigned long p);
void *queue_head_pulse(struct dg_queue *q, unsigned long current_pulse);


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
	queue_free(event_q);

}


void test_event_create(CuTest* tc) {
	static struct dg_queue *event_q;
	event_q = queue_init();
	struct event * mud_event;

	unsigned long currentPulse = 0;

	mud_event = event_create_onqueue(event_q, currentPulse,  simple_func, NULL, 10);

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
	queue_free(event_q);
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
	long queue_key = queue_key_pulse(event_q, retrieve_time_pulse);

	retrieve_time_pulse = ULONG_MAX;
	long queue_key1 = queue_key_pulse(event_q, retrieve_time_pulse);

	CuAssertLongEquals_Msg(tc, "Wrong value returned with pulse = 0", LONG_MAX, queue_key);
	CuAssertLongEquals_Msg(tc, "Wrong value returned with pulse = ULONG_MAX", LONG_MAX, queue_key1);
	

	/**
	* Seccond use case, testing with a queue with only 1 event with key = 10
	*/

	mud_event = event_create_onqueue(event_q, enqueue_time_pulse, simple_func, NULL, 10);

	retrieve_time_pulse = 0l;
	long queue_key2 = queue_key_pulse(event_q, retrieve_time_pulse);

	retrieve_time_pulse = 1l;
	long queue_key3 = queue_key_pulse(event_q, retrieve_time_pulse);

	retrieve_time_pulse = 2l;
	long queue_key4 = queue_key_pulse(event_q, retrieve_time_pulse);

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
	mud_event = event_create_onqueue(event_q, enqueue_time_pulse, simple_func, NULL, 1);
	mud_event = event_create_onqueue(event_q, enqueue_time_pulse, simple_func, NULL, 2);
	mud_event = event_create_onqueue(event_q, enqueue_time_pulse, simple_func, NULL, 3);
	mud_event = event_create_onqueue(event_q, enqueue_time_pulse, simple_func, NULL, 4);
	mud_event = event_create_onqueue(event_q, enqueue_time_pulse, simple_func, NULL, 5);
	mud_event = event_create_onqueue(event_q, enqueue_time_pulse, simple_func, NULL, 6);
	mud_event = event_create_onqueue(event_q, enqueue_time_pulse, simple_func, NULL, 7);
	mud_event = event_create_onqueue(event_q, enqueue_time_pulse, simple_func, NULL, 8);
	mud_event = event_create_onqueue(event_q, enqueue_time_pulse, simple_func, NULL, 9);
	mud_event = event_create_onqueue(event_q, enqueue_time_pulse, simple_func, NULL, 11);

	retrieve_time_pulse = 10l;
	long queue_head_key10 = queue_key_pulse(event_q, retrieve_time_pulse);
	retrieve_time_pulse = 21l;
	long queue_head_key1 = queue_key_pulse(event_q, retrieve_time_pulse);
	retrieve_time_pulse = 52l;
	long queue_head_key2 = queue_key_pulse(event_q, retrieve_time_pulse);
	retrieve_time_pulse = 1203l;
	long queue_head_key3 = queue_key_pulse(event_q, retrieve_time_pulse);
	retrieve_time_pulse = 1000004l;
	long queue_head_key4 = queue_key_pulse(event_q, retrieve_time_pulse);
	retrieve_time_pulse = ULONG_MAX; // (4294967295 % 10 = 5)
	long queue_head_key5 = queue_key_pulse(event_q, retrieve_time_pulse);


	CuAssertLongEquals(tc, 10, queue_head_key10);
	CuAssertLongEquals(tc, 1, queue_head_key1);
	CuAssertLongEquals(tc, 2, queue_head_key2);
	CuAssertLongEquals(tc, 3, queue_head_key3);
	CuAssertLongEquals(tc, 4, queue_head_key4);
	CuAssertLongEquals(tc, 5, queue_head_key5);
	/*
	* Release resources
	*/
	queue_free(event_q);
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
	void * event_data = queue_head_pulse(event_q, retrieve_time_pulse);
	CuAssertPtrEquals(tc, NULL, event_data);


	// Put an event on the queue
	mud_event = event_create_onqueue(event_q, enqueue_time_pulse, simple_func, NULL, event_delay);
	
	// Create the correct pulse to check the queue
	retrieve_time_pulse = enqueue_time_pulse + event_delay;

	// If we pass the wrong pulse, it must return nothing
	void * event_data0 = queue_head_pulse(event_q, retrieve_time_pulse+2);
	CuAssertPtrEquals(tc, NULL, event_data0);

	// if we pass the correct pulse it must return the event
	void * event_data1 = queue_head_pulse(event_q, retrieve_time_pulse);
	CuAssertPtrNotNull(tc, event_data1);
	// TODO: Check if its possible to change the pointer from *void to 'event' type

	// The routine must remove the element from queue. 
	// If we call it again, nothing must be returned
	void * event_data2 = queue_head_pulse(event_q, retrieve_time_pulse);
	CuAssertPtrEquals(tc, NULL, event_data2);
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
	SUITE_ADD_TEST(suite, test_queue_key);
	SUITE_ADD_TEST(suite, test_queue_head);

	return suite;
}