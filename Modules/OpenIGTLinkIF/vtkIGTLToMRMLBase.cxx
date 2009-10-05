/*==========================================================================

  Portions (c) Copyright 2008-2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#include <vtksys/SystemTools.hxx>

#include "vtkObjectFactory.h"
#include "vtkIGTLToMRMLBase.h"

#include <string.h>

#include "igtlMessageBase.h"

vtkStandardNewMacro(vtkIGTLToMRMLBase);
vtkCxxRevisionMacro(vtkIGTLToMRMLBase, "$Revision$");


//---------------------------------------------------------------------------
vtkIGTLToMRMLBase::vtkIGTLToMRMLBase()
{
  this->CheckCRC = 1;
}


//---------------------------------------------------------------------------
vtkIGTLToMRMLBase::~vtkIGTLToMRMLBase()
{
}


//---------------------------------------------------------------------------
void vtkIGTLToMRMLBase::PrintSelf(ostream& os, vtkIndent indent)
{
}


