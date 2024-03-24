#ifndef LOGINTF_H
#define LOGINTF_H

#include <string>

class Log {
public:
    virtual std::ostream& strm(int level) = 0;
    virtual void flush() = 0;
};

#endif
