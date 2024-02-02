# Project Phase 3 Part 2 Report

> Mihir Bani, 2019113003  
> Samay Kothari, 2019113017

# Update Command

This command updates the column of the give relation. This operation allows multiple READS and WRITES simultaneously. For handling this, we use `flock` to create a lock on the page file.

The steps in Update are:

-   **Read Pages:** The pages of the Table are brought to Main Memory, and lock is initiated on the Page. The `Page` constructor is modified to handle this locking, it also involves use of `dup` and `dup2` to change the STDIN to page.

```c++
Page::Page(string tableName, int pageIndex, bool isUpdate)
{
    // same code as before
    if (!isUpdate)
    {
        // same code as before
    }
    else
    {
        FILE_DESCRIPTOR = open(pageName.c_str(), O_APPEND | O_CREAT);
        int savedCin = dup(STDIN_FILENO);
        dup2(FILE_DESCRIPTOR, STDIN_FILENO);
        cout << "Waiting for lock" << endl;
        flock(FILE_DESCRIPTOR, LOCK_EX);
        cout << "Received Lock for page: " << this->pageName << endl;

        // ... code for writing to cin
        dup2(savedCin, STDIN_FILENO);
    }
}

```

-   **Update Pages:** The values are updated. A small sleep is present between the operations on consecutive pages.
-   **Write Pages:** The page is written back to files, and the csv file is also updated with help of `makePermanent` function. The `flock` is unlocked now, so that some other process can have access to this page file. The `writePage()` of `Page` class is modified to handle locking, also involving `dup` and `dup2` usage.

```c++
void Page::writePage(bool isUpdate)
{
    logger.log("Page::writePage");
    if (!isUpdate){
        // same code as before
    }
    else
    {
        int fd = open(pageName.c_str(), O_WRONLY | O_TRUNC);
        int savedOut = dup(STDOUT_FILENO);
        dup2(fd, STDOUT_FILENO);
        // ... code for writing to cout
        dup2(savedOut, STDOUT_FILENO);
        flock(FILE_DESCRIPTOR, LOCK_UN);
        close(fd);
        close(FILE_DESCRIPTOR);
        close(savedOut);
        cout << "Lock Released " << this->pageName << endl;
    }
}

```

Locking the page while reading and unlocking it after writing back to file ensures that any other process cant access the content or modify the page. The other process has to wait till the lock is available and then resumes operation. A global variable `FILE_DESCRIPTOR` is kept to keep track of the locked page's file descriptor.

This ensures that ACID properties are followed and Lost Update Problem is alse taken care of.

## Theoretical Analysis

Conflicting Operations:

1. **Two Simultaneous reads on the table:**  
   Two simultaneous processes can read the page of the tables concurrently since there is no updation of the data. So multiple reads on page is allowed without leading to any erroneous results.
2. **Two Simultaneous reads and writes on the table:**  
   This condition is possible when one of the processes, writes on the page while another instance reads from the same page. This can lead to erroneous results. So while writing if we put a lock, such that for reading the process has to wait for this lock.
3. **Two Simultaneous writes on the table:**  
   When both the simultaneous processes want to update the data. In this case also, the result can be erroneous and can lead to a lost update problem. This can be solved by using a lock before read/write, so that the other process has to wait before earlier update is finished.

## Video Link

[Multiple Update Video](https://iiitaphyd-my.sharepoint.com/:v:/g/personal/samay_kothari_research_iiit_ac_in/EUax2rINUphLo0do9aaE3zABIpMspPfvLLc1EHrJg_WH-A?e=YoIMcl)
