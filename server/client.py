import socket
import struct
import protocol

HOST, PORT = "localhost", 4000
data = b''
data += struct.pack('>B', protocol.ClientMsgTypes.HELLO.value)
data += struct.pack('>I', protocol.VERSION)
data += struct.pack('>B', protocol.ClientMsgTypes.SEND.value)
data += struct.pack('>I', 4)
data += struct.pack('>I', 65)
# data += struct.pack('>B', protocol.ClientMsgTypes.BYE.value)
# Create a socket (SOCK_STREAM means a TCP socket)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    # Connect to server and send data
    sock.connect((HOST, PORT))
    sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.HELLO.value) + struct.pack('>I', protocol.VERSION) + struct.pack('>I', 0xFFFFFFFF))
    received = sock.recv(1024)
    print(received)
    sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.SEND.value) + struct.pack('>I', 4) + struct.pack('>I', 65))
    received = sock.recv(1024)
    print(received)
    sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.BYE.value))

    #print(struct.unpack('>B', data)[0])
    # Receive data from the server and shut down
    #received = sock.recv(1024)
finally:
    sock.close()

print('OK')