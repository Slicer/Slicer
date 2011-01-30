/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTextureTextPolys.h,v $
  Date:      $Date: 2006/05/26 19:59:45 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
/*===========================================================

Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/


// .NAME vtkTextureTextPolys - create an array of quadrilaterals located in a plane
// .SECTION Description


#ifndef __vtkTextureTextPolys_h
#define __vtkTextureTextPolys_h

#include "vtkPolyDataSource.h"


#include "vtkFloatArray.h"

#include "vtkQueryAtlasWin32Header.h"

class vtkTextureText;


class VTK_QUERYATLAS_EXPORT vtkTextureTextPolys : public vtkPolyDataSource 
{
public:
    void PrintSelf(ostream& os, vtkIndent indent);
    vtkTypeRevisionMacro(vtkTextureTextPolys,vtkPolyDataSource);

    // Description:
    // Construct a set of rectangular polygons perpendicular to z-axis, used to create
    // texture mapped based text objects, using a vtkTextureText layout manager to supply
    // data.
    static vtkTextureTextPolys *New();

    void SetTextureText(vtkTextureText *iT);
    vtkTextureText *mTextureText;


protected:
  vtkTextureTextPolys();
  ~vtkTextureTextPolys() {};

  void Execute();

  vtkFloatingPointType Normal[3];

private:
  vtkTextureTextPolys(const vtkTextureTextPolys&);  // Not implemented.
  void operator=(const vtkTextureTextPolys&);  // Not implemented.
};

#endif


