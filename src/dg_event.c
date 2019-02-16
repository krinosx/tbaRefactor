/**
* @file dg_event.c
* This file contains a simplified event system to allow trigedit 
* to use the "wait" command, causing a delay in the middle of a script.
* This system could easily be expanded by coders who wish to implement
* an event driven mud.
* 
* Part of the core tbaMUD source code distribution, which is a derivative
* of, and continuation of, CircleMUD.
* 
* This source code, which was not part of the CircleMUD legacy code,
* was created by the following people:                                      
* $Author: Mark A. Heilpern/egreen/Welcor $                              
* $Date: 2004/10/11 12:07:00$                                            
* $Revision: 1.0.14 $                                                    
*/


#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "db.h"
#include "dg_event.h"
#include "constants.h"
#include "comm.h"  /* For access to the game pulse */
#include "mud_event.h"

/***************************************************************************
 * Begin mud specific event queue functions
 **************************************************************************/
/* file scope variables */
/** The mud specific queue of events. */
static struct dg_queue *event_q;


// Prototype
void *queue_head_local(struct dg_queue *q, unsigned long current_pulse);

/** Initializes the main event queue event_q.
 * @post The main event queue, event_q, has been created and initialized.
 */
void event_init(void)
{
  event_q = queue_init();
}


/**
* Created in order to be able to write unity tests.
* Dont use this, you can use the the default 'event_create' function
* @param current_pulse the actual pulse of the game world.
*/
struct event *event_create_local(struct dg_queue * q, unsigned long current_pulse, EVENTFUNC(*func), void *event_obj, long when)
{
  struct event *new_event;

  if (when < 1) /* make sure its in the future */
    when = 1;

  CREATE(new_event, struct event, 1);
  new_event->func = func;
  new_event->event_obj = event_obj;
  new_event->q_el = queue_enq(q, new_event, when + current_pulse);
  new_event->isMudEvent = FALSE;

  return new_event;
}

/** Creates a new event 'object' that is then enqueued to the global event_q.
 * @pre pulse must be defined. This is a multi-headed queue, the current
 * head is determined by the current pulse.
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
struct event *event_create(EVENTFUNC(*func), void *event_obj, long when) {
	return event_create_local(event_q, pulse, func, event_obj, when);
}

/** Refactored to remove global variables reference and allow to
 * unit testing. Keep calling the 
 * default 'void event_cancel(strict event * event)' version.
 */
void event_cancel_local(struct dg_queue * queue, struct event *event)
{
	if (!event) {
		log("SYSERR:  Attempted to cancel a NULL event");
		return;
	}

	if (!event->q_el) {
		log("SYSERR:  Attempted to cancel a non-NULL unqueued event, freeing anyway");
	}
	else {
		queue_deq(queue, event->q_el);
		event->q_el = NULL;
	}


	if (event->event_obj) {
		cleanup_event_obj(event);
		event->event_obj = NULL;
	}

	DISPOSE(event);
	event = NULL;
}

/** Removes an event from event_q and frees the event.
 * @param event Pointer to the event to be dequeued and removed.
 */
void event_cancel(struct event *event)
{
	event_cancel_local(event_q, event);
}

/* The memory freeing routine tied into the mud event system */
void cleanup_event_obj(struct event *event)
{
  struct mud_event_data * mud_event;

  if (event->isMudEvent) {
	  mud_event = (struct mud_event_data *) event->event_obj;
	  free_mud_event(mud_event);
	  mud_event = NULL;
  }
  else {
	  DISPOSE(event->event_obj);
	  event->event_obj = NULL;
  }
}

/** 
* Refactores to allow unit testing. Removing use of global parameters
 */
