/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Module Description Parser
  Module:    $HeadURL: http://svn.na-mic.org:8000/svn/Slicer3/trunk/Libs/ModuleDescriptionParser/ModuleDescription.h $
  Date:      $Date: 2006-06-20 16:19:04 -0400 (Tue, 20 Jun 2006) $
  Version:   $Revision: 625 $

==========================================================================*/

#ifndef __ModuleProcessInformation_h
#define __ModuleProcessInformation_h

#include <ostream>

extern "C" {
  struct ModuleProcessInformation
  {
    /** Inputs from calling application to the module **/
    char Abort;
    
    /** Outputs from the moduleto the calling application **/
    float Progress;
    char  ProgressMessage[1024];

    double ElapsedTime;

    void Initialize()
      {
        Abort = 0;
        Progress = 0;
        strcpy(ProgressMessage, "");
        ElapsedTime = 0.0;
      };
  };
};

std::ostream& operator<<(std::ostream &os, const ModuleProcessInformation &p);
    

#endif
