#ifndef NMEAParser_h
#define NMEAParser_h

/*!
Parses a bytestream into a full NMEA message
*/
class NMEAParser
{
    public:
        NMEAParser();
        //! Accepts the next byte in the stream, returns true if a full sentence was received
        bool parse(char c);
        //! The most recently received complete message. Will be NULL if no full message has been received.
        const char* message();
        int messageLength();
    private:
        char _message[100];
        int _messageLength;
        int _contentLength;
        int _state;
        int _index;

        int _invalidChecksumCount;
        int _messagesParsedCount;
};

#endif
