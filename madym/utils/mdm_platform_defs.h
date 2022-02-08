/**
 *  @file    mdm_platform_defs.h
 *  @brief   Definitions/includes required for cross-platform builds
 *
 *  Needed so we don't have to include sys/param.h and any other non-windows stuff.
 *	Provides alternative definitions for the code
 *
 *  See https://www.fluentcpp.com/2019/08/30/how-to-disable-a-warning-in-cpp/ for platform
 *  independent warning suppression
 */

#ifndef MDM_PLATFORM_DEFS
#define MDM_PLATFORM_DEFS

#if _WIN32 /*MB TODO: Better to check _MSC_VER?*/
//Alternatives needed for windows
#include <direct.h>
#include <Winsock2.h>
#include <limits.h>

#define getcwd _getcwd // stupid MSFT "deprecation" warning
#define PLATFORM_USER "USERNAME"

#define DISABLE_WARNING_PUSH           __pragma(warning( push ))
#define DISABLE_WARNING_POP            __pragma(warning( pop )) 
#define DISABLE_WARNING(warningNumber) __pragma(warning( disable : warningNumber ))

#define DISABLE_WARNING_UNKNOWN_ESCAPE_SEQUENCE    DISABLE_WARNING(4129)
#define DISABLE_WARNING_DEPRECATED    DISABLE_WARNING(4996)

#elif defined(__GNUC__) || defined(__clang__)

//Original includes on *nix systems - wherever one of the below headers was included include this file instead
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <ifaddrs.h>
#include <float.h>
#define PLATFORM_USER "USER"

#define DO_PRAGMA(X) _Pragma(#X)
#define DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
#define DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop) 
#define DISABLE_WARNING(warningName)   DO_PRAGMA(GCC diagnostic ignored #warningName)

#define DISABLE_WARNING_UNKNOWN_ESCAPE_SEQUENCE    DISABLE_WARNING(-Wunknown-escape-sequence)
#define DISABLE_WARNING_DEPRECATED //Do nothing, GCC and Clang are fine
#else

#define DISABLE_WARNING_PUSH
#define DISABLE_WARNING_POP
#define DISABLE_WARNING_UNKNOWN_ESCAPE_SEQUENCE
#endif

#endif /* MDM_PLATFORM_DEFS */
