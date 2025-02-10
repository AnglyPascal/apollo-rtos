# TODO:

- nvm
    - how to address a particular word?
    - the current nvm_t doesn't really need to be page aligned, 
        but the erase method cannot be called unless it's page aligned

- linux modules? installable code?

- block device abstraction
    - file system rewrite
    - block size is kinda determined by how fast it writes
    - i2c master has freq 100Kbps -> 100b ~ 12B per ms

- setup a different timer for independent time keeping

- memory management bug
    - shell buffer alloc dealloc is going haywire 
    - shell stops working after a couple pkills

- gpio namespace 
- trace unification, and dump signal
- disable intr before and after context switch??

