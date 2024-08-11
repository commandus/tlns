#include <iostream>
#include "subst-call-c.h"
#include "log-intf.h"
#include "libloragw-helper.h"

class SLog: public Log {
public:
    void log(
        int level,
        const std::string &msg
    ) override {
        std::cout << msg << std::endl;
    }
};

int main(int argc, char **argv) {
    LibLoragwHelper llh;
    llh.bind(new SLog, nullptr);
    printf_c("%.2f\n", 10.02);
    printf_c("%02d\n", 10);
    return 0;
}
