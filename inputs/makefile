SIM_AUX_SOURCES := $(wildcard ../mips_simulator_*.c) $(wildcard ../mips_simulator_*.cpp)
SIM_OBJECTS := ../mips_simulator.o $(subst .c,,$(subst .cpp,,$(SIM_AUX_SOURCES)))

# This sets the include directory to pick up mips.h from the driver program
CPPFLAGS += -I..
# Turn on all warnings
CPPFLAGS += -W -Wall
CC = g++

ifeq ($(OS),Windows_NT)
# Needed to bring in endian-ness conversion on MinGW
LDLIBS += -lws2_32
endif

%_driver : %_driver.o $(SIM_OBJECTS)
	$(CC) -o $@ $(LDFLAGS) $^ $(LDLIBS)

%_run : %_driver
	./$*_driver

# Will build then run all the test programs
all : t1_driver t2_driver t3_driver t4_driver t5_driver t6_driver t7_driver
	./t1_driver
	./t2_driver
	./t3_driver
	./t4_driver
	./t5_driver
	./t6_driver
	./t7_driver
