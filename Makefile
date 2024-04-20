##################
##  Vit Pavlik  ##
##   xpavli0a   ##
##    251301    ##
##     IPK2     ##
##     2024     ##
##################

# log level (DEBUG, INFO, WARNING, ERROR, FATAL)
LOGLEVEL=-DLOGLEVEL=DEBUG

# uncomment this to disable all logging
# DNDEBUG=-DNDEBUG

# uncomment this to enable adress sanitizer
# ASAN=-fsanitize=address
# ASAN=-fsanitize=thread  # doesn't work?

RESULT_BINARY=ipk24chat-server

CC=tcc

CFLAGS=-Wall -Wextra -pedantic -std=c11 -g -Og $(LOGLEVEL) $(DNDEBUG) $(ASAN)

LDFLAGS=$(ASAN) # -lpthread

MODULES=main.o argparse.o mmal.o gexit.o server.o

.PHONY: ALL
ALL: $(RESULT_BINARY)

.PHONY: clean
clean:
	rm -f *.o *.bin $(RESULT_BINARY)

.PHONY: remake
remake: clean ALL

main.o: main.c argparse.h gexit.h utils.h server.h iota.h
	$(CC) $(CFLAGS) -c -o $@ $<

argparse.o: argparse.c argparse.h mmal.h gexit.h utils.h
	$(CC) $(CFLAGS) -c -o $@ $<

mmal.o: mmal.c mmal.h gexit.h
	$(CC) $(CFLAGS) -c -o $@ $<

gexit.o: gexit.c gexit.h utils.h
	$(CC) $(CFLAGS) -c -o $@ $<


server.o: server.c server.h argparse.h utils.h
	$(CC) $(CFLAGS) -c -o $@ $<


$(RESULT_BINARY): $(MODULES)
	$(CC) $(LDFLAGS) -o $@ $(MODULES)
