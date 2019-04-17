#ifndef LOG_H
#define LOG_H

#endif // LOG_H

#include <iostream>


class Log {
public:
    int id;
    std::string msg;
    std::string timestamp;
    int status;
    void print();
};
