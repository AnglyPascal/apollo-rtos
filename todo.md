# TODO:

- nvm
    - how to address a particular word?
    - the current nvm_t doesn't really need to be page aligned, 
        but the erase method cannot be called unless it's page aligned

- linux modules? installable code?

- block device abstraction
    - file system rewrite

- memory management bug
    - shell buffer alloc dealloc is going haywire 
    - shell stops working after a couple pkills

```
boot
pkill heart
Process not found
heart
pkill heart

assertion failed ``proc->priority < 0``, in default_alarm, at /home/ahsan/project/src/sched.cpp:184
alarm: "heart" not asleep
hardfault
pc: 0x00002438, lr: 0x000018ef
```
