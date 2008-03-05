/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Module Description Parser
  Module:    $HeadURL: http://www.na-mic.org:8000/svn/Slicer3/trunk/Libs/ModuleDescriptionParser/ModuleFactory.h $
  Date:      $Date: 2007-01-30 09:26:53 -0500 (Tue, 30 Jan 2007) $
  Version:   $Revision: 2384 $

==========================================================================*/
#ifndef __BinaryFileDescriptor_h
#define __BinaryFileDescriptor_h

#include <utility>
#include <vector>

#include "ModuleDescriptionParserWin32Header.h"

#if !defined(WIN32)
#include <bfd.h>

// Implementation for Linux, Mac, etc. to allow us to peek into an
// executable file and determine whether it contains global symbols.
class ModuleDescriptionParser_EXPORT BinaryFileDescriptor
{
public:
  typedef std::pair<asection*, void* > MemorySectionType;
  typedef std::vector<MemorySectionType> MemorySectionContainer;

  BinaryFileDescriptor() {};
  ~BinaryFileDescriptor();

  // Open the object file to investigate
  bool Open(const char *filename);

  // Close the object file
  void Close();

  // Get the address of a symbol in memory
  void *GetSymbolAddress(const char *name);

private:
  bfd *BFD;
  MemorySectionContainer Sections;
};
#endif

#endif
