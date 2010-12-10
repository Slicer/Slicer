/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "vtkArchive.h"

// LibArchive includes
#include <archive.h>
#include <archive_entry.h>

// Std includes
#include <cstdio>
#include <cstring>
#include <ctime>

#define BSDTAR_FILESIZE_PRINTF  "%lu"
#define BSDTAR_FILESIZE_TYPE    unsigned long

void list_item_verbose(FILE *out, struct archive_entry *entry)
{
  char                   tmp[100];
  size_t                         w;
  const char            *p;
  const char            *fmt;
  time_t                         tim;
  static time_t          now;
  size_t u_width = 6;
  size_t gs_width = 13;
  
  /*
   * We avoid collecting the entire list in memory at once by
   * listing things as we see them.  However, that also means we can't
   * just pre-compute the field widths.  Instead, we start with guesses
   * and just widen them as necessary.  These numbers are completely
   * arbitrary.
   */
  if (!now)
    {
    time(&now);
    }
  fprintf(out, "%s %d ",
          archive_entry_strmode(entry),
          archive_entry_nlink(entry));
  
  /* Use uname if it's present, else uid. */
  p = archive_entry_uname(entry);
  if ((p == NULL) || (*p == '\0'))
    {
    sprintf(tmp, "%lu ",
            (unsigned long)archive_entry_uid(entry));
    p = tmp;
    }
  w = strlen(p);
  if (w > u_width)
    {
    u_width = w;
    }
  fprintf(out, "%-*s ", (int)u_width, p);
  /* Use gname if it's present, else gid. */
  p = archive_entry_gname(entry);
  if (p != NULL && p[0] != '\0')
    {
    fprintf(out, "%s", p);
    w = strlen(p);
    } 
  else
    {
    sprintf(tmp, "%lu",
            (unsigned long)archive_entry_gid(entry));
    w = strlen(tmp);
    fprintf(out, "%s", tmp);
    }
  
  /*
   * Print device number or file size, right-aligned so as to make
   * total width of group and devnum/filesize fields be gs_width.
   * If gs_width is too small, grow it.
   */
  if (archive_entry_filetype(entry) == AE_IFCHR
      || archive_entry_filetype(entry) == AE_IFBLK) 
    {
    sprintf(tmp, "%lu,%lu",
            (unsigned long)archive_entry_rdevmajor(entry),
            (unsigned long)archive_entry_rdevminor(entry));
    }
  else 
    {
    /*
     * Note the use of platform-dependent macros to format
     * the filesize here.  We need the format string and the
     * corresponding type for the cast.
     */
    sprintf(tmp, BSDTAR_FILESIZE_PRINTF,
            (BSDTAR_FILESIZE_TYPE)archive_entry_size(entry));
    }
  if (w + strlen(tmp) >= gs_width)
    {
    gs_width = w+strlen(tmp)+1;
    }
  fprintf(out, "%*s", (int)(gs_width - w), tmp);

  /* Format the time using 'ls -l' conventions. */
  tim = archive_entry_mtime(entry);
#define HALF_YEAR (time_t)365 * 86400 / 2
#if defined(_WIN32) && !defined(__CYGWIN__)
  /* Windows' strftime function does not support %e format. */
#define DAY_FMT  "%d"  
#else
#define DAY_FMT  "%e"  /* Day number without leading zeros */
#endif
  if (tim < now - HALF_YEAR || tim > now + HALF_YEAR)
    {
    fmt = DAY_FMT " %b  %Y";
    }
  else
    {
    fmt = DAY_FMT " %b %H:%M";
    }
  strftime(tmp, sizeof(tmp), fmt, localtime(&tim));
  fprintf(out, " %s ", tmp);
  fprintf(out, "%s", archive_entry_pathname(entry));

  /* Extra information for links. */
  if (archive_entry_hardlink(entry)) /* Hard link */
    {
    fprintf(out, " link to %s",
            archive_entry_hardlink(entry));
    }
  else if (archive_entry_symlink(entry)) /* Symbolic link */
    {
    fprintf(out, " -> %s", archive_entry_symlink(entry));
    }
}
#ifdef __BORLANDC__
# pragma warn -8066 /* unreachable code */
#endif
  
long copy_data(struct archive *ar, struct archive *aw)
{
  long r;
  const void *buff;
  size_t size;
  off_t offset;
  
  for (;;) 
    {
    r = archive_read_data_block(ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF)
      {
      return (ARCHIVE_OK);
      }
    if (r != ARCHIVE_OK)
      {
      return (r);
      }
    r = archive_write_data_block(aw, buff, size, offset);
    if (r != ARCHIVE_OK) 
      {
      printf("Problem with archive_write_data_block(): %s", archive_error_string(aw));
      return (r);
      }
    }
  return r;
}


//-----------------------------------------------------------------------------
bool extract_tar(const char* outFileName, bool verbose, bool extract)
{
  struct archive* a = archive_read_new();
  struct archive *ext = archive_write_disk_new();
  archive_read_support_compression_all(a);
  archive_read_support_format_all(a);
  struct archive_entry *entry;
  int r = archive_read_open_file(a, outFileName, 10240);
  if(r)
    { 
    printf("Problem with archive_read_open_file(): %s",
           archive_error_string(a));
    return false;
    }
  for (;;) 
    {
    r = archive_read_next_header(a, &entry);
    if (r == ARCHIVE_EOF)
      {
      break;
      }
    if (r != ARCHIVE_OK)
      {
      printf("Problem with archive_read_next_header(): %s",
             archive_error_string(a));
      }
    if (verbose && extract)
      {
      printf( "x: %s", archive_entry_pathname(entry) );
      }
    if(verbose && !extract)
      {
      list_item_verbose(stdout, entry);
      }
    else if(!extract)
      {
      printf( archive_entry_pathname(entry));
      }
    if(extract)
      {
      r = archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME);
      if (r != ARCHIVE_OK)
        {
        printf( "Problem with archive_write_disk_set_options(): %s",
                archive_error_string(a));
        }

      r = archive_write_header(ext, entry);
      if (r != ARCHIVE_OK)
        {
        printf( "Problem with archive_write_header(): %s",
                archive_error_string(a));
        printf( "Current file: %s",
                archive_entry_pathname(entry) );
        }
      else 
        {
        copy_data(a, ext);
        r = archive_write_finish_entry(ext);
        if (r != ARCHIVE_OK)
          {
          printf("Problem with archive_write_finish_entry(): %s",
                 archive_error_string(ext));
          }
        }
      }
    if (verbose || !extract)
      {
      printf("\n");
      }
    }
  archive_read_close(a);
  archive_read_finish(a);
  
  return true;
}
