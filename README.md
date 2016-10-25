# MIPS Simulator
This repository contains a MIPS 1 processor simulator I wrote in November 2013 in C, as coursework for a computer architecture module. It allows the user to write a binary file endine in `.bin` using a hex editor, then a driver file in C which will set up arguments and place the program in memory, before stepping through the program and exiting.

## Implemented instructions
The full list of commands implemented may be found in `implemented_instructions.csv`. This includes every command from the MIPS 1 instruction set, except for SYSCALL which cannot be implemented using the present layout.

## Usage
In order to use the library, the commands must be read into memory using a driver file, then the function `mips_step(state)` must be called. This function will return a 0 when execution is running normally, and a 1 when an error (such as an overflow, or an illegal instruction) has occurred.


