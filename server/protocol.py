from enum import IntEnum

VERSION = 1


class ClientMsgTypes(IntEnum):
    HELLO = 0
    SEND = 1
    BYE = 2


class ServerMsgTypes(IntEnum):
    OK = 0
    OK_NEW_ID = 1


class Error(IntEnum):
    UNSUPPORTED_PROTOCOL = 100
    INVALID_ID = 101
    DECRYPTION_ERROR = 102
    ILLEGAL_MSG_TYPE = 252
    UNKNOWN_MSG_TYPE = 253
    FATAL_ERROR = 255