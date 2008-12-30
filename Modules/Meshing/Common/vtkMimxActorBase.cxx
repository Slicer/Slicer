/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxActorBase.cxx,v $
Language:  C++
Date:      $Date: 2008/07/28 15:06:02 $
Version:   $Revision: 1.13 $

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


vtkCxxRevisionMacro(vtkMimxActorBase, "$Revision: 1.13 $");


//vtkStandardNewMacro(vtkActorBase);

vtkMimxActorBase::vtkMimxActorBase()
{
  this->FileName = new char[256];
  this->FilePath = new char[256];
  this->UniqueId = new char[64];
  this->FoundationName = new char[256];
  this->DataType = 0;
}

vtkMimxActorBase::~vtkMimxActorBase()
{
  delete this->FileName;
  delete this->FilePath;
  delete this->UniqueId;
  delete this->FoundationName;
}

//void vtkMimxActorBase::SetFileName( const char *InputFileName)
//{
//  strcpy(this->FileName, InputFileName);
//}

void vtkMimxActorBase::SetFilePath(const char *InputFilePath)
{
  strcpy(this->FilePath, InputFilePath);
}

void vtkMimxActorBase::SetUniqueId( const char *Id)
{
  strcpy(this->UniqueId, Id);
}

void vtkMimxActorBase::SetFoundationName(const char *created)
{
  strcpy(this->FoundationName, created);
  strcpy(this->FileName, created);
}

void vtkMimxActorBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

void vtkMimxActorBase::SetObjectName(const char* aFileName, vtkIdType &Count)
{
  char tempbuffer[1024];
  strcpy(tempbuffer, this->FoundationName);
  strcat(tempbuffer, "_");
  strcat(tempbuffer, aFileName);
  char buffer[10];
  sprintf(buffer, "%ld", (long) Count);
  strcat(tempbuffer, buffer);
  strcpy(this->FileName, tempbuffer);
}
