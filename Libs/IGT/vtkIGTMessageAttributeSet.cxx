/*=auto=========================================================================

Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: $
Date:      $Date: $
Version:   $Revision: $

=========================================================================auto=*/

#include "vtkObjectFactory.h"

#include "vtkIGTMessageAttributeSet.h"
#include "vtkIGTMessageAttributeBase.h"
#include "vtkIGTMessageGenericAttribute.h"
#include "vtkIGTMessageImageDataAttribute.h"

#include "vtkImageData.h"

vtkStandardNewMacro(vtkIGTMessageAttributeSet);
vtkCxxRevisionMacro(vtkIGTMessageAttributeSet, "$Revision: 1.0 $");


vtkIGTMessageAttributeSet::vtkIGTMessageAttributeSet()
{
  this->AttributeMap.clear();
  this->OpenTrackerStream = NULL;
  this->TimeStamp = vtkTimeStamp::New();
}


vtkIGTMessageAttributeSet::~vtkIGTMessageAttributeSet()
{
}


void vtkIGTMessageAttributeSet::PrintSelf(ostream& os, vtkIndent indent)
{
}

void vtkIGTMessageAttributeSet::SetHandlerFunction(MessageHandlingFunction* func, void* arg)
{
  this->HandlerFunction = func;
  this->HandlerArgument = arg;
}

vtkIGTMessageAttributeSet::MessageHandlingFunction* vtkIGTMessageAttributeSet::GetHandlerFunction()
{
  return this->HandlerFunction;
}

void* vtkIGTMessageAttributeSet::GetHandlerArgument()
{
  return this->HandlerArgument;
}

