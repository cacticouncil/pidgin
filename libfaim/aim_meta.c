/*
 * Administrative things for libfaim.
 *
 * Changes by EWarmenhoven Wed May 31 00:31:52 UTC 2000
 * - I don't wanna use aim_buildcode! :) I'll put the things that get made
 *   by the .sh file into Makefile.am and hopefully that'll work better.
 *  
 */

#include <faim/aim.h>
/* #include <aim_buildcode.h> generated by mkbuildinfo.sh */

char *aim_getbuilddate(void)
{
  return AIM_BUILDDATE;
}

char *aim_getbuildtime(void)
{
  return AIM_BUILDTIME;
}

char *aim_getbuildstring(void)
{
  static char string[100];

  snprintf(string, 99, "%d.%d.%d-%s%s", 
	   FAIM_VERSION_MAJOR,
	   FAIM_VERSION_MINOR,
	   FAIM_VERSION_MINORMINOR,
	   aim_getbuilddate(),
	   aim_getbuildtime());
  return string;
}

