/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkRasterizerCharacter.cxx,v $
  Date:      $Date: 2006/05/26 19:59:43 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
/*
Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/


#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include "vtkRasterizerCharacter.h"


vtkCxxRevisionMacro(vtkRasterizerCharacter, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkRasterizerCharacter);


vtkRasterizerCharacter::vtkRasterizerCharacter()
{
    this->mAsciiValue = 0;
    this->mHeight = 0;
    this->mWidth = 0;
    this->mAdvanceWidth = 0;
    this->mTopExtent = 0;
    this->mLeftExtent = 0;
    this->mBitmap = NULL;
}


vtkRasterizerCharacter::~vtkRasterizerCharacter()
{
    if (this->mBitmap != NULL) free(this->mBitmap);
}


int vtkRasterizerCharacter::GetBottomExtent() {
    return (this->mTopExtent - this->mHeight);
}


int vtkRasterizerCharacter::GetRightExtent() {
    return (this->mLeftExtent + this->mWidth);
}


void vtkRasterizerCharacter::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    /*  TODO - do print
    os << indent << "X Resolution: " << this->XResolution << "\n";
    os << indent << "Y Resolution: " << this->YResolution << "\n";
    ...
    */
}
