/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkRectangle.h,v $
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


// .NAME vtkRectangle - create a basic rectangle polygon in a plane
// .SECTION Description


#ifndef __vtkRectangle_h
#define __vtkRectangle_h

#include "vtkPolyDataSource.h"
//#include "vtkFloatArray.h"

#include "vtkQueryAtlasWin32Header.h"

class vtkTextureText;


class VTK_QUERYATLAS_EXPORT vtkRectangle : public vtkPolyDataSource 
{
public:
    void PrintSelf(ostream& os, vtkIndent indent);
    vtkTypeRevisionMacro(vtkRectangle,vtkPolyDataSource);

    // Description:
    // Construct a rectangular polygon perpendicular to z-axis, used to make a 
    // surrounding box around a vtkTextureType object.
    static vtkRectangle *New();

    vtkGetMacro(Width, vtkFloatingPointType);
    vtkSetMacro(Width, vtkFloatingPointType);
    vtkGetMacro(Height, vtkFloatingPointType);
    vtkSetMacro(Height, vtkFloatingPointType);

    vtkGetMacro(Left, vtkFloatingPointType);
    vtkSetMacro(Left, vtkFloatingPointType);
    vtkGetMacro(Bottom, vtkFloatingPointType);
    vtkSetMacro(Bottom, vtkFloatingPointType);

    vtkGetMacro(ZOffset, vtkFloatingPointType);
    vtkSetMacro(ZOffset, vtkFloatingPointType);

    vtkGetMacro(TextureTurn, int);
    vtkSetMacro(TextureTurn, int);

//    void SetScalePoints(vtkFloatingPointType x, vtkFloatingPointType y);
    void SetPositionOffset(vtkFloatingPointType x, vtkFloatingPointType y, vtkFloatingPointType z);
    void GetPositionOffset(vtkFloatingPointType pos[3]);

    void AddPositionOffset(vtkFloatingPointType x, vtkFloatingPointType y, vtkFloatingPointType z);
    void AddPositionOffset(vtkFloatingPointType pos[3]);

protected:
  vtkRectangle();
  ~vtkRectangle() {};

  void Execute();

  vtkFloatingPointType Normal[3];
  vtkFloatingPointType Width;
  vtkFloatingPointType Height;
  vtkFloatingPointType Left;
  vtkFloatingPointType Bottom;
  vtkFloatingPointType ZOffset;

  // The TextureTurn count is the # of 90 degree clockwise turns to make for the texture coords
  int TextureTurn;

private:
  vtkRectangle(const vtkRectangle&);  // Not implemented.
  void operator=(const vtkRectangle&);  // Not implemented.
};

#endif


