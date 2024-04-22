# IPK24CHAT server

This is a server application for the IPK24CHAT protocol. It is my solution
for the second project for the **IPK** course. This assignment variant has
codename **IOTA**.

Author: Vit Pavlik (`xpavli0a`)
Date: April 22nd, 2024

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
- `-p PORT` - the port to listen on
- `-d TIMEOUT` - UDP Retransmission timeout in miliseconds
- `-r RETRIES` - number of UDP retransmissions
- `-h` - print help and exit







```log
RECV 127.0.0.1:44122 | AUTH Username=udp Displayname=udp Secret=udp
SENT 127.0.0.1:44122 | MSG
SENT 127.0.0.1:44122 | REPLY
RECV 127.0.0.1:44122 | CONFIRM
RECV 127.0.0.1:44122 | CONFIRM
RECV 127.0.0.1:54372 | AUTH Username=tcp Displayname=tcp Secret=tcp
SENT 127.0.0.1:54372 | REPLY
SENT 127.0.0.1:44122 | MSG
SENT 127.0.0.1:54372 | MSG
RECV 127.0.0.1:44122 | CONFIRM
SENT 127.0.0.1:44122 | CONFIRM
RECV 127.0.0.1:44122 | MSG DisplayName=udp Content=ahoj tcp
SENT 127.0.0.1:54372 | MSG
RECV 127.0.0.1:54372 | MSG DisplayName=tcp Content=ahoj udp
SENT 127.0.0.1:44122 | MSG
RECV 127.0.0.1:44122 | CONFIRM
SENT 127.0.0.1:44122 | CONFIRM
RECV 127.0.0.1:44122 | MSG DisplayName=udp Content=ja mizim cau
SENT 127.0.0.1:54372 | MSG
SENT 127.0.0.1:44122 | CONFIRM
RECV 127.0.0.1:44122 | BYE
SENT 127.0.0.1:54372 | MSG
RECV 127.0.0.1:54372 | BYE
```
