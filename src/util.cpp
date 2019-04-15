/********************************************************************
FileName : util.cpp
Author   : ryanbai
Date     : 2019-04-10
 ********************************************************************/
#include "util.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace rlog
{
int Util::MakeDir(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
    {
        return 0;
    }

    mode_t mask = umask(0);
    char tmp_path[512];
    snprintf(tmp_path, sizeof(tmp_path), "%s", path);
    char *s     = tmp_path;
    char status = 0;

    while (1)
    {
        char c = 0;
        while (*s)
        {
            if (status == 0 && *s == '/')
            {
                status = 1;
            }

            if (status == 1 && *s != '/')
            {
                status = 0;
                c      = *s;
                *s     = 0;
                break;
            }

            ++s;
        }

        if (mkdir(tmp_path, 0755) < 0)
        {
            if (errno != EEXIST || (stat(tmp_path, &st) < 0 || !S_ISDIR(st.st_mode)))
            {
                umask(mask);
                return -1;
            }
        }

        if (!c)
        {
            umask(mask);
            return 0;
        }

        *s = c;
    }

    return 0;
}

void Util::LocalTime(const time_t *ts, struct tm *tm, int time_zone)
{
    static const int kHoursInDay = 24;
    static const int kMinutesInHour = 60;
    static const int kDaysFromUnixTime = 2472632;
    static const int kDaysFromYear = 153;
    static const int kMagicUnkonwnFirst = 146097;
    static const int kMagicUnkonwnSec = 1461;

    time_t unix_sec = *ts;
    tm->tm_sec  =  unix_sec % kMinutesInHour;
    int i      = (unix_sec/kMinutesInHour);
    tm->tm_min  = i % kMinutesInHour;
    i /= kMinutesInHour;
    tm->tm_hour = (i + time_zone) % kHoursInDay;
    tm->tm_mday = (i + time_zone) / kHoursInDay;
    tm->tm_wday = (tm->tm_mday + 4) % 7;
    int a = tm->tm_mday + kDaysFromUnixTime;
    int b = (a * 4  + 3) / kMagicUnkonwnFirst;
    int c = (-b*kMagicUnkonwnFirst) / 4 + a;
    int d =((c*4 + 3) / kMagicUnkonwnSec);
    int e = -d * kMagicUnkonwnSec;
    e = e / 4 + c;
    int m = (5 * e + 2)/kDaysFromYear;
    tm->tm_mday = -(kDaysFromYear * m + 2) / 5 + e + 1;
    tm->tm_mon = (-m / 10) * 12 + m + 2;
    tm->tm_year = b * 100 + d  - 6700 + (m / 10);
    // 不支持夏令时
    tm->tm_isdst = 0;
}
}

