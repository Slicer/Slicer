/*=auto=========================================================================

Portions (c) Copyright 2017 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/


#include "vtkArchive.h"
#include "vtkLoggingMacros.h"
#include "vtksys/Glob.hxx"
#include "vtksys/SystemTools.hxx"

// LibArchive includes
#include <archive.h>
#include <archive_entry.h>

// STD includes
#include <cstring>
#include <iostream>

// VTK include
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkArchive);


namespace
{

// --------------------------------------------------------------------------
class vtkArchiveTools
{
public:
  static void Message(const char* title, const char* message)
    {
    vtkInfoWithoutObjectMacro(<< title << " " << message);
    }
  static void Message(const char* m)
    {
    vtkInfoWithoutObjectMacro(<< m);
    }
  static void Stdout(const char* s)
  {
    std::cout << s;
    std::cout.flush();
  }
  static void Error(const char* m1, const char* m2)
  {
    std::string message = "vtkArchive Error: ";
    if(m1)
      {
      message += m1;
      }
    if(m2)
      {
      message += " ";
      message += m2;
      }
    vtkArchiveTools::Message(message.c_str(), "Error");
  }
};

// --------------------------------------------------------------------------
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
  if ((p == nullptr) || (*p == '\0'))
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
  if (p != nullptr && p[0] != '\0')
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

// --------------------------------------------------------------------------
long copy_data(struct archive *ar, struct archive *aw)
{
  long r;
  const void *buff;
  size_t size;
#if defined(ARCHIVE_VERSION_NUMBER) && ARCHIVE_VERSION_NUMBER >= 3000000
  __LA_INT64_T offset;
#else
  off_t offset;
#endif

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
      vtkArchiveTools::Message("archive_write_data_block()",
                             archive_error_string(aw));
      return (r);
      }
    }
  return r;
}

} // end of anonymous namespace

//----------------------------------------------------------------------------
vtkArchive::vtkArchive() = default;

//----------------------------------------------------------------------------
vtkArchive::~vtkArchive() = default;

//----------------------------------------------------------------------------
void vtkArchive::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//-----------------------------------------------------------------------------
bool vtkArchive::ListArchive(const char* archiveFileName, std::vector<std::string>& files)
{
  struct archive* a = archive_read_new();

  archive_read_support_filter_all(a);
  archive_read_support_format_all(a);

  if (archive_read_open_filename(a, archiveFileName, 10240) != ARCHIVE_OK)
    {
    vtkArchiveTools::Error("Problem with archive_read_open_filename(): ",
                           archive_error_string(a));
    return false;
    }

  files.clear();

  int r;
  struct archive_entry* entry;
  while ((r = archive_read_next_header(a, &entry)) != ARCHIVE_EOF)
    {
    if (r != ARCHIVE_OK)
      {
      vtkArchiveTools::Error("Problem with archive_read_next_header(): ",
                             archive_error_string(a));
      return false;
      }
    files.emplace_back(archive_entry_pathname(entry));
    }

  archive_read_close(a);
  archive_read_free(a);
  return true;
}

//-----------------------------------------------------------------------------
bool vtkArchive::ExtractTar(const char* tarFileName, bool verbose, bool extract, std::vector<std::string> * extracted_files)
{
  struct archive* a = archive_read_new();
  struct archive *ext = archive_write_disk_new();
  archive_read_support_filter_all(a);
  archive_read_support_format_all(a);
  struct archive_entry *entry;
  int r = archive_read_open_filename(a, tarFileName, 10240);
  if(r)
    {
    vtkArchiveTools::Error("Problem with archive_read_open_filename(): ",
                         archive_error_string(a));
    return false;
    }
  for (;;)
    {
    std::string message;
    r = archive_read_next_header(a, &entry);
    if (r == ARCHIVE_EOF)
      {
      break;
      }
    if (r != ARCHIVE_OK)
      {
      vtkArchiveTools::Error("Problem with archive_read_next_header(): ",
                           archive_error_string(a));
      }
    if ( extract && extracted_files)
      {
      extracted_files->push_back(archive_entry_pathname(entry));
      }
    if (verbose && extract)
      {
      message += "x ";
      message += +archive_entry_pathname(entry);
      }
    if(verbose && !extract)
      {
      list_item_verbose(stdout, entry);
      }
    else if(!extract)
      {
      message += archive_entry_pathname(entry);
      }
    if(extract)
      {
      r = archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME);
      if (r != ARCHIVE_OK)
        {
        vtkArchiveTools::Error(
          "Problem with archive_write_disk_set_options(): ",
          archive_error_string(ext));
        }

      r = archive_write_header(ext, entry);
      if (r != ARCHIVE_OK)
        {
        vtkArchiveTools::Error("Problem with archive_write_header(): ",
                             archive_error_string(ext));
        vtkArchiveTools::Error("Current file:",
                             archive_entry_pathname(entry));
        }
      else
        {
        r = copy_data(a, ext);
        if (r != ARCHIVE_OK)
          {
          vtkArchiveTools::Error("Problem with copy_data(): source:",
                               archive_error_string(a));
          vtkArchiveTools::Error("destination: ",
                               archive_error_string(ext));
          }
        r = archive_write_finish_entry(ext);
        if (r != ARCHIVE_OK)
          {
          vtkArchiveTools::Error("Problem with archive_write_finish_entry(): ",
                               archive_error_string(ext));
          }
        }
      }
    if (verbose && !message.empty())
      {
      vtkArchiveTools::Message(message.c_str());
      }
    }
  archive_read_close(a);
  archive_read_free(a);
  return true;
}

//-----------------------------------------------------------------------------
// creates a zip file with the full contents of the directory (recurses)
// zip entries will include relative path of including tail of directoryToZip
bool vtkArchive::Zip(const char* zipFileName, const char* directoryToZip)
{

  //
  // to make a zip file:
  // - check that libarchive supports zip writing
  // - check arguments
  // - get a list of files using vtksys Glob
  // - create the archive
  // -- go file-by-file and add chunks of data to the archive
  // - close up and return success
  //

// only support the libarchive version 3.0 +
#if !defined(ARCHIVE_VERSION_NUMBER) || ARCHIVE_VERSION_NUMBER < 3000000
  return false;
#endif

  if ( !zipFileName || !directoryToZip )
    {
    vtkArchiveTools::Error("Zip:", "Invalid zipfile or directory");
    return false;
    }

  std::vector<vtksys::String> directoryParts;
  directoryParts = vtksys::SystemTools::SplitString(directoryToZip, '/', true);
  std::string directoryName = directoryParts.back();

  vtksys::Glob glob;
  glob.RecurseOn();
  glob.RecurseThroughSymlinksOff();
  std::string globPattern(directoryToZip);
  if ( !glob.FindFiles( globPattern + "/*" ) )
    {
    vtkArchiveTools::Error("Zip:", "Could not find files in directory");
    return false;
    }
  std::vector<std::string> files = glob.GetFiles();

  // now zip it up using LibArchive
  struct archive *zipArchive;
  struct archive_entry *entry, *dirEntry;
  char buff[BUFSIZ];
  size_t len;
  // have to read the contents of the files to add them to the archive
  FILE *fd;

  zipArchive = archive_write_new();

  // create a zip archive
#ifdef HAVE_ZLIB_H
  std::string compression_type = "deflate";
#else
  std::string compression_type = "store";
#endif

  archive_write_set_format_zip(zipArchive);

  archive_write_set_format_option(zipArchive, "zip", "compression", compression_type.c_str());

  archive_write_open_filename(zipArchive, zipFileName);

  // add the data directory
  dirEntry = archive_entry_new();
  archive_entry_set_mtime(dirEntry, 11, 110);
  archive_entry_copy_pathname(dirEntry, directoryName.c_str());
  archive_entry_set_mode(dirEntry, S_IFDIR | 0755);
  archive_entry_set_size(dirEntry, 512);
  archive_write_header(zipArchive, dirEntry);
  archive_entry_free(dirEntry);

  // add the files
  std::vector<std::string>::const_iterator sit;
  sit = files.begin();
  while (sit != files.end())
    {
    vtkArchiveTools::Message("Zip: adding:", (*sit).c_str());
    const char *fileName = (*sit).c_str();
    ++sit;

    //
    // add an entry for this file
    //
    entry = archive_entry_new();
    // use a relative path for the entry file name, including the top
    // directory so it unzips into a directory of it's own
    std::string relFileName = vtksys::SystemTools::RelativePath(
              vtksys::SystemTools::GetParentDirectory(directoryToZip).c_str(),
              fileName);
    vtkArchiveTools::Message("Zip: adding rel:", relFileName.c_str());
    archive_entry_set_pathname(entry, relFileName.c_str());
    // size is required, for now use the vtksys call though it uses struct stat
    // and may not be portable
    unsigned long fileLength = vtksys::SystemTools::FileLength(fileName);
    archive_entry_set_size(entry, fileLength);
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_perm(entry, 0644);
    archive_write_header(zipArchive, entry);

    //
    // add the data for this entry
    //
    fd = fopen(fileName, "rb");
    if (!fd)
      {
      vtkArchiveTools::Error("Zip: cannot open:", (*sit).c_str());
      }
    else
      {
      len = fread(buff, sizeof(char), sizeof(buff), fd);
      while ( len > 0 )
        {
        archive_write_data(zipArchive, buff, len);
        len = fread(buff, sizeof(char), sizeof(buff), fd);
        }
      fclose(fd);
      }
    archive_entry_free(entry);
    }

  archive_write_close(zipArchive);
  int retval = archive_write_free(zipArchive);
  if (retval != ARCHIVE_OK)
    {
    vtkArchiveTools::Error("Zip:", "error on close!");
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
// unzips zip file into destinationDirectory
bool vtkArchive::UnZip(const char* zipFileName, const char* destinationDirectory)
{
  //
  // Unziping the archive
  // - check that files and directories exist
  // - cd to destination
  // - create an extractor from the file
  // - create a writer to disk
  // - read all headers and data into disk
  // - close up the archives
  // - cd back to original directory
  //

  if ( !zipFileName || !destinationDirectory )
    {
    vtkArchiveTools::Error("Unzip:", "Invalid zipfile or directory");
    return false;
    }

  if ( !vtksys::SystemTools::FileExists(zipFileName) )
    {
    vtkArchiveTools::Error("Unzip:", "Zip file does not exist");
    return false;
    }

  if ( !vtksys::SystemTools::FileIsDirectory(destinationDirectory) )
    {
    vtkArchiveTools::Error("Unzip:", "Destination is not a directory");
    return false;
    }

  std::string cwd = vtksys::SystemTools::GetCurrentWorkingDirectory(true);

  if ( vtksys::SystemTools::ChangeDirectory(destinationDirectory) )
    {
    vtkArchiveTools::Error("Unzip:", "could not change to destination directory");
    return false;
    }

  struct archive *zipArchive;
  struct archive *diskDestination;
  struct archive_entry *entry;
  int result;

  zipArchive = archive_read_new();
  // we will typically have zip files, but support all archive types (why not?)
  archive_read_support_filter_all(zipArchive);
  archive_read_support_format_all(zipArchive);
  // Note: the 10240 is just a suggested block size
  result = archive_read_open_filename(zipArchive, zipFileName, 10240);
  if (result != ARCHIVE_OK)
    {
    vtkArchiveTools::Error("Unzip:", "Cannot open archive file");
    return false;
    }

  diskDestination = archive_write_disk_new();
  archive_write_disk_set_standard_lookup(diskDestination);

  for (;;)
    {
    // for each file entry
    result = archive_read_next_header(zipArchive, &entry);
    if (result == ARCHIVE_EOF)
      {
      break;
      }
    if (result != ARCHIVE_OK)
      {
      vtkArchiveTools::Error("Unzip error:", archive_error_string(zipArchive));
      if (result < ARCHIVE_WARN)
        {
        break;
        }
      }
    result = archive_write_header(diskDestination, entry);
    if (result != ARCHIVE_OK)
      {
      vtkArchiveTools::Error("Unzip error:", archive_error_string(diskDestination));
      if (result < ARCHIVE_WARN)
        {
        break;
        }
      }
    else
      {
      // copy data
      const void *buff;
      size_t size;
#if defined(ARCHIVE_VERSION_NUMBER) && ARCHIVE_VERSION_NUMBER >= 3000000
      __LA_INT64_T offset;
#else
      off_t offset;
#endif

      for (;;)
        {
        result = archive_read_data_block(zipArchive, &buff, &size, &offset);
        if (result == ARCHIVE_EOF)
          {
          break;
          }
        if (result != ARCHIVE_OK)
          {
          vtkArchiveTools::Error("Unzip error:", archive_error_string(zipArchive));
          break;
          }
        result = archive_write_data_block(diskDestination, buff, size, offset);
        if (result != ARCHIVE_OK)
          {
          vtkArchiveTools::Error("Unzip error:", archive_error_string(diskDestination));
          break;
          }
        }
      }
    }

  result = archive_read_close(zipArchive);
  if (result != ARCHIVE_OK)
    {
    vtkArchiveTools::Error("Unzip closing zipfile:", archive_error_string(zipArchive));
    return false;
    }
  result = archive_read_free(zipArchive);
  if (result != ARCHIVE_OK)
    {
    vtkArchiveTools::Error("Unzip freeing zipfile:", archive_error_string(zipArchive));
    return false;
    }
  result = archive_write_close(diskDestination);
  if (result != ARCHIVE_OK)
    {
    vtkArchiveTools::Error("Unzip closing disk:", archive_error_string(diskDestination));
    return false;
    }
  result = archive_write_free(diskDestination);
  if (result != ARCHIVE_OK)
    {
    vtkArchiveTools::Error("Unzip freeing disk:", archive_error_string(diskDestination));
    return false;
    }


  if ( vtksys::SystemTools::ChangeDirectory(cwd.c_str()) )
    {
    vtkArchiveTools::Error("Unzip:", "could not change back to working directory");
    return false;
    }

  return (result == ARCHIVE_OK);
}
