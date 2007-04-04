/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkEdgeGradient.h,v $
  Date:      $Date: 2006/05/26 19:59:41 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
/*===========================================================

Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/


#ifndef __vtkEdgeGradient_h
#define __vtkEdgeGradient_h

#include "vtkImageData.h"

#include "vtkQueryAtlasWin32Header.h"



class VTK_QUERYATLAS_EXPORT vtkEdgeGradient : public vtkImageData
{

public:
    void PrintSelf(ostream& os, vtkIndent indent);
    vtkTypeRevisionMacro(vtkEdgeGradient,vtkImageData);

    static vtkEdgeGradient *New();

    vtkGetMacro(Width, int);
    vtkGetMacro(Height, int);

    vtkGetMacro(Initialized,int);
    vtkGetMacro(Error,int);

    //vtkGetMacro(Turn,int);
    //vtkSetMacro(Turn,int);

    vtkGetMacro(Corner,int);
    vtkSetMacro(Corner,int);

    void Create();

protected:
    vtkEdgeGradient();
    ~vtkEdgeGradient();

    int    Width;
    int Height;

    //int Turn;
    int Corner;

    int Initialized;
    int Error;

    vtkEdgeGradient *filter_image(unsigned char *src, unsigned char *dst, int wd, int ht, int filter); 
    void copy_array(unsigned char *dst, unsigned char *src, int size);
    void zero_array(unsigned char *ptr, int size);

    void MakeImageData(unsigned char *Alpha, unsigned char *intensity);

//BTX
    void copyArrayOffset(unsigned char *dst, 
                                     unsigned char *src,
                                     int w, int h, int xOff, int yOff);

    void mattArrayOffset(unsigned char *dstAlpha, unsigned char *dstI, 
                                     unsigned char *srcAlpha, unsigned char srcLevel, 
                                     int w, int h, int xOff, int yOff);
//ETX
};

#endif
