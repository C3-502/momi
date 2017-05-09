#include <iostream>
#include <unistd.h>
#include <string>

#include "util.h"
#include "momi.h"

int main(int argc, char *argv[])
{
    int conn_num=Util::getCpuCoreNum();
    std::string output_path="./";
    std::string url="";
    int ch;

    while((ch = getopt(argc,argv,"n::o::")) != -1)
    {
        switch(ch)
        {
        case 'n':
            conn_num = atoi(argv[optind]);
            break;

        case 'o':
            output_path = std::string(argv[optind]);
            break;
        }
    }

    url = std::string(argv[optind]);

    std::cout<<"-n = "<<conn_num<<std::endl;
    std::cout<<"-o = "<<output_path<<std::endl;
    std::cout<<"url = "<<url<<std::endl;

    Momi *momi = new Momi(conn_num, output_path, url);
    momi->run();
    return 0;
}
