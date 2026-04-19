Part 1 : Static Memory Allocator

The goal was to implement a basic memory allocator using a static array of 1MB. Instead of relying on the system's malloc, we managed memory manually using a cursor that advances through the array with each allocation.

Key concepts covered:

    Memory alignment with _Alignas and max_align_t
    Pointer arithmetic
    A header (t_block) stored just before each allocated block containing its size, a pointer to the next block, and its availability status.


Part 2 : Free and Block Recycling

The goal was to implement static_free and allow previously freed blocks to be reused by future allocations.
Key concepts covered:

A separate linked list of free blocks (free_block).When freeing, the block is pushed onto the free list instead of being erased.
When allocating, the free list is checked first before advancing the cursor.If a suitable free block is found it is reused, otherwise a new block is created at the cursor
