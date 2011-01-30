/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFreetypeRasterizer.h,v $
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


#ifndef __vtkFreetypeRasterizer_h
#define __vtkFreetypeRasterizer_h

#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkCollection.h>

#include "vtkQueryAtlasWin32Header.h"

#include "vtkRasterizerCharacter.h"



#define VTK_FONT_CHAR_OFFSET (33)
#define VTK_FONT_CHAR_NUM (255 - VTK_FONT_CHAR_OFFSET)


class VTK_QUERYATLAS_EXPORT vtkFreetypeRasterizer : public vtkObject {

public:
     void PrintSelf(ostream& os, vtkIndent indent);
    vtkTypeRevisionMacro(vtkFreetypeRasterizer,vtkObject);

    static vtkFreetypeRasterizer *New();

    int LoadFont(char *fontname);

    int mError;

    int mIsKerning;
    int mKernPairs[VTK_FONT_CHAR_NUM][VTK_FONT_CHAR_NUM];

    vtkCollection *mRasterizerCharacters;

    static void SetDefaultRasterizerResolution(int res);
    static int GetDefaultRasterizerResolution();

    vtkFloatingPointType GetRasterizerScale();

    vtkFloatingPointType GetAdvanceWidthKerned(int c, int c2);

    vtkSetMacro(Resolution, int);

protected:
    vtkFreetypeRasterizer();
    ~vtkFreetypeRasterizer();

    int Resolution;
    static int sDefaultRasterizerResolution;


private:
    vtkFreetypeRasterizer(const vtkFreetypeRasterizer&);  // Not implemented.
    void operator=(const vtkFreetypeRasterizer&);  // Not implemented.
};

#endif

