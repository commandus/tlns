#if defined(_MSC_VER) || defined(__MINGW32__)
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include "strptime.h"
#endif

#include "lorawan/lorawan-date.h"

#include <chrono>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iomanip>

#if defined(_MSC_VER) || defined(__MINGW32__)
#include "strptime.h"
#else
#include <sys/time.h>
#include <iostream>

#define TMSIZE sizeof(struct tm)
#define localtime_s(tm, time) memmove(tm, localtime(time), TMSIZE)
#define gmtime_s(tm, time) memmove(tm, gmtime(time), TMSIZE)
#endif

// hours offset
const static char *dateformat_gmtoff = "%FT%T%z";
// time zone name
const static char *dateformat1_tm_zone = "%FT%T%Z";

/**
 * Return formatted time stamp
 * @param value tm structure, local ot GMT
 * @param usec microseconds
 * @param format format template string
 * @return time stamp
 */
static std::string TM2String(
    const struct tm &value,
    int usec,
    const std::string &format
)
{
    char dt[64];
    strftime(dt, sizeof(dt), format.c_str(), &value);
    if (usec < 0)
        return dt;
    std::stringstream ss;
    ss << std::string(dt) << "." << std::setw(6) << std::setfill('0') << usec;
    return ss.str();
}

/**
 * Return formatted time stamp local time
 * @param value seconds
 * @param usec microseconds
 * @param format format template string
 * @return time stamp
 */
std::string ltimeString(
	time_t value,
	int usec,
	const std::string &format
) {
	if (!value)
		value = time(nullptr);
	struct tm tm{};
	localtime_s(&tm, &value);
    return TM2String(tm, usec, format);
}

/**
 * Return formatted time stamp local time
 * @param value seconds
 * @param usec microseconds
 * @param format format template string
 * @return time stamp
 */
std::string gtimeString(
    time_t value,
    int usec,
    const std::string &format
) {
    if (!value)
        value = time(nullptr);
    struct tm tm{};
    gmtime_s(&tm, &value);
    return TM2String(tm, usec, format);
}

static time_t mktimeWithOffset(
    struct tm &tm,
    long int tz
)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    time_t o = 0;
    time_t t = mktime(&tm);
    return t - o - tz;
#else
    long o = tm.tm_gmtoff;
    long t = mktime(&tm);
    return t - o - tz;
#endif
}

/**
 * Unix epoch time (seconds) or 2015-11-25T23:59:11
 * @see https://stackoverflow.com/questions/28991427/why-is-mktime-unsetting-gmtoff-how-to-make-mktime-use-the-gmtoff-field
 */
time_t parseDate(
    const char *v
)
{
    struct tm tmd0 {};
#if defined(_MSC_VER) || defined(__MINGW32__)
    long tz = _timezone;
#else
    long tz = timezone;
#endif
    if (tz == 0) {
        // somehow in Ubuntu
        std::time_t gt = std::time(0);
        auto gm = *std::gmtime(&gt);
        std::time_t lt = std::mktime(&gm);
        tz = (long) (lt - gt);
    }
    bool invFmt = strptime(v, dateformat_gmtoff, &tmd0) == nullptr;
    if (invFmt) {
        invFmt = strptime(v, dateformat1_tm_zone, &tmd0) == nullptr;
        if (invFmt)
            return strtol(v, nullptr, 0);
        else
            return mktimeWithOffset(tmd0, tz);
    } else {
#if !(defined(_MSC_VER) || defined(__MINGW32__))
        // check TZ if 0
        if (tmd0.tm_gmtoff == 0) {
            //
            struct tm tmd1 {};
            strptime(v, dateformat1_tm_zone, &tmd1);
            if (tmd1.tm_gmtoff != 0)
                return mktimeWithOffset(tmd1, tz);
        }
#endif
        return mktimeWithOffset(tmd0, tz);
    }
}

#if defined(_MSC_VER) || defined(__MINGW32__)
/**
 * @see https://stackoverflow.com/questions/10905892/equivalent-of-gettimeofday-for-windows
 * @param tp
 * @param tzp
 * @return
 */
extern "C" int gettimeofday(
    struct timeval *tp,
    struct timezone *tzp
)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime(&system_time, &file_time);
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#endif

static time_t time_ms(unsigned int &ms) {
	struct timeval tp {};
	gettimeofday(&tp, nullptr);
	ms = tp.tv_usec / 1000;
	return tp.tv_sec;
}

// https://github.com/davidcalhoun/gps-time.js/blob/master/gps-time.js

