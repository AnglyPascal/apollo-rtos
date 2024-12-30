# Executive

## Executive tasks

- priority based, fixed number of tasks 
- can allocate dynamic memory
- core set table: process descriptor table
    - contains all processes 
    - first entry is the current running process
    - context switch just swaps the current process with target 
    - register called NEWJOB holds the highest priority job's priority
        - if it's 0 then the current job is the highest priority 
        - otherwise, the current job must give up 
        - the current job lowers its priority, calls to scheduler, and incrs its
            priority before the switch 
    - this register must be read every 20ms (modifiable) and the executive checks if it
        hasn't been read for over 650ms, and invokes the switching itself 
- jobs can be put to sleep, if waiting for some event, this involves creating a
    waitlist task, which will keep checking for the condition at certain interval, and
    once the event occurs, will wake this job up. 
- jobs can actively lower their priority to allow for other jobs to run
    - once a job lowers its priority, it must wait for all higher priority jobs to
        finish running before it can increase its priority again.

- priority sort of acts as a locking mechanism for the shared memory and resources. 
- if two process shares some memory, then the lower priority process can only access
    the memory once the higher priority process is done


### new_job register

When a process is created, it sets new_job to its priority. The new_job should be
wrapped around by a class, that keep track of whether a running process has queried
new_job in the last 20ms. If not, this generated a timer interrupt/signal to call to
the scheduler.

- new_job register contains the address of the process that's waiting to run
    - so a positive value refers to a valid address
    - and 0, or nullptr indicates that the current job is the highest priority job
- this register should also keep a variable storing the last time it was accessed. 
    - timer interrupts check if this register is checked in the last 640ms
    - if not, executive forces a task switch, 
        - or in an extreme case, cause a restart

### sleeping job

- to put a job to sleep for a certain amount of time, or until certain even happens, a
    waitlist task is created
    - for a timer delay, the waitlist is set up with a delay 
    - for an event, it could be a while loop inside a waitlist, what checks for the
        condition every time it is awoken, or an interrupt handler


## The waitlist

- used to schedule work to perform time-critical functions
- pair of tables, contains reference for 'tasks' scheduled to run in the future
- waitlists tasks run in interrupt-inhibited mode
- restricted to 5ms, but needs to be changed for modern cpus

- two tables: 
    - table 1: start time 
    - table 2: address with banking information
- seems to be a queue?

The start time is relative. Say, new task needs to run 180 ms from now (10ms is the
unit), and two entries already exist in the waitlist, both scheduled to run before this
new request. Say these two tasks have intervals 30ms and 40ms resp. Thus, the new task
will have a start time interval of 110ms = 180 - 30 - 40. 

So it's an ordered list of tasks, ordered by their start time, and the time entry in
the table is relative order from previous task

- timer interrupts are used to transfer control to the waitlist management routine

