#ifndef UTIL_H_
#define UTIL_H_

#include <sys/time.h>
#include <sys/unistd.h>
#include <time.h>

// 强制inline
#define __must_inline__ __attribute__((always_inline))

// __GLIBC_PREREQ定义在stdlib中
#include <cstdlib>
#if !__GLIBC_PREREQ(2, 3)
#define __builtin_expect(x, expected_value) (x)
#endif
#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

namespace rlog
{
// 辅助类
class Util
{
public:
    static int MakeDir(const char *path);
    static void LocalTime(const time_t *ts, struct tm *tm, int time_zone = 8);
};

} // namespace rlog

#endif // UTIL_H_
