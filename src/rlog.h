#ifndef RLOG_H_
#define RLOG_H_

// 系统文件
#include <errno.h>
#include <pthread.h>
// C++库
#include <stdint.h>
#include <string.h>
// 其他库
// 项目内
#include "util.h"

namespace rlog
{
static const int PATH_MAX_LEN = 256;
//
static const int MODULE_NAME_MAX_LEN = 32;

// 前置声明
class LogContext;
class LogGlobal;

// 全局变量
extern LogGlobal g_rlog_global;
extern __thread LogContext* g_rlog_context;

// 日志级别
enum LogLevel
{
    LOG_LEVEL_TRACE     = 40,
    LOG_LEVEL_DEBUG     = 50,
    LOG_LEVEL_INFO      = 60,
    LOG_LEVEL_WARN      = 70,
    LOG_LEVEL_ERR       = 80,
};

/*
 * @LogGlobal 日志的全局数据区
 */
class LogGlobal
{
public:
    LogGlobal()
        : priority_(LOG_LEVEL_TRACE)
    {
    }
    // 初始化
    void Init(const char* priority_name, const char* path = nullptr,
            const char* module_name = nullptr);
    // reload加载
    void Reload(const char* priority_name);
    // 检查日志级别
    inline bool Check(int priority)
    {
        // 先创建上下文
        if (unlikely(g_rlog_context == nullptr))
        {
            int ret = CreateIfNecessary(g_rlog_context);
            if (0 != ret)
            {
                return false;
            }
        }

        // 判断级别
        return priority >= priority_;
    }
    const char* GetPath() const {return path_;}
    
protected:
    // 创建线程私有变量
    int CreateIfNecessary(LogContext*& context);
    // 将字符串转换为数值
    int ConvertLogPriority(const char* priority_name);

private:
    // 当前的日志级别
    int priority_;
    //
    //
    char path_[PATH_MAX_LEN];
    //
    char module_name_[MODULE_NAME_MAX_LEN];
};

/*
 * @LogContext 日志上下文(线程私有变量)
 */
class LogContext
{
public:
    LogContext()
        : log_file_index_(0)
        , log_fd_(-1)
        , last_check_ms_(0)
        , find_next_file_(false)
    {
    }
    // 打开log
    int OpenLog();
    // 写日志
    void Log(const char *format, ...) __attribute__((format(printf, 2, 3)));
    // 检测文件是否需要更新
    void CheckFile(const struct tm& now);
    // 获取文件名
    void GetLogFileName(char* log_file, size_t len, const struct tm& now);

private:
    //
    char buffer_[1024];
    // 日志文件序号
    int log_file_index_;
    // 日志文件句柄
    int log_fd_;
    // 上次检查时间
    time_t last_check_ms_;
    // 是否需要查找下一个文件
    bool find_next_file_;
};

//------------------------------------------------
void InitGlobalLog(const char* priority = "debug", const char* path = nullptr, const char* module_name = nullptr);

} // namespace rlog

#define THIS_FILE ((strrchr(__FILE__, '/') ?: __FILE__ - 1) + 1)
#define ERR_LOG(format, args...)                                                                           \
    {                                                                                                                   \
        if (likely(rlog::g_rlog_global.Check(rlog::LOG_LEVEL_ERR)))                                                           \
        {                                                                                                               \
            rlog::g_rlog_context->Log("ERR|%s:%d:%s|" format "\n",                                \
                THIS_FILE, __LINE__, __FUNCTION__, ##args);                                                             \
        }                                                                                                               \
    }

#define WARN_LOG(format, args...)                                                                          \
    {                                                                                                                   \
        if (likely(rlog::g_rlog_global.Check(rlog::LOG_LEVEL_WARN)))                                                            \
        {                                                                                                               \
            rlog::g_rlog_context->Log("WARN|%s:%d:%s|" format "\n",                                \
                THIS_FILE, __LINE__, __FUNCTION__, ##args);                                                             \
        }                                                                                                               \
    }

#define INFO_LOG(format, args...)                                                                          \
    {                                                                                                                   \
        if (likely(rlog::g_rlog_global.Check(rlog::LOG_LEVEL_INFO)))                                                            \
        {                                                                                                               \
            rlog::g_rlog_context->Log("INFO|%s:%d:%s|" format "\n",                                       \
                THIS_FILE, __LINE__, __FUNCTION__, ##args);                                                             \
        }                                                                                                               \
    }

#define DEBUG_LOG(format, args...)                                                                                      \
    {                                                                                                                   \
        if (unlikely(rlog::g_rlog_global.Check(rlog::LOG_LEVEL_DEBUG))) {                                                   \
            rlog::g_rlog_context->Log("DEBUG|%s:%d:%s|" format "\n",                                                      \
                THIS_FILE, __LINE__, __FUNCTION__, ##args);                                                             \
        }                                                                                                               \
    }

#define TRACE_LOG(format, args...)                                                                         \
    {                                                                                                                   \
        if (unlikely(rlog::g_rlog_global.Check(rlog::LOG_LEVEL_TRACE))) {                                                       \
            rlog::g_rlog_context->Log("TRACE|%s:%d:%s|" format "\n",                                     \
                THIS_FILE, __LINE__, __FUNCTION__, ##args);                                                             \
        }                                                                                                               \
    }

#endif // RLOG_H_
