# IPK24CHAT server

This is a server application for the IPK24CHAT protocol. It is my solution
for the second project for the **IPK** course. This assignment variant has
codename **IOTA**.

Author: Vit Pavlik (`xpavli0a`)

Date: April 22nd, 2024

See the assignment here: https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%202/iota

## Build instructions

Server is written in the C programming language. It uses `epoll`, therefore
it's linux-specific.

### Prerequisites

The server was developed on Debian GNU/Linux, but except for
`epoll`, nothing non-portable was used.

Here are the software versions known to work:

- Debian `12.5` with glibc `2.36` and Linux `5.15`
- GNU Make `4.3`
- gcc `13.1.0`

### Configuring

In `Makefile`, there is a variable `DBROAD`. When it's commented out,
the server makes sure not to broadcast the
`{DisplayName} has joined {channel}` message to the client joining.

This might make chatting on the server a little more pleasant for the
users.

### Build process

- Running `make` will build the `ipk24chat-server` binary

## Usage

`ipk24chat-server [-l ADDRESS] [-p PORT] [-d TIMEOUT] [-r RETRIES] [-h]`

### Options

- `-l ADDRESS` - the adress to listen on, for example `0.0.0.0` or `127.0.0.1`
  - default is `0.0.0.0`
- `-p PORT` - the port to listen on
  - default is `4567`
- `-d TIMEOUT` - UDP Retransmission timeout in miliseconds
  - default is 250
- `-r RETRIES` - number of UDP retransmissions
  - default is 3
- `-h` - print help and exit

## Theoretical background

Like the client application in the first project, server also needs to
multiplex many I/O streams. This time, it needs to manage network connection
and there can be a lot of them. For that, it is good to use the `epoll` I/O
notification facility, which is good at handling many I/O streams and one
can save a small amount of data for each monitored stream. [1]

## Implementation approach

I decided to somehow encapsulate the client instances into `struct client`
in the `client` module (see `client.h` and `client.c`). That module contains
everything necessary to process client-sent messages and to send individual
responses.

A notable property of the `client` module is that most of the code
is shared for TCP and UDP.

To enable communication of multiple clients, the server needs to have the
clients in a list of some sort, so it can broadcast messages between all of
them. I solved that partly in `server.c` and `clientlist.c`

## Testing

### UDP port switching

To test if client and server can initiate a communication nicely,
I used `tcpdump -X` which monitors traffic according to the chosen filters
and prints them like `hexdump`. That way, one can easily inspect the packets
sent.

```
sudo tcpdump -X -l -i lo ip and (udp  or icmp)

tcpdump: verbose output suppressed, use -v[v]... for full protocol decode
listening on lo, link-type EN10MB (Ethernet), snapshot length 262144 bytes
21:56:42.349229 IP localhost.56039 > localhost.4567:  wb-101! (invalid)
        0x0000:  4500 003b 5356 4000 4011 e959 7f00 0001  E..;SV@.@..Y....
        0x0010:  7f00 0001 dae7 11d7 0027 fe3a 0200 4575  .........'.:..Eu
        0x0020:  7365 726e 616d 6500 6469 7370 6c61 796e  sername.displayn
        0x0030:  616d 6500 7365 6372 6574 00              ame.secret.
21:56:42.349339 IP localhost.4567 > localhost.56039: UDP, length 3
        0x0000:  4500 001f 5357 4000 4011 e974 7f00 0001  E...SW@.@..t....
        0x0010:  7f00 0001 11d7 dae7 000b fe1e 0000 45    ..............E
21:56:42.349807 IP localhost.49621 > localhost.56039: UDP, length 42
        0x0000:  4500 0046 5358 4000 4011 e94c 7f00 0001  E..FSX@.@..L....
        0x0010:  7f00 0001 c1d5 dae7 0032 fe45 0400 0053  .........2.E...S
        0x0020:  6572 7665 7200 6469 7370 6c61 796e 616d  erver.displaynam
        0x0030:  6520 6861 7320 6a6f 696e 6564 2067 656e  e.has.joined.gen
        0x0040:  6572 616c 2e00                           eral..
21:56:42.349936 IP localhost.56039 > localhost.49621: UDP, length 3
        0x0000:  4500 001f 5359 4000 4011 e972 7f00 0001  E...SY@.@..r....
        0x0010:  7f00 0001 dae7 c1d5 000b fe1e 0000 00    ...............
21:56:42.349937 IP localhost.49621 > localhost.56039: UDP, length 19
        0x0000:  4500 002f 535a 4000 4011 e961 7f00 0001  E../SZ@.@..a....
        0x0010:  7f00 0001 c1d5 dae7 001b fe2e 0100 0101  ................
        0x0020:  0045 6175 7468 2073 7563 6365 7373 00    .Eauth.success.
21:56:42.350128 IP localhost.56039 > localhost.49621: UDP, length 3
        0x0000:  4500 001f 535b 4000 4011 e970 7f00 0001  E...S[@.@..p....
        0x0010:  7f00 0001 dae7 c1d5 000b fe1e 0000 01    ...............
```

You can see that the client sends only the initial AUTH message to port
4657 and everything else is on the dynamic port, like expected.

Note I have this version of `tcpdump`:
```
tcpdump version 4.99.3
libpcap version 1.10.3 (with TPACKET_V3)
```


### TCP stream processing

I made a script `test_stream.py` that connects to the server and sends
separately a number of TCP message fragmens. Some of those are multiple
messages in one and some are incomplete.

