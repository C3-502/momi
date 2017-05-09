#ifndef MOMI_H
#include <string>
#define MOMI_H


class Momi
{
public:
    Momi(int conn_num, std::string output_path, std::string url);

    Momi(const Momi&);

    void run();

private:
    int conn_num;
    std::string output_path;
    std::string url;
};

#endif // MOMI_H
