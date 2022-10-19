# Tea-host
Time, Event, Action programming veneer allows complex, precisely timed programming without an RTOS simply by programming different.
All code is run as a menagerie of actions.
Actions are atomic units of code that
run in under a millisecond and take no arguments nor leave a result but they get work done:
```
void some_action();
```
Actions are run sequentially from a global queue which is fed by any action.
A loop becomes a state machine which starts, runs for its duration as a sequence of actions, and then finishes.
State machines become the programming structure supporting simple event-action pairs, to complex context-laden multi state machines.
Actions are run from other actions, events or a time event.
When there are no actions to do, the program sleeps.
When there are no actions to do and nothing is sleeping, the program ends.
State machines may be linked together by event-action binding. State machine 1 binds an action to state machine 2's published event.
When state machine 2 gets to that event, the action from state machine 1 is run.
An action can queue an action: 
```
later(action0);
```
An action can bind an action to an event:
```
when(event1, action1);
```
An action can happen after a period of time: 
```
after(msec(145), action2);
```
For computers, nanosleep() and gettimeofday() provide a time reference.
For embedded systems, this is tied to hardware and interrupts.

This programming methodology promotes:
1. better factored code
2. smaller operating stack footprints
3. no need for multi tasking support or extra memory requirements
4. do what is needed now and defer to later for more actions
5. shallow programs
6. no scheduling nightmares
7. portability across environments
8. more dependable programs
9. more predictable operations
10. easier debugging
11. smaller code
