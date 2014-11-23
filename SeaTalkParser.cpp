#include "SeaTalkParser.h"
#include <cstring>
#include "Arduino.h"


typedef enum {
    SeaTalkParserStateReset = 0,
    SeaTalkParserStateParsingHeader,
    SeaTalkParserStateParsingContent,
    SeaTalkParserStateComplete
} NMEAParserState;


SeaTalkParser::SeaTalkParser() {
    _index = 0;
    _state = SeaTalkParserStateReset;
    _invalidMessageCount = 0;
    _messagesParsedCount = 0;
}

bool SeaTalkParser::parse(uint16_t c) {
    // Sanity check: messages shouldn't be too long
    if (_index > 18) {
        _state = SeaTalkParserStateReset;
    }
    // New messages have the 9th bit set
    if (c & 0x100) {
        _state = SeaTalkParserStateReset;
        memset(_message, 0, sizeof(_message));
        _message[0] = c & 0xFF;
        _messageLength = 0;
        _index = 1;
        _state = SeaTalkParserStateParsingHeader;
        return false;
    }

    // Parse other chars according to parser state
    switch(_state) {
        case SeaTalkParserStateParsingHeader:
            // Least significant nibble of the 2nd byte is the length of the optional section
            if (_index == 1) {
                _messageLength = (c & 0x0F) + 3;
            }
            _message[_index++] = c;
            // The header is always 3 bytes long, the rest of the message is 0-15 bytes
            if (_index > 2) {
                if (_messageLength > 3) {
                    _state = SeaTalkParserStateParsingContent;
                } else {
                    _state = SeaTalkParserStateComplete;
                    return true;
                }
            }
            break;
        case SeaTalkParserStateParsingContent:
            _message[_index++] = c;
            if (_index >= _messageLength) {
                _state = SeaTalkParserStateComplete;
                return true;
            }
            break;
        // Wait for a new header to bump us out of the complete state
        case SeaTalkParserStateComplete:
            break;
        default:
            _state = SeaTalkParserStateReset;
            break;
    }
    return false;
}

const uint8_t *SeaTalkParser::message() {
    if (_state == SeaTalkParserStateComplete) {
        return _message;
    } else {
        return NULL;
    }
}

int SeaTalkParser::messageLength() {
    if (_state == SeaTalkParserStateComplete) {
        return _messageLength;
    } else {
        return 0;
    }
}
