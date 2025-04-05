#TODO:

## Error handling:

- fatal: causes hardfault, requires manual intervention, clears out the hardware setup?
- error: causes soft reset, flushes i2c etc? 
- warn: terminates the current process by sending a kill signal


## hardware setup

- setup a different timer for independent time keeping
- gpio namespace, setup buttons
- setup magnetometer
- radio

## new features

- trace unification, and dump signal
- message pass signals
- character files

- why can you press ctrl-D and cause reset when the reset table is being set up??


## Testing

- automatic registration of tests:
    - tests are functions
    - the top level testing framework process calls the tests one by one
    - each test then might spawn some processes, and then it waits for those processes
        to return <-- TODO -->
        - this could be done manually, using some shared variables
            - for now this is the solution
        - could be made into a feature
    - need a way to turn off start-up processes: shell, display etc

* tests that need to be done
    - scheduler ordering is based on the priority
    - waitlist time intervals are strictly maintained
        - need to keep one timer running even after interrupts are turned on before and
            after waitlist
    - waitlist doesn't take too long to run
    - 


## Report:

- Write about the difference in my waitlist implementation vs AGC's

