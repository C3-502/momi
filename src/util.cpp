#include "util.h"

Util::Util()
{

}

int Util::get_cpu_core_num()
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

int Util::is_dir_exist(std::string& dir)
{
#if _WIN32
    struct _stat fileStat;
    if ((_stat(dir.c_str(), &fileStat) == 0) && (fileStat.st_mode & _S_IFDIR))
    {
        return 0;
#else
    struct stat fileStat;
    if ((stat(dir.c_str(), &fileStat) == 0) && S_ISDIR(fileStat.st_mode))
    {
        return 0;
#endif
    }else{
        return -1;
    }
}

void Util::writelog_(const char* str, const char* file, int line)
{
    std::cout<<"[file]:"<<file<<"\t"<<"[line]:"<<line<<"\t"<<"[msg]:"<<str<<std::endl;
}


void Util::str_append(std::string &str, const int num) {
    std::stringstream ss;
    ss<<num;
    str.append(ss.str());
}

void Util::str_append(std::string &str, const char* chs) {
    std::stringstream ss;
    ss<<chs;
    str.append(ss.str());
}
