#include "NMEAParser.h"
#include <cstring>
#include "NMEAShared.h"
#include <Arduino.h>

typedef enum {
    NMEAParserStateReset = 0,
    NMEAParserStateParsingContent,
    NMEAParserStateParsingChecksum,
    NMEAParserStateComplete
} NMEAParserState;

int hexAsciiToInt(char hexCharacter) {
    hexCharacter = toupper(hexCharacter);
    int output = (hexCharacter >= 'A') ? hexCharacter - 'A' + 10 : hexCharacter - '0';
    return output;
}

NMEAParser::NMEAParser() {
    _index = 0;
    _state = NMEAParserStateReset;
    _invalidChecksumCount = 0;
    _messagesParsedCount = 0;
}

bool NMEAParser::parse(char c) {
    // Sanity check: messages shouldn't be too long
    if (_index >= 100) {
        _state = NMEAParserStateReset;
    }
    // LF and CR always reset parser
    if ((c == 0x0A) || (c == 0x0D)) {
        _state = NMEAParserStateReset;
    }
    // '$' starts an NMEA message, '!' starts a AIVDM message
    if (c == '$' || c == '!') {
        memset(_message, 0, sizeof(_message));
        _message[0] = c;
        _index = 1;
        _state = NMEAParserStateParsingContent;
        return false;
    }
    // Parse other chars according to parser state
    switch(_state) {
        case NMEAParserStateParsingContent:
            // All chars after '$' or '!' and before '*' are the content
            _message[_index] = c;
            _index++;
            if (c == '*') {
                _state = NMEAParserStateParsingChecksum;
                // Ignore the preceding '$' and the trailing '*'
                _contentLength = _index - 2;
            }
            break;
        case NMEAParserStateParsingChecksum:
            // First char following '*' is checksum MSB
            // Second char following '*' is checksum LSB
            _message[_index] = c;
            _index++;
            // Todo: Wikipedia says this: "According to the official specification, the checksum is optional for most data sentences, but is compulsory for RMA, RMB, and RMC (among others)."
            // We only process the message if it has a valid checksum. Not sure if we should be more open.
            if (_index - _contentLength >= 4) {
                int calculatedChecksum = calculateChecksum(&_message[1], _contentLength);
                int actualChecksum = hexAsciiToInt(_message[_index - 2]) * 16;
                actualChecksum += hexAsciiToInt(_message[_index - 1]);

                _message[_index++] = '\r';
                _message[_index++] = '\n';
                _messageLength = _index;
                _message[_index++] = '\0';

                if (actualChecksum != calculatedChecksum) {
                    Serial.println("ERROR: NMEA Checksum Failed");
                    Serial.print("Calculated: ");
                    Serial.print(calculatedChecksum);
                    Serial.print(" Actual: ");
                    Serial.print(actualChecksum);
                    Serial.println(" for message:");
                    Serial.println(_message);
                    _state = NMEAParserStateReset;
                    _invalidChecksumCount++;
                    return false;
                }

                _messagesParsedCount++;
                _state = NMEAParserStateComplete;
                return true;
            }
            break;
        default:
            _state = NMEAParserStateReset;
            break;
    }
    return false;
}

const char *NMEAParser::message() {
    if (_state == NMEAParserStateComplete) {
        return _message;
    } else {
        return NULL;
    }
}

int NMEAParser::messageLength() {
    if (_state == NMEAParserStateComplete) {
        return _messageLength;
    } else {
        return 0;
    }
}
