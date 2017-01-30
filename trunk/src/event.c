/* Module EVENTS
   Contains the event processor, the message sending functions and the event
   initialization function.
*/
#include <allegro.h>

#include "cgui.h"
#include "cgui/mem.h"
#include "event.h"
#include "cguiinit.h"
#include "object.h"

/* The same message structure is used for the both queues (delayed and
   current) to make the usage simpler. Not all of the components are used in
   both q:s */
typedef struct t_message {
   void (*Action) (void *msg); /* The call-back function */
   void *msg;                  /* Data pointer to the call-back */
   long delay;                 /* Only used for messages in the delayed q */
   unsigned id;                /* Id-number that can be used to remove a queued
                                  message */
   int objid;                  /* Id-number that may be optionally used to create
                                  a link between the event message and an object
                                  to make the message automatically removed when
                                  the object is destroyed */
   unsigned mask;              /* Mask for the object (for allocation) */
   int pending;                /* Used if the event occured in interrupt mode,
                                  the interrupt handler can not safely insert
                                  the message in a q itself so it has to leave
                                  it to the q processor to be done when there is
                                  time */
   struct t_block *bl;         /* Pointer to the block where the message is
                                  located (just for conveniance) */
   struct t_message *next;     /* Next message in the q */
} t_message;

/* For allocation of message-objects. There must always be a buffer of free
   messages to be used in interrupt mode (where allocation is not possible) */
typedef struct t_block {
   unsigned inuse; /* bit-array telling which messages that are in use */
   t_message m[32];
   struct t_block *next;
} t_block;

/* A meeting structure. For optional event processing - the gui will only use
   one, which means that it is unnecessary */
typedef struct t_meeting {
   int key;
   void *data;
   char *text;
   int ack;
   struct t_meeting *next;
} t_meeting;

typedef struct t_pair {
   int (*Function) (void *data);
   void *data;
} t_pair;

#define APPLICATION_FLAG 0x80000000
#define ReturnElement(m) (m)->bl->inuse &= (m)->mask

static t_meeting *meeting;

/* memory buffer for events. `event_message_buffer_size' controls the amount
   of events that can be simultanousely queued. This memory buffer is
   allocated from the system by the queue processor, to avoid memory
   allocations in the interrupt-routine (which drops an event if no memory
   available). The queue-processor continously (as soon as there is time for
   it) checks if the memory buffer large enough. It will try to keep at
   least `event_message_buffer_size' empty objects buffered (so the memory)
   will be extended as soon as the queue grows. The check algorithm is simple
   and fast which leads to a memory allocation that is rather
   `event_message_buffer_size' objects after the last allocated. */
static t_block *firstblock = NULL, *lastblock = NULL;
int event_message_buffer_size = 1024, event_inserting_stopped = 0;
static long event_id = 0;
static int give_time_to_system = 1;

/* "current queue", the actual queue from which the processor pops elements
   and executes the action speciifed. */
static t_message *cqstart = NULL, *cqend = NULL;

/* queue for delayed events. New events are inserted in the order they are
   requested. */
static t_message *dqstart = NULL, *dqend = NULL;

/* The interface between the interrupt routine and the event processor. */
static volatile int hold = 0, decrements = 0, pending_events = 0,
                    accessing_pending_events = 0;

/* Allocates a block of 32 event-message data objects */
static void AllocateBlock(void)
{
   int i;

   if (firstblock == NULL) {
      firstblock = lastblock = GetMem0(t_block, 1);
   } else
      lastblock = lastblock->next = GetMem0(t_block, 1);
   LOCK_DATA(lastblock, sizeof(t_block));
   for (i = 0; i < 32; i++) {
      lastblock->m[i].bl = lastblock;
      lastblock->m[i].mask = ~(1 << i);
      lastblock->m[i].id = ++event_id;
   }
}

static void AllocateBlocks(void)
{
   int i, n, nr_free;
   t_block *bl;

   for (bl = firstblock, nr_free = 0; bl; bl = bl->next)
      nr_free += bl->inuse == 0;
   n = event_message_buffer_size / 32 + ((event_message_buffer_size % 32) > 0);
   for (i = nr_free; i < n; i++)
      AllocateBlock();
}

