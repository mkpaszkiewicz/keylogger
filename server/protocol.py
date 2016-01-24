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
    PARSE_ERROR = 102
    ILLEGAL_MSG_TYPE = 103
    UNKNOWN_MSG_TYPE = 104