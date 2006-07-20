/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Module Description Parser
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/
// .NAME ModuleDescriptionParserWin32Header - manage Windows system differences
// .SECTION Description
// The ModuleDescriptionParserWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __ModuleDescriptionParserWin32Header_h
#define __ModuleDescriptionParserWin32Header_h

#include <ModuleDescriptionParserConfigure.h>

#if defined(WIN32) && !defined(ModuleDescriptionParser_STATIC)
#if defined(ModuleDescriptionParser_EXPORTS)
#define ModuleDescriptionParser_EXPORT __declspec( dllexport ) 
#else
#define ModuleDescriptionParser_EXPORT __declspec( dllimport ) 
#endif
#else
#define ModuleDescriptionParser_EXPORT 
#endif

#if defined(_MSC_VER)
#  pragma warning (disable: 4251) /* missing DLL-interface */
#endif

#endif
