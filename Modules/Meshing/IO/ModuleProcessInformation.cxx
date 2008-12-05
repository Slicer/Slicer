/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Module Description Parser
  Module:    $HeadURL: http://www.na-mic.org:8000/svn/Slicer3/trunk/Libs/ModuleDescriptionParser/ModuleProcessInformation.cxx $
  Date:      $Date: 2007/04/09 02:28:29 $
  Version:   $Revision: 1.1 $

==========================================================================*/
#include "ModuleProcessInformation.h"


std::ostream& operator<<(std::ostream &os, const ModuleProcessInformation &p)
{
  os << "  Abort: " << p.Abort << std::endl;
  os << "  Progress: " << p.Progress << std::endl;
  os << "  Stage Progress: " << p.StageProgress << std::endl;
  os << "  ProgressMessage: " << p.ProgressMessage << std::endl;
  os << "  ElapsedTime: " << p.ElapsedTime << std::endl;

  return os;
}
