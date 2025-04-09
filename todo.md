TODO
----

## hardware setup

- setup a different timer for independent time keeping
- gpio namespace, setup buttons
- setup magnetometer
- radio


## new features

- trace unification, and dump signal
- message pass signals
- character files


## Testing

- each test then might spawn some processes, and then it waits for those processes
    to return
    - this could be done manually, using some shared variables
        - for now this is the solution
    - could be made into a feature

* tests that need to be done
    - scheduler ordering is based on the priority
    - waitlist time intervals are strictly maintained
        - need to keep one timer running even after interrupts are turned on before and
            after waitlist
    - waitlist doesn't take too long to run


## Report:

- write about the difference in my waitlist implementation vs AGC's
- argue that apollo-rtos is not modular, it's monolithic. So it's not straightforward
    to produce a minimal build with a selected subset of features
