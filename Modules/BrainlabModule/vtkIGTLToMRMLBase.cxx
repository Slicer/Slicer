/*==========================================================================

  Portions (c) Copyright 2008-2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Modules/OpenIGTLinkIF/vtkIGTLToMRMLBase.cxx $
  Date:      $Date: 2009-10-05 17:19:02 -0400 (Mon, 05 Oct 2009) $
  Version:   $Revision: 10576 $

==========================================================================*/

#include <vtksys/SystemTools.hxx>

#include "vtkObjectFactory.h"
#include "vtkIGTLToMRMLBase.h"

#include <string.h>

#include "igtlMessageBase.h"

vtkStandardNewMacro(vtkIGTLToMRMLBase);
vtkCxxRevisionMacro(vtkIGTLToMRMLBase, "$Revision: 10576 $");


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


