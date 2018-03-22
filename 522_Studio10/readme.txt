Name: Zimu Wang, Wei Weng, Diqiu Zhou
2.
pi@raspberrypi:~/Desktop/Operating System/522_Studio10 $ ./memory 
arena: 135168, ordblks: 1, smblks: 0, hblks: 0, hblkhd: 0, usmblks: 0, fsmblks: 0, uordblks: 56, fordblks: 135112, keepcost: 135112 
arena: 135168, ordblks: 1, smblks: 1, hblks: 0, hblkhd: 0, usmblks: 0, fsmblks: 56, uordblks: 0, fordblks: 135168, keepcost: 135112 

pi@raspberrypi:~/Desktop/Operating System/522_Studio10 $ ./memory 
arena: 135168, ordblks: 1, smblks: 0, hblks: 0, hblkhd: 0, usmblks: 0, fsmblks: 0, uordblks: 208, fordblks: 134960, keepcost: 134960 
arena: 135168, ordblks: 1, smblks: 0, hblks: 0, hblkhd: 0, usmblks: 0, fsmblks: 0, uordblks: 0, fordblks: 135168, keepcost: 135168 

arena: total amount of usable memory
ordblks: number of continuous free memory regions. It's 1 in both cases because we allocated memory at the beginning or the end of the memory region.
hblks: This should be corresponding to memory mapped files (open and read from files)
hblkds: Size of memory used for mmaped region
uordblks: Memory space allocated
fordblks: memory space freed. It is different for allocating memory size of 50 and larger value is that memory size of 50 is not restored to reusable memory space.
keepcost: remaining usable memory

3.
Double free:
pi@raspberrypi:~/Desktop/Operating System/522_Studio10 $ MALLOC_CHECK_=1 ./3
*** Error in `./3': free(): invalid pointer: 0x01f87008 ***
Aborted

Free non-zero uninitialized:
pi@raspberrypi:~/Desktop/Operating System/522_Studio10 $ MALLOC_CHECK_=1 ./3
*** Error in `./3': free(): invalid pointer: 0x0038e995 ***
Aborted

Realloc freed:
pi@raspberrypi:~/Desktop/Operating System/522_Studio10 $ MALLOC_CHECK_=1 ./3
*** Error in `./3': realloc(): invalid pointer: 0x00ffa008 ***
Aborted

Writing string into freed region:
This seems to be fine and was not detected.

Write beyond limit and not freeing:
This is fine as well.

4.
Monitoring:
root@raspberrypi:/home/pi/Desktop/Operating System/522_Studio10# ./monitoring /sys/fs/cgroup/memory/child1/cgroup.event_control /sys/fs/cgroup/memory/child1/memory.oom_control
Opening /sys/fs/cgroup/memory/child1/cgroup.event_control
first string 3 5
Message received 67072

Monitored doesn't have output, except showing killed when exceeding the memory limit

5.
Monitoring:
root@raspberrypi:/home/pi/Desktop/Operating System/522_Studio10# ./monitoring /sys/fs/cgroup/memory/child1/cgroup.event_control /sys/fs/cgroup/memory/child1/memory.oom_control /sys/fs/cgroup/memory/child2/cgroup.event_control /sys/fs/cgroup/memory/child2/memory.oom_control
Opening /sys/fs/cgroup/memory/child1/cgroup.event_control
Opening /sys/fs/cgroup/memory/child2/cgroup.event_control
first string 3 6
first string 4 8
10000 seconds elapsed. 
Message received 1994394600

Monitored both outputed killed when reaching memory limit


