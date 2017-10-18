/*************************************************************************
	> File Name: momi_exception.h
	> Author: kongxun 
	> Mail: kongxun.yb@gmail.com
	> Created Time: 2017年10月08日 星期日 12时41分29秒
 ************************************************************************/
#include <stdexcept>
#include <string>
#include <iostream>
#ifndef _EXCEPTION_H
#define _EXCEPTION_H
namespace momi {

class Exception : public std::logic_error
{
public:
    explicit Exception(const char* msg) : std::logic_error(std::string(msg))
    {
    }
};

}
#endif
