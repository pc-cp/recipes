#ifndef MUDUO_BASE_EXCEPTION_H
#define MUDUO_BASE_EXCEPTION_H

#include <exception>
#include <string>

namespace muduo {
    class Exception : public std::exception {
        public:
            explicit Exception(const char* what);
            /*
                	throw();：告诉编译器和开发者，这个函数不会抛出任何异常。
                    如果该函数在实际执行过程中抛出了异常，则会调用std::unexpected()，通常会导致程序终止。

                    // C++98语法，C++11已经用noexcept关键字替换
            */
            virtual ~Exception() throw();
            virtual const char* what() const throw();
            const char* stackTrace() const throw();

        private:
            std::string     message_;
            std::string     stack_;
    };
}

#endif // MUDUO_BASE_EXCEPTION_H
