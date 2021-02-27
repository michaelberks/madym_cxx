/**
 *  @file    mdm_utils.h
 *  @brief   Gio's utilities - header
 *
 *  Original author GA Buonaccorsi 21 May 2004
 *  (c) Copyright ISBE, University of Manchester 2004
 *
 *  Last edited GAB 21 May 2004
 *  GAB mods:
 *  21 May 2004
 *  - Created with progAbort and fileAbort
 *
 */

#ifndef MDM_UTILS_HDR
#define MDM_UTILS_HDR

#define MDM_YES    1
#define MDM_NO     0

#define MDM_EXIT    1
#define MDM_NOEXIT  0

int sortstrcmp(char **str1, char **str2);
char **mdm_makeDirList(char *mask, int *nitems, char *directory);
void mdm_progAbort(const char *progName, const char *message);
void mdm_fileAbort(const char *message);
void mdm_printUsage(const char *progName, const char *version, const char *usageMsg, int exitFlag);

#endif /* MDM_UTILS_HDR */
