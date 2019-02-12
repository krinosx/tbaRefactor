#include "../src/conf.h"
#include "../src/sysdep.h"
#include "../src/structs.h"
#include "../src/utils.h"
#include "../src/db.h"
#include "../src/dg_event.h"
#include "../src/constants.h"
#include "../src/comm.h" /* For access to the game pulse */
#include "../src/mud_event.h"


#include "CuTest.h"



/*-------------------------------------------------------------------------*
 * CuString Test
 *-------------------------------------------------------------------------*/
 /** Function prototypes for code being tested */
struct dg_queue *queue_init(void);

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



/** Creates a new event 'object' that is then enqueued to the global event_q.
 * @post If the newly created event is valid, it is always added to event_q.
 * @param func The function to be called when this event fires. This function
 * will be passed event_obj when it fires. The function must match the form
 * described by EVENTFUNC.
 * @param event_obj An optional 'something' to be passed to func when this
 * event fires. It is func's job to cast event_obj. If event_obj is not needed,
 * pass in NULL.
 * @param when Number of pulses between firing(s) of this event.
 * @retval event * Returns a pointer to the newly created event.
 **/
struct event *event_create(EVENTFUNC(*func), void *event_obj, long when);
struct event *event_create_onqueue(struct dg_queue * q, EVENTFUNC(*func), void *event_obj, long when);
long queue_key_pulse(struct dg_queue *q, unsigned long p);


static EVENTFUNC(simple_func) {
	
	int i = 1 + 1;
	return 0l;
}


void test_event_create(CuTest* tc) {
	static struct dg_queue *event_q;
	event_q = queue_init();
	struct event * mud_event;
	mud_event = event_create_onqueue(event_q, simple_func, NULL, 10);

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
	unsigned long pulse = 0l;
	static struct dg_queue *event_q;
	event_q = queue_init();
	
	// MIN para unsigned long
	pulse = 0l;
	long queue_key = queue_key_pulse(event_q, pulse);

	pulse = ULONG_MAX;
	long queue_key1 = queue_key_pulse(event_q, pulse);

	CuAssertLongEquals_Msg(tc, "Wrong value returned with pulse = 0", LONG_MAX, queue_key);
	CuAssertLongEquals_Msg(tc, "Wrong value returned with pulse = ULONG_MAX", LONG_MAX, queue_key1);
	

	/**
	* Seccond use case, testing with a queue with only 1 event with key = 10
	*/
	struct event * mud_event;
	mud_event = event_create_onqueue(event_q, simple_func, NULL, 10);

	pulse = 0l;
	long queue_key2 = queue_key_pulse(event_q, pulse);

	pulse = 1l;
	long queue_key3 = queue_key_pulse(event_q, pulse);

	pulse = 2l;
	long queue_key4 = queue_key_pulse(event_q, pulse);

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
	mud_event = event_create_onqueue(event_q, simple_func, NULL, 1);
	mud_event = event_create_onqueue(event_q, simple_func, NULL, 2);
	mud_event = event_create_onqueue(event_q, simple_func, NULL, 3);
	mud_event = event_create_onqueue(event_q, simple_func, NULL, 4);
	mud_event = event_create_onqueue(event_q, simple_func, NULL, 5);
	mud_event = event_create_onqueue(event_q, simple_func, NULL, 6);
	mud_event = event_create_onqueue(event_q, simple_func, NULL, 7);
	mud_event = event_create_onqueue(event_q, simple_func, NULL, 8);
	mud_event = event_create_onqueue(event_q, simple_func, NULL, 9);
	mud_event = event_create_onqueue(event_q, simple_func, NULL, 11);
	

	pulse = 10l;
	long queue_head_key10 = queue_key_pulse(event_q, pulse);
	pulse = 21l;
	long queue_head_key1 = queue_key_pulse(event_q, pulse);
	pulse = 52l;
	long queue_head_key2 = queue_key_pulse(event_q, pulse);
	pulse = 1203l;
	long queue_head_key3 = queue_key_pulse(event_q, pulse);
	pulse = 1000004l;
	long queue_head_key4 = queue_key_pulse(event_q, pulse);
	pulse = ULONG_MAX; // (4294967295 % 10 = 5)
	long queue_head_key5 = queue_key_pulse(event_q, pulse);


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
	
	
	
	return suite;
}