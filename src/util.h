#ifndef UTIL_H
#include <unistd.h>
#include <sys/sysinfo.h>
#define UTIL_H


class Util
{
public:
    Util();

    static int getCpuCoreNum();
};

#endif // UTIL_H
