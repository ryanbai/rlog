# rlog
log组件

很多日志库太庞大，上手慢。这个小组件提供快速开发的能力，支持多线程（免锁）、日志文件大小限制等功能。

##示例编译
g++ --std=c++11 rlog.cpp util.cpp main.cpp -lpthread