```py
test("auth 1 as 1 using 1\r\n")
test("msg from 2 is 2\r\nmsg from 3 is 3\r\nmsg from 4 is 4\r\n")
test("msg from 5 is 5\r\nmsg from 6 is 6\r\nmsg from 7 is")
test(" 7\r\n")
test("msg from")
test(" 8 is 8\r\n")

print("Pres enter to shutdown the socket: ", flush=True, end="")
input()
sock.shutdown(socket.SHUT_RDWR)
```

Note: `test` is a function that calls `input()` an then sends the message.
That way, one can send the messages one-by-one by pressing enter.

Client connected to the server saw this:
```
Server: 1 has joined general.
2: 2
3: 3
4: 4
5: 5
6: 6
7: 7
8: 8
Server: 8 has left general.
```

You can see that all the messages are present and in a correct order. Also,
it is clear that the server remembers the last display name of the client.


### Communication between UDP and TCP clients

I fired up two instances of my client application and the server on a single
machine. Clients were communicating via the loopback interface (IPv4
address `127.0.0.1` or `localhost`).

I run the first client like this:

`./ipk24chat-client -t udp -s localhost`

Here's the input and outut from the terminal:
```
/auth udp udp udp
Server: udp has joined general.
Success: auth success
Server: tcp has joined general.
hello tcp
tcp: hello udp
tcp: let's go to channel kitty
Server: tcp has left general.
/join kitty
Success: join success
Server: udp has joined kitty.
hi
tcp: hello
tcp: ok let's leave, bye
bye
```

I ran the second client in TCP mode like this:
`./ipk24chat-client -t tcp -s localhost`

Here's the input and output:
```
/auth tcp tcp tcp
Success: auth success
Server: tcp has joined general.
udp: hello tcp
hello udp
let's go to channel kitty
/join kitty
Success: join success
Server: tcp has joined kitty.
Server: udp has joined kitty.
udp: hi
hello
ok let's leave, bye
udp: bye
Server: udp has left kitty.
```

Lastly, the server's stdout looked like this:
```log
RECV 127.0.0.1:45601 | AUTH Username=udp Displayname=udp Secret=udp
SENT 127.0.0.1:45601 | MSG
SENT 127.0.0.1:45601 | REPLY
RECV 127.0.0.1:45601 | CONFIRM
RECV 127.0.0.1:45601 | CONFIRM
RECV 127.0.0.1:50622 | AUTH Username=tcp Displayname=tcp Secret=tcp
SENT 127.0.0.1:50622 | REPLY
SENT 127.0.0.1:45601 | MSG
SENT 127.0.0.1:50622 | MSG
RECV 127.0.0.1:45601 | CONFIRM
SENT 127.0.0.1:45601 | CONFIRM
RECV 127.0.0.1:45601 | MSG DisplayName=udp Content=hello tcp
SENT 127.0.0.1:50622 | MSG
RECV 127.0.0.1:50622 | MSG DisplayName=tcp Content=hello udp
SENT 127.0.0.1:45601 | MSG
RECV 127.0.0.1:45601 | CONFIRM
RECV 127.0.0.1:50622 | MSG DisplayName=tcp Content=let's go to channel kitty
SENT 127.0.0.1:45601 | MSG
RECV 127.0.0.1:45601 | CONFIRM
RECV 127.0.0.1:50622 | JOIN DisplayName=tcp ChannelID=kitty
SENT 127.0.0.1:50622 | REPLY
SENT 127.0.0.1:45601 | MSG
SENT 127.0.0.1:50622 | MSG
RECV 127.0.0.1:45601 | CONFIRM
SENT 127.0.0.1:45601 | CONFIRM
RECV 127.0.0.1:45601 | JOIN DisplayName=udp ChannelID=kitty
SENT 127.0.0.1:45601 | REPLY
SENT 127.0.0.1:45601 | MSG
SENT 127.0.0.1:50622 | MSG
RECV 127.0.0.1:45601 | CONFIRM
RECV 127.0.0.1:45601 | CONFIRM
SENT 127.0.0.1:45601 | CONFIRM
RECV 127.0.0.1:45601 | MSG DisplayName=udp Content=hi
SENT 127.0.0.1:50622 | MSG
RECV 127.0.0.1:50622 | MSG DisplayName=tcp Content=hello
SENT 127.0.0.1:45601 | MSG
RECV 127.0.0.1:45601 | CONFIRM
RECV 127.0.0.1:50622 | MSG DisplayName=tcp Content=ok let's leave, bye
SENT 127.0.0.1:45601 | MSG
RECV 127.0.0.1:45601 | CONFIRM
SENT 127.0.0.1:45601 | CONFIRM
RECV 127.0.0.1:45601 | MSG DisplayName=udp Content=bye
SENT 127.0.0.1:50622 | MSG
SENT 127.0.0.1:45601 | CONFIRM
RECV 127.0.0.1:45601 | BYE
SENT 127.0.0.1:50622 | MSG
RECV 127.0.0.1:50622 | BYE
```

In conclusion, everything went up to expectations. You can notice the
server doesn't log the content of the messages it sends. That would be one of
the first future improvements, as it would be very helpful to
anyone reading the log.

## Bibliography

[1] Linux maintainers. epoll manual page [Computer program] December, 2023 [cited 2024-04-22] Available at: https://man7.org/linux/man-pages/man7/epoll.7.html
