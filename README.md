# rlog
服务器log组件

linux上做服务器开发必然会用到日志，但很多日志库太庞大，上手慢。这个小组件提供快速开发的能力，支持多线程（免锁）、日志文件大小限制等功能。

##示例编译
g++ --std=c++11 rlog.cpp util.cpp main.cpp -lpthread
