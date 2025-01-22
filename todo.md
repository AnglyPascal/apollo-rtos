
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

