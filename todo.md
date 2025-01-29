# TODO:

- implement manual process termination 
    - terminate when assert fails or some other things
    - how will the resouce be released up on an untimely death?

- restart mechanism
    - method for adding checkpoints
    - default recover methods: do nothing, restart proc 
        - procs can maintain internal state for recovery
    - streamline process creation and restart installation
        - do it like bash cmds
    - three levels of protection:
        - no protection 
        - restart from runtime information stored on ram
        - restart from non-volatile storage

    * table with info for each process
    * remove entry upon process termination

- nvm
    - how to address a particular word?
    - the current nvm_t doesn't really need to be page aligned, 
        but the erase method cannot be called unless it's page aligned

- fs
    - is causing problems on the actual device 
    - check that no assumptions are being made about the file contents
        - clear out runtime map of the file, and don't load it 

- reset recovery:
    - recovery section variables needs explicit initialization by code
    - and this needs to happen only when booting for the first time
    - needs methods for initializing these variables properly

- memory trace:
    - why doesn't it print free?


## BUG frame 1:

```
x: 61, y: 3,
assertion failed ``intr_pid != null_pid``, in i2c0_spi0_handler, at /home/ahsan/project/src/i2c.cpp:184
!!! WTF !!!
pc: 0x40002000, lr: 0x0000011c
sched:
        0. idle_proc : 1, [runnable]
                stack: 0x20003b10, sz: 160, stk_ptr: 0x20003f8c
        1. test_proc : -5, [asleep]
                stack: 0x200039d0, sz: 224, stk_ptr: 0x20003a74
        2. accel_bg : 32, [running]
                stack: 0x200037d0, sz: 416, stk_ptr: 0x200038bc
                0x20000674: 0
        3. display : -2, [asleep]
                stack: 0x20003650, sz: 288, stk_ptr: 0x2000372c
                0x20000698: 0
        5. accel : -30, [asleep]
                stack: 0x200032d0, sz: 288, stk_ptr: 0x2000336c
waitlist:
        display
                interval: 16
        test_proc
                interval: 16
        accel
                interval: 80
filesystem:
        5. sz: 1020, pg: 0x0000b000, rt: 0x200010a8
```

## BUF frame 2:

```
x: 54, y: 19,
assertion failed ``proc->priority < 0 && proc->state == state_t::ASLEEP``, in wakeup, at /home/ahsan/project/src0
wakeup: "accel_bg" not asleep
!!! WTF !!!
pc: 0x40002000, lr: 0x0000011c
sched:
        0. idle_proc : 1, [runnable]
                stack: 0x20003b10, sz: 160, stk_ptr: 0x20003f8c
                0x2000062c: 0
        1. test_proc : -5, [asleep]
                stack: 0x200039d0, sz: 224, stk_ptr: 0x20003a74
        2. accel_bg : 32, [running]
                stack: 0x200037d0, sz: 416, stk_ptr: 0x200038bc
                0x20000674: 0
        3. display : -2, [asleep]
                stack: 0x20003650, sz: 288, stk_ptr: 0x2000372c
        5. accel : -30, [asleep]
                stack: 0x200032d0, sz: 288, stk_ptr: 0x2000336c
waitlist:
        display
                interval: 16
        test_proc
                interval: 32
        accel
                interval: 64
filesystem:
        5. sz: 1020, pg: 0x0000b000, rt: 0x200010a8
```
