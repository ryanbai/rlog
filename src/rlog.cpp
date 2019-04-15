#include "rlog.h"
#include "util.h"
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

namespace rlog
{
// 全局变量
LogGlobal g_rlog_global;
// 每个线程拥有一个上下文
__thread LogContext *g_rlog_context = nullptr;
// 线程私有变量key
static pthread_key_t rlog_thread_key;

void LogGlobal::Init(const char* priority_name, const char* path, const char* module_name)
{
    // 设置优先级
    priority_ = ConvertLogPriority(priority_name);

    if (nullptr != module_name)
    {
        snprintf(module_name_, sizeof(module_name_), "%s", module_name);
    }

    // 日志路径，默认为当前路径
    snprintf(path_, sizeof(path_), "%s/%s/", path == nullptr ? "." : path, module_name_);

    // 创建目录
    Util::MakeDir(path_);
}

void LogGlobal::Reload(const char* priority_name)
{
    priority_ = ConvertLogPriority(priority_name);
}

// 在子线程中执行
int LogGlobal::CreateIfNecessary(LogContext *&context)
{
    context = new LogContext();
    if (0 != context->OpenLog())
    {
        delete context;
        context = nullptr;
        return -1;
    }

    pthread_setspecific(rlog_thread_key, context);
    return 0;
}

int LogGlobal::ConvertLogPriority(const char* priority_name)
{
    if (strcasecmp("TRACE", priority_name) == 0)
    {
        return LOG_LEVEL_TRACE;
    }
    else if (strcasecmp("DEBUG", priority_name) == 0)
    {
        return LOG_LEVEL_DEBUG;
    }
    else if (strcasecmp("INFO", priority_name) == 0)
    {
        return LOG_LEVEL_INFO;
    }
    else if (strcasecmp("WARN", priority_name) == 0)
    {
        return LOG_LEVEL_WARN;
    }
    else if (strcasecmp("ERROR", priority_name) == 0)
    {
        return LOG_LEVEL_ERR;
    }
    else
    {
        return -1;
    }
}

const static int MAX_LOG_FILE_SIZE = 1024000000;

// -----------------------------------------------------------------
int LogContext::OpenLog()
{
    last_check_ms_ = 0;

    // 找到合适的日志文件
    log_file_index_ = 0;
    time_t now_sec = time(nullptr);
    struct tm now;
    Util::LocalTime(&now_sec, &now);
    while(true)
    {
        char log_file[PATH_MAX_LEN];
        GetLogFileName(log_file, sizeof(log_file), now);
        struct stat buf;
        int fd = open(log_file, O_RDONLY);
        if (fd >= 0 && fstat(fd, &buf) == 0 && buf.st_size >= MAX_LOG_FILE_SIZE)
        {
            ++log_file_index_;
            continue;
        }

        break;
    }

    return 0;
}

void LogContext::Log(const char *format, ...)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    struct tm now;
    Util::LocalTime(&t.tv_sec, &now);
    time_t ms = t.tv_sec * 1000 + t.tv_usec / 1000;
    // 200ms check 一次
    if (log_fd_ < 0 || ms - last_check_ms_ >= 200)
    {
        last_check_ms_  = ms;
        CheckFile(now);
    }
    // 文件打不开，不写日志
    if (log_fd_ < 0)
    {
        return;
    }
    static pid_t pid = getpid();
    int header_len = snprintf(buffer_, sizeof(buffer_), "%04d%02d%02d %02d:%02d:%02d.%06ld|%d|",
            1900 + now.tm_year, 1 + now.tm_mon, now.tm_mday,
            now.tm_hour, now.tm_min, now.tm_sec, t.tv_usec,
            pid);
    va_list argptr;
    va_start(argptr, format);
    int log_len = vsnprintf(&buffer_[header_len], sizeof(buffer_) - header_len, format, argptr);
    va_end(argptr);
    if (log_len < 0)
    {
        return;
    }
    buffer_[header_len + log_len] = 0;
    write(log_fd_, buffer_, header_len + log_len);
}

void LogContext::CheckFile(const struct tm& now)
{
    if (log_fd_ >= 0)
    {
        // 旧的日志文件符合要求
        struct stat buf;
        if (fstat(log_fd_, &buf) == 0 && buf.st_size < MAX_LOG_FILE_SIZE)
        {
            return;
        }
    }

    // 尝试下一个序号的文件
    if (find_next_file_)
    {
        ++log_file_index_;
    }
    find_next_file_ = true;

    char log_file[PATH_MAX_LEN];
    GetLogFileName(log_file, sizeof(log_file), now);
    mode_t mask = umask(0);
    log_fd_ = open(log_file, O_APPEND | O_CREAT | O_RDWR, 0644);
    umask(mask);
}

void LogContext::GetLogFileName(char* log_file, size_t len, const struct tm& now)
{
    if (log_file_index_ == 0)
    {
        snprintf(log_file, len, "%s/%04d%02d%02d%02d.log",
                g_rlog_global.GetPath(), 1900 + now.tm_year, 1 + now.tm_mon,
                now.tm_mday, now.tm_hour);
    }
    else
    {
        snprintf(log_file, len, "%s/%04d%02d%02d%02d.log.%u",
                g_rlog_global.GetPath(), 1900 + now.tm_year, 1 + now.tm_mon,
                now.tm_mday, now.tm_hour, log_file_index_);
    }
}

// ---------------------------------全局函数------------------------------------------
void DestroyThreadLog(void *ptr)
{
    if (NULL == ptr)
    {
        return;
    }

    LogContext *context = reinterpret_cast<LogContext *>(ptr);
    delete context;
    g_rlog_context = NULL;
}

void InitGlobalLog(const char* priority, const char* path, const char* module_name)
{
    pthread_key_create(&rlog_thread_key, DestroyThreadLog);
    g_rlog_global.Init(priority, path, module_name);
}

}  // namespace rlog