/* Retrievs a message structure to be used as elements in the messages q:s
   from the pool. Allocates a new block if necessary. */
static t_message *GetQueElement(void (*Action) (void *), void *msg, int delay, int objid, int appflag)
{
   t_block *bl;
   t_message *m;
   int i;
   static volatile int critical;

   for (bl = firstblock; bl && bl->inuse == 0xffffffff; bl = bl->next)
      ;
   if (bl) {
      for (i = 0; i < 32; i++) {
         critical = 1;
         if ((bl->inuse & (1 << i)) == 0) {
            /* it is always at this point the interrupts comes :-) */
            bl->inuse |= 1 << i;
            if (!critical)
               continue; /* an interrupt was here in between, find a new one*/
            m = bl->m + i;
            m->msg = msg;
            m->objid = objid;
            m->delay = delay;
            m->Action = Action;
            m->id &= ~APPLICATION_FLAG;
            m->id |= appflag;
            m->pending = 0;
            if (objid)
               Event2ObjectLink(objid); /* Does not need to be locked, in
                    interrupt mode there will be no reference to an object */
            critical = 0;
            /* if we are in interrupt, we will always reach this point */
            return m;
         }
      }
   }
   AllocateBlocks();
   return GetQueElement(Action, msg, delay, objid, appflag);
}
END_OF_STATIC_FUNCTION(GetQueElement);

static t_message *DeQ(t_message **qstart, t_message **qend)
{
   t_message *m;

   m = *qstart;
   *qstart = m->next;
   if (*qstart == NULL)
      *qend = NULL;
   return m;
}

static void InsertInQ(t_message **qstart, t_message **qend, t_message *m)
{
   m->next = NULL;
   if (*qend == NULL)
      *qstart = *qend = m;
   else
      *qend = (*qend)->next = m;
}

static void CheckDelayedMessages(void)
{
   t_message *m, *p;
   volatile int ticks;
   t_block *bl;
   int i, n;

   hold = 1;
   ticks = decrements;
   decrements = 0;
   hold = 0;
   for (m = dqstart; m; m = dqstart) {
      m->delay -= ticks;
      if (m->delay <= 0) {
         m->delay = 0;
         InsertInQ(&cqstart, &cqend, DeQ(&dqstart, &dqend));
      } else
         break;
   }
   if (dqstart) {
      for (p = dqstart, m = dqstart->next; m;) {
         m->delay -= ticks;
         if (m->delay <= 0) {
            p->next = m->next;
            InsertInQ(&cqstart, &cqend, m);
            m = p->next;
            if (m == NULL)
               dqend = p;
         } else {
            p = m;
            m = m->next;
         }
      }
   }
   n = pending_events;
   for (bl = firstblock; bl && n > 0; bl = bl->next) {
      for (i = 0; i < 32; i++) {
         m = bl->m + i;
         if (m->pending) {
            n--;
            if (m->delay >= ticks)
               m->delay -= ticks;
            else
               m->delay = 0;
         }
      }
   }
}

static void CheckPendingMessages(void)
{
   int i, n, org;
   t_message *m;
   t_block *bl;

   if (pending_events) {
      n = pending_events;
      org = n;
      for (bl = firstblock; bl && n > 0; bl = bl->next) {
         for (i = 0; i < 32; i++) {
            m = bl->m + i;
            if (m->pending) {
               m->pending = 0;
               n--;
               if (m->delay > 0)
                  InsertInQ(&dqstart, &dqend, m);
               else
                  InsertInQ(&cqstart, &cqend, m);
            }
         }
      }
      AllocateBlocks(); /* check if we need to allocate more memory */

      /* Protect the operation from the interrupt handler. Not all
      architectures have subtraction directly on memory locataions (the same
      effect as such a machine may also be achieved by a simple compiler
      generating stack-machine code without any optimazation). In such
      a case the events occuring just between the subtraction and assignment
      will be weeped out. Setting the flag notifies the interrupt handler
      that number of pending needs to be re-counted next time an interrupt
      occures (this will recover the dropped event). */
      accessing_pending_events = 1;
      pending_events -= org;
      accessing_pending_events = 0;
   }
}

