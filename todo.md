
- implement manual process termination 
    - terminate when assert fails or some other things
    - how will the resouce be released up on an untimely death?

- restart mechanism
    - method for adding checkpoints
    - default recover methods: do nothing, restart proc 
        - procs can maintain internal state for recovery

    * table with info for each process
    * remove entry upon process termination

- nvm
    - how to address a particular word?
    - the current nvm_t doesn't really need to be page aligned, 
        but the erase method cannot be called unless it's page aligned
