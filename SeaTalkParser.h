#ifndef SeaTalkParser_h
#define SeaTalkParser_h

#include "inttypes.h"

/*!
Parses a bytestream into a full SeaTalk message
*/
class SeaTalkParser
{
public:
    SeaTalkParser();
    //! Accepts the next byte in the stream, returns true if a full sentence was received
    bool parse(uint16_t c);
    //! The most recently received complete message. Will be NULL if no full message has been received.
    const uint8_t* message();
    int messageLength();
    void reset();
private:
    uint8_t _message[19];
    int _messageLength;
    int _state;
    int _index;

    int _invalidMessageCount;
    int _messagesParsedCount;
};

#endif
