/**
 *  @file    mdm_utils.c
 *  @brief   Gio's utilities
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

#include "mdm_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>      /* For MAXPATHLEN */

/**
 * @author   Unknown
 * @brief    Wrapper for strcmp
 * @version  n/a
 * @param    str1   First string
 * @param    str2   Second string
 */
int sortstrcmp(char **str1, char **str2)
{
  return (strcmp(*str1, *str2));
}

static char *dirlist[MAXPATHLEN];
static int nentries;

/**
 * @author   Unknown
 * @brief    Wrapper for strcmp
 * @version  n/a
 * @param    str1   First string
 * @param    str2   Second string
 */
char **mdm_makeDirList(char *mask, int *nitems, char *directory)
{
  /* Local variables */
  FILE  *p;
  int    i = 0;
  char   buf[MAXPATHLEN + 17];

/* This directory listing */
sprintf(buf, "/bin/ls %s",directory);
if (mask != NULL) strcat(buf, mask);

/* Get memory for list */
for (i = 0; i < nentries; i++)
{
  cfree(dirlist[i]);                        /* use free() */
  dirlist[i] = NULL;
}

/* Get list and order */
p = popen(buf, "r");
if (p != NULL)
{
  i = 0;
  while (fgets(buf, 255, p) != NULL)
  {
    buf[strlen(buf) - 1] = 0;
    if (strcmp(buf, "."))
    {
      dirlist[i] = (char *) malloc(strlen(buf) + 1);
      strcpy(dirlist[i], buf);
      i++;
    }
  }
  pclose(p);
  nentries = i;
  qsort(dirlist, nentries, sizeof(char *), sortstrcmp);
}

/* Check there is actually something */
*nitems = nentries;
if (nentries == 0)
  return NULL;
else
  return dirlist;
}

/**
 * @author   Legion !!
 * @brief    Print error message and quit gracefully
 * @version  n/a
 * @param    message   String message to display on stderr
 */
void mdm_progAbort(const char *progName, const char *message)
{
  fprintf(stderr, "%s:  %s\n", progName, message);
  exit(EXIT_FAILURE);
}

/**
 * @author   Legion !!
 * @brief    Print error message and quit gracefully
 * @version  n/a
 * @param    message   String message to display on stderr
 */
void mdm_fileAbort(const char *message)
{
  perror(message);
  exit(EXIT_FAILURE);
}

/**
 * @author   Legion !!
 * @brief    Print usage message and quit gracefully if req'd
 * @version  n/a
 * @param    progName   String name of calling program
 * @param    version    String version # of calling program
 * @param    usageMsg   String message to display on stderr
 * @param    exitFlag   Integer flag, quit if non-zero
 */
void mdm_printUsage(const char *progName, const char *version, const char *usageMsg, int exitFlag)
{
  fprintf(stderr, "\n%s version %s\n", progName, version);
  fprintf(stderr, "Usage: %s %s\n\n", progName, usageMsg);
  if (exitFlag)
    exit(exitFlag);
}

