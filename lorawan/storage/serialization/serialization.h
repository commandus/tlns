#ifndef LORAWAN_STORAGE_SERIALIZATION_H
#define LORAWAN_STORAGE_SERIALIZATION_H

typedef enum SerializationKnownType {
    SKT_UNKNOWN = 0,
    SKT_BINARY = 1,
    SKT_TEXT_JSON = 2
} SERIALIZATION_KNOWN_TYPE;

class Serialization {
public:
    SERIALIZATION_KNOWN_TYPE serializationType;
    Serialization();
    Serialization(SERIALIZATION_KNOWN_TYPE serializationType);
    const char* mimeType() const;
    virtual ~Serialization();
};

const char *serializationKnownType2MimeType(SERIALIZATION_KNOWN_TYPE value);
SERIALIZATION_KNOWN_TYPE mimeType2SerializationKnownType(const char *value);

#endif
