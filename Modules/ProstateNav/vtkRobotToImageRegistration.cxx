/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.slicer.org/Slicer3/trunk/Modules/OpenIGTLinkIF/vtkRobotRegistration.cxx $
  Date:      $Date: 2009-01-05 13:28:20 -0500 (Mon, 05 Jan 2009) $
  Version:   $Revision: 8267 $

==========================================================================*/

#include "vtkObjectFactory.h"

#include "vtkRobotToImageRegistration.h"

#include <string.h>

vtkStandardNewMacro(vtkRobotToImageRegistration);
vtkCxxRevisionMacro(vtkRobotToImageRegistration, "$Revision: 8267 $");


//---------------------------------------------------------------------------
vtkRobotToImageRegistration::vtkRobotToImageRegistration()
{
  this->FiducialVolume        = NULL;
  this->RobotToImageTransform = NULL;
}


//---------------------------------------------------------------------------
vtkRobotToImageRegistration::~vtkRobotToImageRegistration()
{
}


//---------------------------------------------------------------------------
void vtkRobotToImageRegistration::PrintSelf(ostream& os, vtkIndent indent)
{
}


