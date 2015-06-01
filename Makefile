CC=gcc
#CFLAGS=-O3 -Wall
CFLAGS=-ggdb -Wall 
LIBS=-lm -lreadline -ltermcap

OBJECTS=mybb.o storage.o stack.o parser.o executor.o runtime.o \
		tables.o list.o utilities.o functions.o

basic: $(OBJECTS)
#	$(CC) -pg -o mybb  $(OBJECTS) $(LIBS)
	$(CC) -o mybb $(OBJECTS) $(LIBS)

clean: 
	rm -f $(OBJECTS) mybb

mybb.o: mybb.c mybb.h errors.h operators.h
storage.o: storage.c mybb.h errors.h operators.h
stack.o: stack.c mybb.h errors.h operators.h
parser.o: parser.c mybb.h errors.h operators.h
executor.o: executor.c mybb.h errors.h operators.h
runtime.o: runtime.c mybb.h errors.h operators.h
list.o: list.c mybb.h errors.h operators.h
tables.o: tables.c mybb.h errors.h operators.h
utilities.o: utilities.c mybb.h errors.h operators.h
functions.o: functions.c mybb.h errors.h operators.h
