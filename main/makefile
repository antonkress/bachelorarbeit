IDIR =inc
CC=gcc
CFLAGS+=-I$(IDIR) -O3 -Wall -pthread -g

ODIR=src
LDIR =../lib
IMGDIR=img
OUTDIR=output
RSDIR=raspistill

LIBS=-lm -lgpiod -lwiringPi -lpigpio -lrt

_DEPS = main.h config.h utils.h color_detection.h sorting_control.h 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o utils.o color_detection.o sorting_control.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_DEPS_PIPE = testgpiod.h light_gate.h utils.h
DEPS_PIPE = $(patsubst %,$(IDIR)/%,$(_DEPS_PIPE))

_OBJ_PIPE = testgpiod.o light_gate.o utils.o
OBJ_PIPE = $(patsubst %, $(ODIR)/%,$(_OBJ_PIPE))

_DEPS_MOTOR = test_motor.h config.h utils.h color_detection.h sorting_control.h 
DEPS_MOTOR = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ_MOTOR = test_motor.o utils.o color_detection.o sorting_control.o 
OBJ_MOTOR = $(patsubst %, $(ODIR)/%,$(_OBJ_MOTOR))

_DEPS_PIGPIO = test_pigpiod.h config.h 
DEPS_PIGPIO = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ_PIGPIO = test_pigpiod.o
OBJ_PIGPIO = $(patsubst %, $(ODIR)/%,$(_OBJ_PIGPIO))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

runColorDetection: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

pipe: $(OBJ_PIPE)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

testmotor: $(OBJ_MOTOR)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

testpigpio: $(OBJ_PIGPIO)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
