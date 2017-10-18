#include <iostream>
#include <unistd.h>
#include <string>
#include <sstream>
#include "./include/args.hxx"

#include "define.h"
#include "util.h"
#include "exception.h"
#include "momi.h"

int main(int argc, char *argv[])
{
    args::ArgumentParser parser("momi: cross-platform download accelerator", "");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<int> conn_num(parser, "conn_num", "number of the connections", {'n'});
    args::ValueFlag<std::string> output_path(parser, "output_path", "output path to save the file", {'o'});
    args::Positional<std::string> url(parser, "url", "the download url");

    try
    {
        parser.ParseCLI(argc, argv);

        if (conn_num) {
            std::stringstream ss;
            ss<<args::get(conn_num);
            std::string str = "-n = ";
            str.append(ss.str());
            momi::writelog(str.c_str());
        }
        if (output_path) {
            std::stringstream ss;
            ss<<args::get(output_path);
            std::string str = "-o = ";
            str.append(ss.str());
            momi::writelog(str.c_str());
        }
        if (url) {
            std::stringstream ss;
            ss<<args::get(url);
            std::string str = "url = ";
            str.append(ss.str());
            momi::writelog(str.c_str());
        }

        //修正参数信息
        int conn_num_ = args::get(conn_num);
        std::string url_ = args::get(url);
        std::string output_path_ = args::get(output_path);
        int protocol_ = Define::P_HTTP;

        //从url中提取文件名
        int pos = url_.find_last_of("/");
        std::string filename_ = url_.substr(pos+1, -1);

        pos = output_path_.find_last_of("/");
        if(pos+1 < output_path_.size()){
            //若指定了输出文件名，修正
            filename_ = output_path_.substr(pos+1, -1);
            output_path_ = output_path_.substr(0, pos+1);
        }

        //从url提取协议类型，待优化
        pos = url_.find_first_of(":");
        if(pos<url_.size()){
            std::string protocol_str = url_.substr(0, pos);
            if(protocol_str == "http"){
                protocol_ = momi::PROTOCOLS::P_HTTP;
            }else if(protocol_str == "https"){
                protocol_ = momi::PROTOCOLS::P_HTTPS;
            }else{
                throw momi::Exception("协议暂不支持");
            }
        }

        momi::Momi *obj = new momi::Momi(conn_num_, output_path_, filename_, url_, protocol_);
        obj->init();
        obj->run();
        //obj->pause();
        //obj->resume();
        return 0;

    }
    catch (args::Help)
    {
        std::cout << parser;
        return 0;
    }
    catch (args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    catch (args::ValidationError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    catch (const momi::Exception &e) {
        std::cerr << e.what() << std::endl;
    }
}
