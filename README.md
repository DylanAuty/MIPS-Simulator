# MIPS Simulator
This repository contains a MIPS 1 processor simulator I wrote in November 2013 in C, as coursework for a computer architecture module. It allows the user to write a binary file ending in `.bin` using a hex editor, then a driver file in C which will set up arguments and place the program in memory, before stepping through the program and exiting.

## Implemented instructions
The full list of commands implemented may be found in `implemented_instructions.csv`. This includes every command from the MIPS 1 instruction set, except for SYSCALL which cannot be implemented using the present layout.

## Usage
In order to use the library, the commands must be read into memory using a driver file, then the function `mips_step(state)` must be called. This function will return a 0 when execution is running normally, and a 1 when an error (such as an overflow, or an illegal instruction) has occurred.

The testing (driver) files illustrate this process.

### Driver files
The files involved in each driver are:
- `tn_driver.c` - `n = [1-4]` authored by David Thomas, `n = [5-7]` authored by me.
- `tn_input-mips.bin` - `n = [1-4]` authored by David Thomas, `n = [5-7]` authored by me.
- `driver_helper.c` - Authored by David Thomas.

where `n` is the number of the test. Note that tests 1 - 5 were written and provided by the lecturer, David Thomas. These will not output anything provided no error occurs. 

`t5_driver.c` and `t6_driver.c` were written by myself, and will print out the contents of the registers after execution of each instruction in the `.bin` file. If there is any error in execution, the error thrown will provide the arguments, the simulation's return value, and the expected correct value.

Of these tests, `t6_driver.c` is the most complete, and the results of this testing can be seen in `T6_LOG.xlsx` where bold outputs indicate a desired output. Only registers `$4`, `$5`, and `$6` are used. Output is always written to `$6`.

#### Makefile
The makefile included was authored by David Thomas, and modified by me. It compiles the processor and the tests, and then executes the tests in the order specified within the text file.

## Processor files
The main body of the processor was implemented in `mips_simulator.cpp`, which was written by myself. The API was detailed in `mips.h`, which was provided as part of the specification.

