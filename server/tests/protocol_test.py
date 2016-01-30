#!/usr/bin/env python
import unittest
import threading
import socket
import struct
import os
import sys

sys.path.insert(0, os.path.pardir)

import server
import protocol


class ProtocolTest(unittest.TestCase):
    """
    Class tests whether server properly responses for given request or message.
    """
    HOST, PORT = 'localhost', 4000
    config_file = 'test_config.ini'
    log_file = 'test_server.log'

    @classmethod
    def setUpClass(cls):
        """
        Creates configuration file, overrides existing one.
        Starts a test server on given address.
        Logs are printed only to file.
        """
        with open(ProtocolTest.config_file, 'w+') as file:
            file.seek(0)
            file.write('[Client ID]\n')
            file.write('id = 2\n')

        server.Server.allow_reuse_address = True
        cls.test_server = server.Server(ProtocolTest.HOST, ProtocolTest.PORT, server.TCPHandler, config_file=ProtocolTest.config_file,
                                        log_file=ProtocolTest.log_file, console=False)

        server_thread = threading.Thread(target=cls.test_server.serve_forever)
        # Exit the server thread when the main thread terminates
        server_thread.daemon = True
        server_thread.start()

    @classmethod
    def tearDownClass(cls):
        """
        shutdown the test server and tidy up.
        """
        cls.test_server.shutdown()
        cls.test_server.server_close()
        os.remove(ProtocolTest.config_file)

    def setUp(self):
        """
        Called before each test, creates and configure socket.
        """
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((ProtocolTest.HOST, ProtocolTest.PORT))

    def tearDown(self):
        """
        Called after each test, closes socket.
        """
        self.sock.close()

    def test_hello_bye(self):
        """
        Connects to the server and sends two simple messages - HELLO and BYE.
        Checks whether server responses are the same as expected.
        """
        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.HELLO.value) + struct.pack('>I', protocol.VERSION) + struct.pack('>I', 1))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.ServerMsgTypes.OK, 'Improper message type')

        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.BYE.value))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 0, 'Server still communicates after BYE message')

    def test_illegal_msg1(self):
        """
        Connects to the server and sends illegal message.
        Checks whether server return expected error.
        """
        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.SEND.value))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.Error.ILLEGAL_MSG_TYPE, 'Server responses with improper error')

    def test_illegal_msg2(self):
        """
        Connects to the server and sends illegal message.
        Checks whether server return expected error.
        """
        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.BYE.value))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.Error.ILLEGAL_MSG_TYPE, 'Server responses with improper error')

    def test_unknown_msg(self):
        """
        Connects to the server and sends message with unknown type.
        Checks whether server return expected error.
        """
        self.sock.sendall(struct.pack('>B', 0xFF))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.Error.UNKNOWN_MSG_TYPE, 'Server responses with improper error')

    def test_connection_setup(self):
        """
        Connects to the server and tries to connect with wrong client or protocol version id.
        Checks whether server return expected error.
        """
        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.HELLO.value) + struct.pack('>I', protocol.VERSION + 4) + struct.pack('>I', 2))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.Error.UNSUPPORTED_PROTOCOL, 'Server responses with improper error')

        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.HELLO.value) + struct.pack('>I', protocol.VERSION) + struct.pack('>I', 3))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.Error.INVALID_ID, 'Server responses with improper error')

        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.HELLO.value) + struct.pack('>I', protocol.VERSION + 5) + struct.pack('>I', 1))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.Error.UNSUPPORTED_PROTOCOL, 'Server responses with improper error')

        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.HELLO.value) + struct.pack('>I', protocol.VERSION) + struct.pack('>I', 1))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.ServerMsgTypes.OK, 'Everything is ok, but server says something different')

        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.BYE.value))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 0, 'Server still communicates after BYE message')

    def test_register_new_client1(self):
        """
        Connects to the server and tries register as a new client.
        Checks whether server attach to the client proper id.
        """
        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.HELLO.value) + struct.pack('>I', protocol.VERSION) + struct.pack('>I', 2))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.Error.INVALID_ID, 'Server responses with improper error')

        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.HELLO.value) + struct.pack('>I', protocol.VERSION) + struct.pack('>I', 0xFFFFFFFF))
        received = self.sock.recv(1024)
        self.assertEqual(int(received[0]), protocol.ServerMsgTypes.OK_NEW_ID, 'Server responses with improper message')
        self.assertEqual(struct.unpack('>I', received[1:5])[0], 2, 'Server attach to a new client improper id')

        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.BYE.value))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 0, 'Server still communicates after BYE message')

    def test_register_new_client2(self):
        """
        Connects to the server and tries register as a new client, then sends improper message.
        Checks whether server attach to the client proper id and then return proper error.
        """
        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.HELLO.value) + struct.pack('>I', protocol.VERSION) + struct.pack('>I', 0xFFFFFFFF))
        received = self.sock.recv(1024)
        self.assertEqual(int(received[0]), protocol.ServerMsgTypes.OK_NEW_ID, 'Server responses with improper message')
        self.assertEqual(struct.unpack('>I', received[1:5])[0], 3, 'Server attach to a new client improper id')

        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.HELLO.value) + struct.pack('>I', protocol.VERSION) + struct.pack('>I', 0xFFFFFFFF))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.Error.ILLEGAL_MSG_TYPE, 'Server responses with improper error')

    def test_correct_communication(self):
        """
        Connects to the server and tries register as a new client, then sends improper message.
        Checks whether server attach to the client proper id and then return proper error.
        """
        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.HELLO.value) + struct.pack('>I', protocol.VERSION) + struct.pack('>I', 1))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.ServerMsgTypes.OK, 'Server responses with improper message')

        # send 'down a, down a'
        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.SEND.value) + struct.pack('>I', 4) + struct.pack('>I', 0x011E011E))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.ServerMsgTypes.OK, 'Server responses with improper message')

        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.BYE.value))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 0, 'Server still communicates after BYE message')

    def test_incorrect_communication(self):
        """
        Connects to the server and tries register as a new client, then sends improper message.
        Checks whether server attach to the client proper id and then return proper error.
        """
        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.HELLO.value) + struct.pack('>I', protocol.VERSION) + struct.pack('>I', 1))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.ServerMsgTypes.OK, 'Server responses with improper message')

        # send 'down a, down a'
        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.SEND.value) + struct.pack('>I', 4) + struct.pack('>I', 0x011E011E))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.ServerMsgTypes.OK, 'Server responses with improper message')

        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.HELLO.value) + struct.pack('>I', protocol.VERSION) + struct.pack('>I', 1))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.Error.ILLEGAL_MSG_TYPE, 'Server responses with improper error')

    def test_data_parser(self):
        """
        Connects to the server and tries register as a new client, then sends improper message.
        Checks whether server attach to the client proper id and then return proper error.
        """
        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.HELLO.value) + struct.pack('>I', protocol.VERSION) + struct.pack('>I', 1))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.ServerMsgTypes.OK, 'Server responses with improper message')

        # send '??? a, down a'
        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.SEND.value) + struct.pack('>I', 4) + struct.pack('>I', 0x041E011E))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.Error.PARSE_ERROR, 'Server responses with improper error')

        # send 'down s, up a'
        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.SEND.value) + struct.pack('>I', 4) + struct.pack('>I', 0x011F001E))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.ServerMsgTypes.OK, 'Server responses with improper message')

        # send 'down ??, down a'
        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.SEND.value) + struct.pack('>I', 4) + struct.pack('>I', 0x0180011E))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 1, 'Improper message length')
        self.assertEqual(int(received[0]), protocol.Error.PARSE_ERROR, 'Server responses with improper error')

        self.sock.sendall(struct.pack('>B', protocol.ClientMsgTypes.BYE.value))
        received = self.sock.recv(1024)
        self.assertEqual(len(received), 0, 'Server still communicates after BYE message')

if __name__ == '__main__':
    unittest.main()