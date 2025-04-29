TODO
----

## hardware setup

- gpio namespace, setup buttons
- setup magnetometer
- radio


## new features

- trace unification, and dump signal
- message pass signals


## Testing

* tests that need to be done
    - waitlist time intervals are strictly maintained
        - need to keep one timer running even after interrupts are turned on before and
            after waitlist
    - waitlist doesn't take too long to run

    - recover with stored data


## Report:

- Move the codebase structure section to the appendix
- make listings font slightly smaller
- the file system implementation is too verbose
- modify the codebase appendex
- write about the new hardfault handler and error handling

- talk about fragmentation issue

- conveniences: cmake, linker script, process creation and testing macros
- testing framework
    - write about the tests i've done

- argue that apollo-rtos is not modular, it's monolithic. So it's not straightforward
    to produce a minimal build with a selected subset of features
    - talk about it in the architecture section

- The small potential fixes to the RTS should be in the Evaluation section rather than
    the Conclusions. The future work in the conclusions should be a whole new project
    for another student.

- The conclusion chapter should have a Reflections section where you talk about what
    you have learned doing the project and what you would do differently.

