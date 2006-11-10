/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Module Description Parser
  Module:    $HeadURL: http://svn.na-mic.org:8000/svn/Slicer3/trunk/Libs/ModuleDescriptionParser/ModuleParameter.cxx $
  Date:      $Date: 2006-10-26 13:41:16 -0400 (Thu, 26 Oct 2006) $
  Version:   $Revision: 1526 $

==========================================================================*/
#include "ModuleProcessInformation.h"


std::ostream& operator<<(std::ostream &os, const ModuleProcessInformation &p)
{
  os << "  Abort: " << p.Abort << std::endl;
  os << "  Progress: " << p.Progress << std::endl;
  os << "  ProgressMessage: " << p.ProgressMessage << std::endl;
  os << "  ElapsedTime: " << p.ElapsedTime << std::endl;

  return os;
}
