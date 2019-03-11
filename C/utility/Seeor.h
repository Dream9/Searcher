#include<exception>
#include<string>

class Serror:std::exception{
public:
    explicit Serror(const std::string &s): std::exception(s){}
}
