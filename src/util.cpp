#include "util.h"

Util::Util()
{

}

int Util::getCpuCoreNum()
{
#if _WIN32
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
#elif __APPLE__ || linux || __unix__
    return get_nprocs();
#else
    return 4;
#endif
}


