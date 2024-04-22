##################
##  Vit Pavlik  ##
##   xpavli0a   ##
##    251301    ##
##     IPK2     ##
##     2024     ##
##################

#
# Test multiple messages in one recv
# Python version: Python 3.12.2
# OS: Debian 12.5
#
import socket

ADDRESS = "127.0.0.1"
PORT = 4567

def test(s: str) -> None:
    escaped = s.replace("\n", "\\n").replace("\r", "\\r")
    print(f"Press enter to send \"{escaped}\": ", flush=True, end="")
    input()
    sock.send(s.encode("ascii"))
    print("sent")

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((ADDRESS, PORT))
print("connected")

test("auth 1 as 1 using 1\r\n")
test("msg from 2 is 2\r\nmsg from 3 is 3\r\nmsg from 4 is 4\r\n")
test("msg from 5 is 5\r\nmsg from 6 is 6\r\nmsg from 7 is")
test(" 7\r\n")
test("msg from")
test(" 8 is 8\r\n")

print("Pres enter to shutdown the socket: ", flush=True, end="")
input()
sock.shutdown(socket.SHUT_RDWR)
print()
