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

CC=gcc

CFLAGS=-Wall -Wextra -pedantic -std=c11 -g -Og $(LOGLEVEL) $(DNDEBUG) $(ASAN)

LDFLAGS=$(ASAN) # -lpthread

MODULES=main.o argparse.o server.o client.o clientlist.o

.PHONY: ALL
ALL: $(RESULT_BINARY)

.PHONY: clean
clean:
	rm -f *.o *.bin $(RESULT_BINARY)

.PHONY: remake
remake: clean ALL

main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ $<

argparse.o: argparse.c
	$(CC) $(CFLAGS) -c -o $@ $<

client.o: client.c
	$(CC) $(CFLAGS) -c -o $@ $<

server.o: server.c
	$(CC) $(CFLAGS) -c -o $@ $<

clientlist.o: clientlist.c
	$(CC) $(CFLAGS) -c -o $@ $<


$(RESULT_BINARY): $(MODULES)
	$(CC) $(LDFLAGS) -o $@ $(MODULES)