static void WaitMsg(t_message *copy)
{
   t_message *m;
   do {
      if (give_time_to_system) {
         rest(1);
      }
      CheckDelayedMessages();
      if (pending_events) {
         CheckPendingMessages();
      }
   } while (cqstart == NULL);
   m = DeQ(&cqstart, &cqend);
   *copy = *m;
   ReturnElement(m);
}

/* cgui-internal function, similar to InsertMessage, but it never tries to
   allocate memory and can thus be safely called from within an interrupt.
   If no message object is available the message will be dropped. */
extern int SafeInsertMessage(void (*Action) (void *), void *msg)
{
   t_message *m;
   t_block *bl;
   static int do_count;
   int i;

   if (lastblock == NULL || lastblock->inuse == 0xffffffff)
      return 0;
   if (event_inserting_stopped)
      return 0;
   if (do_count) {
      do_count = 0;
      /* Re-count the pending events, to recover those that has eventually
      been missed */
      pending_events = 0;
      for (bl = firstblock; bl; bl = bl->next) {
         for (i = 0; i < 32; i++)
            pending_events += bl->m[i].pending;
      }
   }
   if (accessing_pending_events)
      do_count = 1;
   m = GetQueElement(Action, msg, 0, 0, 0);
   pending_events++;
   m->pending = 1;
   return m->id;
}
END_OF_FUNCTION(SafeInsertMessage);

/* This is a callback routine, directly called by the timer interrupt. */
extern void CheckEvents(void)
{
   static int ticks = 0;

   ticks++;
   if (!hold) {
      decrements += ticks;
      ticks = 0;
   }
}
END_OF_FUNCTION(CheckEvents);

static void FreeMessageBlocks(t_block *bl)
{
   if (bl) {
      FreeMessageBlocks(bl->next);
      Release(bl);
   }
}

static void DeInitEvent(void *data nouse)
{
   if (firstblock) {
      if (cqstart)
         ExecuteAllPendingEvents();
      remove_int(CheckEvents);
      FreeMessageBlocks(firstblock);
      firstblock = lastblock = NULL;
      cqstart = cqend = dqstart = dqend = NULL;
      event_id = 0;
      decrements = 0;
      pending_events = 0;
      accessing_pending_events = 0;
   }
}

static int InitEvent(void)
{
   static int virgin = 1;
   int starting = 0;

   if (virgin) {
      virgin = 0;
      LOCK_VARIABLE(firstblock);
      LOCK_VARIABLE(lastblock);
      LOCK_VARIABLE(cqstart);
      LOCK_VARIABLE(cqend);
      LOCK_VARIABLE(dqstart);
      LOCK_VARIABLE(dqend);
      LOCK_VARIABLE(hold);
      LOCK_VARIABLE(decrements);
      LOCK_VARIABLE(pending_events);
      LOCK_VARIABLE(accessing_pending_events);

      LOCK_FUNCTION(CheckEvents);
      LOCK_FUNCTION(GetQueElement);
      LOCK_FUNCTION(SafeInsertMessage);
   }
   if (firstblock == NULL) { /* do init */
      starting = 1;
      HookCguiDeInit(DeInitEvent, NULL);
      AllocateBlocks();
      LOCK_FUNCTION(CheckEvents);
      install_int(CheckEvents, 1);
   }
   hold = 0;
   return starting;
}

static int RemoveEvent(t_message **qstart, t_message **qend, unsigned id)
{
   t_message *m, *p;

   if (*qstart) {
      if ((*qstart)->id == id) {
         ReturnElement(*qstart);
         *qstart = (*qstart)->next;
         if (*qstart == NULL)
            *qend = NULL;
         return 1;
      }
      if ((*qstart)->next) {
         for (p = *qstart, m = (*qstart)->next; m; m = m->next, p = p->next) {
            if (m->id == id) {
               p->next = m->next;
               if (p->next == NULL)
                  *qend = p;
               ReturnElement(m);
               return 1;
            }
         }
      }
   }
   return 0;
}

