/*************************************************************************
	> File Name: momi_exception.h
	> Author: kongxun 
	> Mail: kongxun.yb@gmail.com
	> Created Time: 2017年10月08日 星期日 12时41分29秒
 ************************************************************************/
#include <stdexcept>
#include <string>
#include <iostream>
#ifndef _MOMI_EXCEPTION_H
#define _MOMI_EXCEPTION_H
class Momi_Exception : public std::logic_error
{
public:
    explicit Momi_Exception(const char* msg) : std::logic_error(std::string(msg))
    {
    }
};
#endif
