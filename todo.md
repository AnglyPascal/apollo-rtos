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
