/*************************************************************************
	> File Name: test_saver.cpp
	> Author: kongxun 
	> Mail: kongxun.yb@gmail.com
	> Created Time: 2017年10月28日 星期六 17时04分27秒
 ************************************************************************/

#include<iostream>
#include "../src/saver.h"

int main() 
{
    momi::SaveQueue q;
    char str[] = "123";
    momi::SaveNode * node = new momi::SaveNode(str,1,1,1);
    q.push(node);
    momi::SaveNode *obj = q.head();
    std::cout<<obj->str()<<obj->pos()<<obj->count()<<obj->timestamp()<<std::endl;
    q.unshift();
    std::cout<<q.empty()<<std::endl;
    return 0;
}