// Difference in time between Jan 1, 1970 (Unix epoch) and Jan 6, 1980 (GPS epoch).
static const uint32_t gpsUnixEpochDiff = 315964800;

static const uint32_t gpsLeap[] = {
	46828800, 78364801, 109900802, 173059203, 252028804, 315187205, 346723206,
	393984007, 425520008, 457056009, 504489610, 551750411, 599184012, 820108813,
	914803214, 1025136015, 1119744016, 1167264017	// last is 2017-Jan-01
};

/**
 * Determines whether a leap second should be added.  Logic works slightly differently
 * for unix->gps and gps->unix.
 * @param {number} gpsMS GPS time in milliseconds.
 * @param {number} curGPSLeapMS Currently leap represented in milliseconds.
 * @param {number} totalLeapsMS Total accumulated leaps in milliseconds.
 * @param {boolean} isUnixToGPS True if this operation is for unix->gps, falsy if gps->unix.
 * @return {boolean} Whether a leap second should be added.
 */
static bool shouldAddLeap (
	uint32_t gps,
	uint32_t curGPSLeap,
	uint32_t totalLeaps,
	bool isUnixToGPS
)
{
	if (isUnixToGPS) {
    	// for unix->gps
    	return gps >= curGPSLeap - totalLeaps;
  } else {
    	// for gps->unix
    	return gps >= curGPSLeap;
  }
}

/**
 * Counts the leaps from the GPS epoch to the inputted GPS time.
 * @param {number} gpsMS GPS time in milliseconds.
 * @param {boolean} isUnixToGPS
 * @return {number}
 */
static uint32_t countLeaps(
	uint32_t gps,
	bool isUnixToGPS
) 
{
	int numLeaps = 0;
	for (int i = 0; i < sizeof(gpsLeap) / sizeof(uint32_t); i++) {
    	if (shouldAddLeap(gps, gpsLeap[i], i, isUnixToGPS)) {
	    	numLeaps++;
    	}
  	}
	return numLeaps;
}

time_t gps2utc(
	 uint32_t value
) {
	// Epoch diff adjustment.
  	uint32_t r = value + gpsUnixEpochDiff;
	// Account for leap seconds between 1980 epoch and gpsMS.
	r -= countLeaps(value, false);
	return r;
}

uint32_t utc2gps(time_t value) {
	// Epoch diff adjustment.
	time_t gps = value - gpsUnixEpochDiff;
	// Account for leap seconds between 1980 epoch and gpsMS.
	gps += countLeaps((uint32_t) gps, true);
	return (uint32_t) gps;
}

std::string timeval2string(
	const struct timeval &val
)
{
	char buf[64];
	const time_t t = val.tv_sec;	// time_t 32/64 bits
	struct tm *tm = localtime(&t);
	strftime(buf, sizeof(buf), dateformat_gmtoff, tm);
	std::stringstream ss;
	ss << buf << "." << std::setw(6) << std::setfill('0') << val.tv_usec;
	return ss.str();
}

std::string time2string(
	time_t val
)
{
	char buf[64];
	struct tm *tm = localtime(&val);
    if (!tm)
        return "0";
	strftime(buf, sizeof(buf), dateformat_gmtoff, tm);
	return buf;
}

void incTimeval(
	struct timeval &val,
	int seconds,
	int usec
)
{
	val.tv_sec += seconds;
	if (usec > 0) {
		val.tv_usec += usec;
		if (val.tv_usec >= 1000000) {
			val.tv_sec++;
			val.tv_usec -= 1000000;
		}
	} else {
		if (usec < 0) {
			if (val.tv_usec < 0) {
				val.tv_sec--;
				val.tv_usec = 1000000 + val.tv_usec;
			}
		}
	}
}

std::string taskTime2string(
    TASK_TIME time,
    const bool local
)
{
    std::time_t t = std::chrono::system_clock::to_time_t(time);
    std::tm *tm;
    if (local)
        tm = std::localtime(&t);
    else
        tm = std::gmtime(&t);
    std::stringstream ss;

    ss << std::put_time(tm, dateformat_gmtoff);

    // add milliseconds
    auto duration = time.time_since_epoch();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration) - std::chrono::duration_cast<std::chrono::seconds>(duration);
    ss << "." << std::setw(3) << std::setfill('0') << ms.count();

    return ss.str();
}

uint32_t tmstAddMS(
    uint32_t value, // in microseconds
    uint32_t addValue
)
{
    return value + (addValue * 1000);
}
