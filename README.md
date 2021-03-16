# Unix System Inspector
Dustin (Yen Dah) Hsiang

## About
A Unix system inspector utility giving the user an activity monitor on the system and hardware information with processes and tasks in a top-like format. All processes and tasks information are retrieved from the '/proc/[PID]/status' file.

### What is proc?
It is a virtual file system for Unix operating systems. It contains runtime system information for each individual task or process, and hardware configuration. Each system process and task is a number directory within the proc file system.

### Program Options
The display can be toggled with the following options:

```bash
$ ./inspector -h
Usage: ./inspector [-ho] [-i interval] [-p procfs_dir]

Options:
    * -h              Display help/usage information
    * -i              Task List
    * -p procfs_dir   Set the expected procfs mount point (default: /proc)
    * -o              Operate in one-shot mode (no curses or live updates)
```

### Included Files
There are several files included. These are:
   - <b>Makefile</b>: File used to compile and run the program and test cases.
   - <b>inspector.c</b>: The main driver for the program.
   - <b>display.c</b>: Functions to display the program in an activity monitor format to the user.
   - <b>procfs.c</b>: Functions to get the system and hardware information of the system. This information includes hostname, kernel version, uptime, CPU usage, etc.
   - <b>util.c</b>: Utility functions to give desired output. Functions include drawing out the percentage bar for CPU and memory Usage and getting user from UID.
   - <b>helper.c</b>: Includes helper functions such as reading in a file line and return the line as buffer, getting the next token based on delimeter, and checking if a directory name is all digits.

### Program Output

```bash
Hostname: dustin-vm | Kernel Version: 5.4.13
CPU: AMD EPYC Processor (with IBPB), Processing Units: 2
Uptime: 26 days, 21 hours, 26 minutes, 18 seconds

Load Average (1/5/15 min): 0.02 0.02 0.32
CPU Usage:    [--------------------] 2.4%
Memory Usage: [############--------] 62.9% (0.6/1.0 GB)

Tasks: 97 total
1 running, 0 waiting, 96 sleeping, 0 stopped, 0 zombie

      PID |                 Task Name |        State |             User
----------+---------------------------+--------------+-----------------
   260015 |                 inspector |      running |             root
```


To compile and run:

```bash
make
./inspector
```

## Testing

To execute the test cases, use `make test`. To pull in updated test cases, run `make testupdate`. You can also run a specific test case instead of all of them:

```
# Run all test cases:
make test

# Run a specific test case:
make test run=4

# Run a few specific test cases (4, 8, and 12 in this case):
make test run='4 8 12'
```
