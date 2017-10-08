#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>

class Connection
{
public:
    Connection(std::string, std::string);

    void set_start_byte(int start_byte);
    void set_conn_size(int conn_size);
    void set_conn_id(int conn_id);
    int get_start_byte();
    int get_end_byte();
    int get_conn_size();
    int get_conn_id();
    std::string get_url();
    std::string get_file_path();

private:
    int start_byte=0;
    int conn_size=0;
    std::string url;
    int conn_id=0;
    std::string filepath;
};

#endif // CONNECTION_H