extern void DestroyLinkedEvents(int objid)
{
   t_message *m, *p;

   for (m = dqstart; m; m = p) {
      p = m->next;
      if (m->objid == objid)
         RemoveEvent(&dqstart, &dqend, m->id);
   }

   for (m = cqstart; m; m = p) {
      p = m->next;
      if (m->objid == objid)
         RemoveEvent(&cqstart, &cqend, m->id);
   }
}

static unsigned DoGenEvent(void (*Action) (void *), void *msg, unsigned delay, int objid, int appflag)
{
   t_message *m;

   InitEvent();
   if (event_inserting_stopped)
      return 0;
   m = GetQueElement(Action, msg, delay, objid, appflag);
   if (delay)
      InsertInQ(&dqstart, &dqend, m);
   else
      InsertInQ(&cqstart, &cqend, m);
   AllocateBlocks(); /* check if we need to allocate more memory */
   return m->id;
}

static void IteratorWrapper(void *data)
{
   t_pair *pair = data;
   if (pair->Function(pair->data)) {
      GenEvent(IteratorWrapper, pair, 0, 0);
   } else {
      StopProcessEvents();
   }
}

/* Application interface */

extern int Invite(int key, void *data, char *text)
{
   t_message msg;
   t_meeting a;
   int starting;

   starting = InitEvent();
   a.key = key;
   a.data = data;
   a.text = text;
   a.ack = 0;
   a.next = meeting;
   meeting = &a;
   do {
      WaitMsg(&msg);
      if (msg.Action) {
         msg.Action(msg.msg);
      }
   } while (a.ack == 0);
   if (starting) {
      ExecuteAllPendingEvents();
      DeInitEvent(NULL);
   }
   return 1;
}

extern void *Attend(int key)
{
   t_meeting *a;

   a = meeting;
   if (a) {
      if (a->key == key) {
         meeting = a->next;
         a->ack = 1;
         return a->data;
      }
   }
   return NULL;
}

extern int _KillEventOfCgui(unsigned id)
{
   int found = 0;

   found = RemoveEvent(&dqstart, &dqend, id);
   if (!found)
      found = RemoveEvent(&cqstart, &cqend, id);
   return found;
}

extern int KillEvent(unsigned id)
{
   int found = 0;

   if (id & APPLICATION_FLAG) {
      found = RemoveEvent(&dqstart, &dqend, id);
      if (!found)
         found = RemoveEvent(&cqstart, &cqend, id);
   }
   return found;
}

extern void FlushGenEvents(void)
{
   t_message *m, *p;

   for (m = dqstart; m; m = p) {
      p = m->next;
      if (m->id & APPLICATION_FLAG)
         RemoveEvent(&dqstart, &dqend, m->id);
   }
   for (m = cqstart; m; m = p) {
      p = m->next;
      if (m->id & APPLICATION_FLAG)
         RemoveEvent(&cqstart, &cqend, m->id);
   }
}

extern unsigned GenEvent(void (*Action) (void *), void *msg, unsigned delay, int objid)
{
   return DoGenEvent(Action, msg, delay, objid, APPLICATION_FLAG);
}

extern unsigned _GenEventOfCgui(void (*Action) (void *), void *msg, unsigned delay, int objid)
{
   return DoGenEvent(Action, msg, delay, objid, 0);
}

extern void ProcessEvents(void)
{
   Invite(0x80000000, NULL, "System");
}

extern void StopProcessEvents(void)
{
   Attend(0x80000000);
}

/* Executes all events waiting in the current queue, all delayed events and
   all events pending for insertion in any of the queues. During the
   execution the queues are stopped for input */
extern void ExecuteAllPendingEvents(void)
{
   t_message *m;

   event_inserting_stopped++;
   CheckPendingMessages();
   for (m = dqstart; m; m = m->next)
      m->delay = 0;
   CheckDelayedMessages();
   while (cqstart) {
      m = DeQ(&cqstart, &cqend);
      if (m->Action)
         m->Action(m->msg);
      if (cqstart)
         ReturnElement(m);
   }
   event_inserting_stopped--;
}

extern void CguiYieldTimeslice(int state)
{
   give_time_to_system = state;
}

extern void CguiEventIterateFunction(int (*Function) (void *data), void *data)
{
   t_pair pair = {Function, data};
   GenEvent(IteratorWrapper, &pair, 0, 0);
   ProcessEvents();
}

