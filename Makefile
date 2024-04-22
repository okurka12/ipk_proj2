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
ASAN=-fsanitize=address

# uncomment this so that server.c broadcasts join even to the person joined
# DBROAD=-DBROAD

RESULT_BINARY=ipk24chat-server

CC=gcc

CFLAGS=-Wall -Wextra -pedantic -std=c11 -g -Og $(LOGLEVEL) $(DNDEBUG) \
$(ASAN) $(DBROAD)

LDFLAGS=$(ASAN) # -lpthread

MODULES=main.o argparse.o server.o client.o clientlist.o msg.o tcp_parse.o \
tcp_render.o

.PHONY: ALL
ALL: $(RESULT_BINARY)

.PHONY: clean
clean:
	rm -f *.o *.bin $(RESULT_BINARY)

.PHONY: remake
remake: clean ALL

main.o: main.c argparse.h utils.h server.h argparse.h client.h iota.h msg.h
	$(CC) $(CFLAGS) -c -o $@ $<

argparse.o: argparse.c argparse.h utils.h
	$(CC) $(CFLAGS) -c -o $@ $<

client.o: client.c client.h utils.h tcp_parse.h tcp_render.h msg.h iota.h \
server.h argparse.h
	$(CC) $(CFLAGS) -c -o $@ $<

server.o: server.c server.h argparse.h client.h utils.h client.h \
clientlist.h server.h argparse.h client.h msg.h
	$(CC) $(CFLAGS) -c -o $@ $<

clientlist.o: clientlist.c clientlist.h server.h argparse.h client.h \
client.h server.h argparse.h client.h utils.h msg.h
	$(CC) $(CFLAGS) -c -o $@ $<

msg.o: msg.c msg.h utils.h
	$(CC) $(CFLAGS) -c -o $@ $<

tcp_parse.o: tcp_parse.c tcp_parse.h msg.h utils.h iota.h
	$(CC) $(CFLAGS) -c -o $@ $<

tcp_render.o: tcp_render.c msg.h utils.h



$(RESULT_BINARY): $(MODULES)
	$(CC) $(LDFLAGS) -o $@ $(MODULES)
