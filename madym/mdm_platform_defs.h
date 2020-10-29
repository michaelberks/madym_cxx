/**
 *  @file    mdm_platform_defs.h
 *  @brief   Definitions/includes required for cross-platform builds
 *
 *  Needed so we don't have to include sys/param.h and any other non-windows stuff.
 *	Provides alternative definitions for the code
 *
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

#else
//Original includes on *nix systems - wherever one of the below headers was included include this file instead
#include <unistd.h>
#include <sys/param.h>
#include <ifaddrs.h>
#include <float.h>
#define PLATFORM_USER "USER"
#endif


#endif /* MDM_PLATFORM_DEFS */
