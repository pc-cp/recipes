#include "Exception.h"

/*
    cxxabi.h 中最常用的功能是解修饰函数 __cxa_demangle，
    它可以将修饰后的符号名转换为可读的 C++ 名字。
*/
#include <cxxabi.h>
#include <execinfo.h>
#include <stdlib.h>

using namespace muduo;

Exception::Exception(const char* what) : message_(what) {
    const int len = 200;
    void *buffer[len];
    int nptrs = ::backtrace(buffer, len);
    char **strings = ::backtrace_symbols(buffer, nptrs);
    if(strings) {
        for(int i = 0; i < nptrs; ++i) {
            // TODO demangle function name with abi::__cxa_demangle
            stack_.append(strings[i]);
            stack_.push_back('\n');
        }
        free(strings);
    }
}

Exception::~Exception() throw() {

}

const char* Exception::what() const throw() {
    return message_.c_str();
}

const char* Exception::stackTrace() const throw() {
    return stack_.c_str();
}