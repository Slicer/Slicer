/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkRasterizerCharacter.h,v $
  Date:      $Date: 2006/05/26 19:59:43 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
/*===========================================================

Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/


#ifndef __vtkRasterizerCharacter_h
#define __vtkRasterizerCharacter_h

#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include "vtkQueryAtlasWin32Header.h"


class VTK_QUERYATLAS_EXPORT vtkRasterizerCharacter : public vtkObject {
public:

     void PrintSelf(ostream& os, vtkIndent indent);
    vtkTypeRevisionMacro(vtkRasterizerCharacter,vtkObject);

    static vtkRasterizerCharacter *New();

    int GetBottomExtent();
    int GetRightExtent();

    int mAsciiValue;

    int mHeight;
    int mWidth;
    int mAdvanceWidth;
    int mTopExtent;
    int mLeftExtent;

    unsigned char *mBitmap;


protected:
    vtkRasterizerCharacter();
    ~vtkRasterizerCharacter();


private:
    vtkRasterizerCharacter(const vtkRasterizerCharacter&);  // Not implemented.
    void operator=(const vtkRasterizerCharacter&);  // Not implemented.
};

#endif
