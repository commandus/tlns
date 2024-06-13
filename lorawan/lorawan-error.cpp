#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "lorawan-error.h"

#define LOG_LEVEL_COUNT	8

static const char *logLevelList[LOG_LEVEL_COUNT] = {
	"",
	"F", // "fatal",
	"C", // "critical",
	"E", // "error",
	"W", // "warning",
	"I", // "info",
	"I", // "info",
	"D", // "debug"
};

static const char *logLevelColorList[LOG_LEVEL_COUNT] = {
	"",
	"0;41",  // background red
	"0;41",  // background red
	"0;31",  // red
	"0;33 ", // yellow
	"0;37",  // white
	"0;37",  // bright white
	"0;32"   // green
};

const char *logLevelString(
	int logLevel
)
{
	if (logLevel < 0)
		logLevel = 0;
	if (logLevel >= LOG_LEVEL_COUNT)
		logLevel = 0;
	return logLevelList[logLevel];
}

const char *logLevelColor(
	int logLevel
)
{
	if (logLevel < 0)
		logLevel = 0;
	if (logLevel >= LOG_LEVEL_COUNT)
		logLevel = 0;
	return logLevelColorList[logLevel];
}
