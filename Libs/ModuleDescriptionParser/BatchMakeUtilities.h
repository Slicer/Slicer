/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Module Description Parser
  Module:    $HeadURL: http://www.na-mic.org/svn/Slicer3/trunk/Libs/ModuleDescriptionParser/ModuleDescription.h $
  Date:      $Date: 2007-01-30 09:26:53 -0500 (Tue, 30 Jan 2007) $
  Version:   $Revision: 2384 $

==========================================================================*/

#ifndef __BatchMakeUtilities_h
#define __BatchMakeUtilities_h

#include "ModuleDescriptionParserWin32Header.h"

#include "ModuleDescription.h"

ModuleDescriptionParser_EXPORT std::string GenerateBatchMakeWrapper(const ModuleDescription &module);

#endif
