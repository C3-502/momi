#include "connection.h"

Connection::Connection(std::string url, std::string filepath):url(url), filepath(filepath)
{

}

void Connection::set_start_byte(int start_byte) {
    this->start_byte = start_byte;
}

void Connection::set_conn_size(int conn_size) {
    this->conn_size = conn_size;
}

void Connection::set_conn_id(int conn_id) {
    this->conn_id = conn_id;
}

int Connection::get_start_byte() {
    return this->start_byte;
}

int Connection::get_end_byte() {
    return this->start_byte + this->conn_size -1;
}

int Connection::get_conn_size() {
    return this->conn_size;
}

std::string Connection::get_url() {
    return this->url;
}

int Connection::get_conn_id() {
    return this->conn_id;
}

std::string Connection::get_file_path() {
    return this->filepath;
}