void event_process_local(unsigned long current_pulse, struct dg_queue * queue)
{
  struct event *the_event;
  long new_time;

  /**
  * If the head element of current bucket should have been executed 
  * in the past or now...
  */
  while (current_pulse >= queue_key_local(queue, current_pulse)) {
	// Check if its an valid event and retrieve it from 
	// the queue (remove it from queue also)
    if (!(the_event = (struct event *) queue_head_local(queue, current_pulse))) {
      log("SYSERR: Attempt to get a NULL event");
      return;
    }

    /* Set the_event->q_el to NULL so that any functions called beneath 
     * event_process can tell if they're being called beneath the actual
     * event function. ?? WHAT??? we MUST refactor it to remove void pointers. */
    the_event->q_el = NULL;

    /* call event func, reenqueue event if retval > 0 */
	if ((new_time = (the_event->func)(the_event->event_obj)) > 0) {
		the_event->q_el = queue_enq(queue, the_event, new_time + current_pulse);
	}
	else
    {
      if (the_event->isMudEvent && the_event->event_obj != NULL)
        free_mud_event((struct mud_event_data *) the_event->event_obj);
      /* It is assumed that the_event will already have freed ->event_obj. */
	  DISPOSE(the_event);
    }
      
  }
}

/** Process any events whose time has come. Should be called from, and at, every
 * pulse of heartbeat. Re-enqueues multi-use events.
 *-- GLOBALS
 * @pre pulse must be defined
 * @pre event_q must be defined
 */
void event_process(void) {
	event_process_local(pulse, event_q);
}

/** Refactored in order to remove global variables reference
* and enable unit testing.
*/
unsigned long event_time_local(struct event *event, unsigned long pulse)
{
	long when;

	when = queue_elmt_key(event->q_el);

	return (when - pulse);
}

/** Returns the time remaining before the event as how many pulses from now. 
 * @param event Check this event for its scheduled activation time.
 * @retval long Number of pulses before this event will fire. */
unsigned long event_time(struct event *event)
{
	return event_time_local(event, pulse);
}

/** Frees all events from event_q. */
void event_free_all(void)
{
  queue_free(event_q);
}

/** Boolean function to tell whether an event is queued or not. Does this by
 * checking if event->q_el points to anything but null.
 * @retval int 1 if the event has been queued, 0 if the event has not.
 **/
int event_is_queued(struct event *event)
{
	if (event->q_el) {
		return 1;
	}
	else {
		return 0;
	}
}
/***************************************************************************
 * End mud specific event queue functions
 **************************************************************************/

/***************************************************************************
 * Begin generic (abstract) priority queue functions
 **************************************************************************/
/** Create a new, empty, priority queue and return it.
 * @retval dg_queue * Pointer to the newly created queue structure. */
struct dg_queue *queue_init(void)
{
  struct dg_queue *q;

  CREATE(q, struct dg_queue, 1);

  return q;
}

/** Add some 'data' to a priority queue. 
 * @pre The paremeter q must have been previously created by queue_init.
 * @post A new q_element is created to hold the data parameter.
 * @param q The existing dg_queue to add an element to. 
 * @param data The data to be associated with, and theoretically used, when
 * the element comes up in q. data is wrapped in a new q_element.
 * @param key Indicates where this event should be located in the queue, and
 * when the element should be activated.
 * @retval q_element Pointer to the created q_element that contains
 * the data. */
struct q_element *queue_enq(struct dg_queue *q, void *data, unsigned long key)
{
  struct q_element *qe, *i;
  int bucket;

  CREATE(qe, struct q_element, 1);
  qe->data = data;
  qe->key = key;

  bucket = key % NUM_EVENT_QUEUES;   /* which queue does this go in */

  if (!q->head[bucket]) 
  { /* queue is empty */
    q->head[bucket] = qe;
    q->tail[bucket] = qe;
  }
  else 
  {
    for (i = q->tail[bucket]; i; i = i->prev) {

		if (i->key < key) { /* found insertion point */
			if (i == q->tail[bucket]) {
				q->tail[bucket] = qe;
			}
			else {
				qe->next = i->next;
				i->next->prev = qe;
			}

			qe->prev = i;
			i->next = qe;
			break;
		}
    }

    if (i == NULL) { /* insertion point is front of list */
      qe->next = q->head[bucket];
      q->head[bucket] = qe;
      qe->next->prev = qe;
    }
  }

  return qe;
}

/** Remove queue element qe from the priority queue q.
 * @pre qe->data has been dealt with in some way.
 * @post qe has been freed. 
 * @param q Pointer to the queue containing qe.
 * @param qe Pointer to the q_element to remove from q.
 */
