#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>

class Connection
{
public:
    Connection(std::string url, std::string filepath):url(url), filepath(filepath){}

    void set_start_byte(int start_byte) {
        this->start_byte = start_byte;
    }

    void set_conn_size(int conn_size) {
        this->conn_size = conn_size;
    }

    void set_conn_id(int conn_id) {
        this->conn_id = conn_id;
    }

    int get_start_byte() {
        return this->start_byte;
    }

    int get_end_byte() {
        return this->start_byte + this->conn_size -1;
    }

    int get_conn_size() {
        return this->conn_size;
    }

    std::string get_url() {
        return this->url;
    }

    int get_conn_id() {
        return this->conn_id;
    }

    std::string get_file_path() {
        return this->filepath;
    }

private:
    int start_byte=0;
    int conn_size=0;
    std::string url;
    int conn_id=0;
    std::string filepath;
};

#endif // CONNECTION_H
