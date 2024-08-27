#include <cstring>
#include "serialization.h"

Serialization::Serialization()
    : serializationType(SKT_UNKNOWN)
{

}

Serialization::Serialization(
    SERIALIZATION_KNOWN_TYPE aSerializationType
)
    : serializationType(aSerializationType)
{

}

Serialization::~Serialization() = default;

const char* Serialization::mimeType() const
{
    return serializationKnownType2MimeType(serializationType);
}

static const char *KNOWN_MIME_TYPES[] {
    "application/octet-stream",
    "application/octet-stream",
    "text/javascript"
};

const char *serializationKnownType2MimeType(
    SERIALIZATION_KNOWN_TYPE value
)
{
    return KNOWN_MIME_TYPES[value];
}

SERIALIZATION_KNOWN_TYPE mimeType2SerializationKnownType(
    const char *value
)
{
    for (auto i = 0; i < 2; i++) {
        if (strcmp(KNOWN_MIME_TYPES[i], value) == 0)
            return (SERIALIZATION_KNOWN_TYPE) i;
    }
    return SKT_UNKNOWN;
}
