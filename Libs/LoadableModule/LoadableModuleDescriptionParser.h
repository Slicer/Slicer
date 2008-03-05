/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Loadable Module
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/
#ifndef __LoadableModuleDescriptionParser_h
#define __LoadableModuleDescriptionParser_h

#include <string>
#include <vector>

#include "LoadableModuleWin32Header.h"

class LoadableModuleDescription;

class LoadableModule_EXPORT LoadableModuleDescriptionParser
{
public:
  LoadableModuleDescriptionParser() {};
  ~LoadableModuleDescriptionParser() {};

  int Parse( const std::string& xml, LoadableModuleDescription& description);
};

#endif
