#ifndef UTIL_H
#define UTIL_H

#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <string>
#include <fstream>
#include <sstream>

#if _WIN32
    #include <direct.h>
#else
    #include <sys/stat.h>
#endif

namespace momi {

#define writelog(str, ...) writelog_(str, __FILE__, __LINE__)


/**
 * @brief get_cpu_core_num
 * @return
 */
static int get_cpu_core_num() {
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

/**
 * @brief isDirExist 判断输出目录是否存在
 * @return
 */
static int is_dir_exist(std::string& dir) {
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

/**
 * @brief writelog
 * @param str
 */
static void writelog_(const char* str, const char* file, int line) {
    printf("[file]:%s\t[line]:%d\t[msg]:%s\n", file, line, str);
}

static void str_append(std::string &str, const int num) {
    std::stringstream ss;
    ss<<num;
    str.append(ss.str());
}

static void str_append(std::string &str, const char* chs) {
    std::stringstream ss;
    ss<<chs;
    str.append(ss.str());
}

static bool file_exist(const std::string& file_path)
{
    if (file_path.empty())
        return false;
    if (access(file_path.c_str(), F_OK) < 0)
        return false;
    return true;
}

template <typename NumberType>
std::string pack(NumberType val)
{
    char* ptr = (char*)(&val);
    return std::string(ptr, sizeof(val));
}

template <typename NumberType>
NumberType unpack(const std::string& val)
{
    const char* ptr = val.c_str();
    NumberType num;
    memcpy(&num, ptr, sizeof(NumberType));
    return num;
}


}
#endif // UTIL_H
