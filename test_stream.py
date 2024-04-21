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

test("auth a as a using a\r\n")
test("msg from a is a\r\nmsg from b is b\r\nmsg from c is c\r\n")
test("msg from a is a\r\nmsg from b is b\r\nmsg from c is")
test(" c\r\n")
test("msg from")
test(" a is a\r\n")

print("Pres enter to shutdown the socket: ", flush=True, end="")
input()
sock.shutdown(socket.SHUT_RDWR)
print()
