#ifndef UTIL_H
#define UTIL_H

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

#define writelog(str, ...) Util::writelog_(str, __FILE__, __LINE__)

class Util
{
public:
    Util();

    static int get_cpu_core_num();

    /**
     * @brief isDirExist 判断输出目录是否存在
     * @return
     */
    static int is_dir_exist(std::string& dir);

    /**
     * @brief writelog
     * @param str
     */
    static void writelog_(const char* str, const char* file, int line);

    static void str_append(std::string &str, const int num);

    static void str_append(std::string &str, const char* chs);
};

#endif // UTIL_H
