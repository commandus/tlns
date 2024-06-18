#ifndef LOGINTF_H
#define LOGINTF_H

#include <string>

class Log {
public:
    virtual void log(
        int level,
        const std::string &msg
    ) = 0;
};

#endif
