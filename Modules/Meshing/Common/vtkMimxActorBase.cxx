/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxActorBase.cxx,v $
Language:  C++
Date:      $Date: 2007/07/18 16:16:37 $
Version:   $Revision: 1.8 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "vtkMimxActorBase.h"
#include "vtkActor.h"
#include "vtkDataSet.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"


vtkCxxRevisionMacro(vtkMimxActorBase, "$Revision: 1.8 $");


//vtkStandardNewMacro(vtkActorBase);

vtkMimxActorBase::vtkMimxActorBase()
{
  this->FileName = new char[1024];
  this->FilePath = new char[1024];
}

vtkMimxActorBase::~vtkMimxActorBase()
{
  delete this->FileName;
  delete this->FilePath;
}

void vtkMimxActorBase::SetFileName( const char *InputFileName)
{
  strcpy(this->FileName, InputFileName);
}

void vtkMimxActorBase::SetFilePath(const char *InputFilePath)
{
  strcpy(this->FilePath, InputFilePath);
}

void vtkMimxActorBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

void vtkMimxActorBase::SetObjectName(const char* FileName, vtkIdType Count)
{
  strcpy(this->FileName, FileName);
  char buffer[10];
  sprintf(buffer, "%d", Count);
  strcat(this->FileName, buffer);
}
