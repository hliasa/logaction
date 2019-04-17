#include "log.h"

const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";      /* Red */
const std::string GREEN = "\033[32m";      /* Green */

void Log::print() {
    switch (this->status) {
        default:
        case 0 :
            std::cout<<this->id<<"|"<<this->timestamp<<"|"<<this->msg<<std::endl;
            break;
        case 1:
            std::cout<<this->id<<"|"<<GREEN<<this->timestamp<<RESET<<"|"<<this->msg<<std::endl;
            break;
        case 2:
            std::cout<<this->id<<"|"<<RED<<this->timestamp<<RESET<<"|"<<this->msg<<std::endl;
            break;
    }
}
