# Project Phase-2 Report

## JOIN

### NESTED

-   Read all the blocks of both relations in a nested loop and then compare the operator.
-   nB-2 blocks of relation R, 1 Block for relation S and 1 Block for resulting relation in the main memory is kept.
-   As soon as a block of resulting relation is complete, we dump it to file.
-   Implemented for all the binary operations

Assumptions:

-   Minimum Buffer size in argument is 3.

### PARTHASH

-   It first makes partitions of both the relations according to the Buffer Size. The hash function is `key % nB`. (nB = Buffer size, key = value of column)
-   The partitions are alse saved as pages, and if multiple values have the same hash value resulting to a big partition, then this partition is stored in mulitple pages.
-   Then load the partitions into memory for joining.
-   Implemented for equality binary operator. For all binary operators other than `EQUAL`, we use `NESTED JOIN` internally.

**Performance Comparison:** (for binary_operator != EQUAL)  
In PARTHASH join we must make partitions and then load the partitions in memory. But if the binary operator is not EQUAL, then the point of making partitions is useless. As now we will have to read all the partitions from both the relations in a nested loop. This is very similar to using NESTED join itself.
The benefit of making partitions was that we can limit our search space, but with other operators we will have to compare all the rows.

Moreover, if majority rows point to the same hash value, then the partitions created will have multiple pages. So the total number of block access will increase even further. As now we have to load all the nB-1 partitions and then in some of these partions there will be mulitple blocks to load. In the best case scenario, when all the rows are equally divided in the partitions then the total number of blocks will be minimized, and will be same as in the NESTED join case.

Assumptions:

-   Minimum Buffer size in argument is 2.

## Block Count

For counting the total number of blocks read/written from/to the disk. We maintain a global counter `BLOCK_ACCESS_COUNT`. Its value is incremented in the functions of BufferManager which handle reading/writing a block from/to memory.

-   `BufferManager::insertIntoPool` : reading the block from file and adding it to the main memory (in the pool).
-   `BufferManager::insertPartitionIntoPool` : reading the partition block from file and adding it to the main memory (in the pool).
-   `BufferManager::writePage` : writing the block from memory to file.
