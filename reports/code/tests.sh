args_parser:
    all_cases                      # tests for argument parsing correctness

bitset:
    basic_operations               # bitset creation, and basic bitwise operations
    iteration                      # iteration over set bits

circular_buffer:
    basic_operations               # push/pop operations in normal usage
    capacity_and_overwrite         # behavior when buffer capacity is exceeded
    edge_cases                     # handling full/empty buffers and boundary conditions
    wrap_around_and_iteration      # correct behavior during wrap-around scenarios

stack:
    basic_integer_operations       # push/pop with integer data
    edge_cases                     # handling empty stack, overflow prevention
    order_preservation             # LIFO (Last-In-First-Out) behavior verification
    struct_operations              # storing and manipulating struct data
    template_specialization        # tests for templated stack specializations

static_list:
    basic_operations               # insert, remove, and traversal operations
    edge_cases                     # boundary tests such as empty list handling
    iteration_functionality        # iterator support and traversal correctness
    node_management                # memory reuse and node handling tests

allocator:
    basic_allocation_and_reuse     # basic alloc/free scenarios and reuse
    edge_cases                     # allocating 0 bytes, handling invalid frees
    freelist_management            # freelist integrity after allocs and deallocs
    memory_reuse_scenarios         # stress tests for memory reuse patterns
    rounding                       # block size rounding correctness tests

malloc:
    basic_malloc_free              # basic malloc/free cycle validation
    cleanup_after_process          # automatic memory cleanup on process exit
    complex_pattern_with_cleanup   # malloc/free with cleanup validation
    null_and_zero_handling         # behavior when allocating 0 bytes or NULL freeing
    process_isolation              # memory isolation between concurrent processes

memlib:
    memcmp_test                    # memory comparison correctness
    memcpy_test                    # memory copy correctness
    memset_test                    # memory set (initialization) correctness
    strcmp_test                    # string comparison correctness
    strcpy_test                    # string copy correctness

file_system:
    fs_basic:
        mmap                       # memory-mapped file I/O tests
        multi_blk_file             # multi-block file allocation and access
        open_store_close           # file open, store, and close cycle verification
    fs_contention:
        write_contention           # concurrent file writing contention scenarios
    fs_mapping:
        unshared_mapping           # behavior of private (unshared) file mappings
    ifstream:
        basic_read_write           # sequential read/write correctness
    ofstream:
        basic_write                # basic output file stream functionality
        write_redirection          # output redirection and formatted writes

scheduler:
    sched_basic:
        sleep                      # sleep and wakeup scheduling tests
        channel_wakeup             # wakeup through communication channels
        priority_order             # priority-based task execution ordering
        round_robin                # fairness among equal-priority tasks
        timer_duration             # sleep duration validation
    sched_concurrency:
        sleep_notify               # IPC through channel
        mutex_exclusion            # mutual exclusion via mutexes
    sched_fairness:
        sleep_fairness             # fairness among sleeping tasks
        yield_fairness             # fairness during voluntary yielding
    waitlist:
        waitlist_basic             # basic task scheduling using the waitlist
        waitlist_fs_op             # waitlist usage during file system operations
