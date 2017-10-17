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
    Util(){}

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
        std::cout<<"[file]:"<<file<<"\t"<<"[line]:"<<line<<"\t"<<"[msg]:"<<str<<std::endl;
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
};

#endif // UTIL_H