void queue_deq(struct dg_queue *q, struct q_element *qe)
{
  int i;

  if (!qe) {
	  log("SYSERR:  Attempted to queue_deq a NULL queue element");
	  return;
  }

  i = qe->key % NUM_EVENT_QUEUES;

  if (qe->prev == NULL) {
	  q->head[i] = qe->next;
  }
  else {
	  qe->prev->next = qe->next;
  }

  if (qe->next == NULL) {
	  q->tail[i] = qe->prev;
  }
  else {
	  qe->next->prev = qe->prev;
  }

  DISPOSE(qe);
}

/**
* Refactored version of the method to enable unit testing.
* This method does not use global variables.
* keep using the original version 'queue_head(struct dg_queue *q)'
* - Scenarios to be tested based on method execution graph
*	 1 - When queue has no element at head[i]
*	 2 - when queue has element at head [i]
* - Exception scenarios
*	1 - when head[i]->data is NULL: We dont know if there is no 
*    event or if the event_data is null, but in this case, does it matter?
*
*/
void *queue_head_local(struct dg_queue *q, unsigned long current_pulse)
{
  void *dg_data;
  int i;

  i = current_pulse % NUM_EVENT_QUEUES;

  if (!q->head[i]) {
	  return NULL;
  }
  else {
	  dg_data = q->head[i]->data;
	  queue_deq(q, q->head[i]);
	  return dg_data;
  }
}

/** 
*  Removes and returns the data of the first element of the priority queue q.
 * @pre pulse must be defined. This is a multi-headed queue, the current
 * head is determined by the current pulse.
 * @post the q->head is dequeued.
 * @param q The queue to return the head of.
 * @retval void * NULL if there is not a currently available head, pointer
 * to any data object associated with the queue element. */
void * queue_head(struct dg_queue *q ) {
	return queue_head_local(q, pulse);
}


/**
* Refactored version. Used to unity testing. Use the default version 'queue_key'
*/
unsigned long queue_key_local(struct dg_queue *q, unsigned long p)
{
	int i;

	i = p % NUM_EVENT_QUEUES;

	if (q->head[i]) 
	{
		return q->head[i]->key;
	}
	else 
	{
		return LONG_MAX;
	}
}

/** 
 * Returns the key (pulse) of the head element of the priority queue.
 * @pre pulse must be defined. This is a multi-headed queue, the current
 * head is determined by the current pulse.
 * @param q Queue to check for.
 * @retval long Return the key element of the head q_element. If no head
 * q_element is available, return LONG_MAX. */
unsigned long queue_key(struct dg_queue *q)
{
	return queue_key_local(q, pulse);
}

/** Returns the key of queue element qe.
 * @param qe Pointer to the keyed q_element.
 * @retval long Key of qe. */
unsigned long queue_elmt_key(struct q_element *qe)
{
  return qe->key;
}

/** Free q and all contents.
 * @pre Function requires definition of struct event.
 * @post All items associeated qith q, including non-abstract data, are freed.
 * @param q The priority queue to free.
 */
void queue_free(struct dg_queue *q)
{
  int i;
  struct q_element *qe, *next_qe;
  struct event *event;

  for (i = 0; i < NUM_EVENT_QUEUES; i++)
  {
    for (qe = q->head[i]; qe; qe = next_qe) 
    {
      next_qe = qe->next;
      if ((event = (struct event *) qe->data) != NULL) 
      {
        if (event->event_obj)
          cleanup_event_obj(event);

        DISPOSE(event);
      }
	  DISPOSE(qe);
    }
  }

  DISPOSE(q);
}


/**
* Refactoring to avoid 'Dangling pointers' 
*/
void queue_free_2(struct dg_queue ** q2)
{
	int i;
	struct q_element *qe, *next_qe;
	struct event *event;
	for (i = 0; i < NUM_EVENT_QUEUES; i++)
	{
		for (qe = (*q2)->head[i]; qe; qe = next_qe)
		{
			next_qe = qe->next;
			if ((event = (struct event *) qe->data) != NULL)
			{
				if (event->event_obj) {
					cleanup_event_obj(event);
				}
				DISPOSE(event);
				event = NULL;
			}
			DISPOSE(qe);
			qe = NULL;
		}
	}
	DISPOSE(*q2);

	*q2 = NULL;
}

