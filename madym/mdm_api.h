/**
 *  @file    mdm_api.h
 *  @brief   Definitions required to create cross-platform shared libraries
 *
 *	Why? Dynamic linking on Windows is a pain. We need to add __decspec import and export statements
 *	in the right places (compiling the DLL, we need to export symbols so they can be used externally,
 *	compiling an application that sees only the header interface, we need to import the symbols)
 *	On linux this should just work, and it is also not needed if we're building static libraries
 *
 *	However we need to build an exportable shared library if we want to use or libraries in Python,
 *	hence the requirement to add in all this bumf
 *
 */

#ifndef MDM_API_H
#define MDM_API_H

/*MB TODO: For cross platform compatibility, look-up whether I need to wrap everything in the extern C macro*/

#if _WIN32 /*MB TODO: Better to check _MSC_VER?*/

#ifdef MDM_BUILD_SHARED
#  ifdef MDM_API_EXPORTS
#    define MDM_API __declspec(dllexport)
#  else
#    define MDM_API __declspec(dllimport)
#  endif
#else
#  define MDM_API
#endif

#else
#  define MDM_API
#endif


#endif /* MDM_API_H */
