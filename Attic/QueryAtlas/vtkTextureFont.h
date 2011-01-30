/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTextureFont.h,v $
  Date:      $Date: 2006/05/26 19:59:44 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
/*===========================================================

Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/


#ifndef __vtkTextureFont_h
#define __vtkTextureFont_h


#include "vtkFreetypeRasterizer.h"
#include "vtkFontParameters.h"

#include "vtkImageData.h"
#include "vtkQueryAtlasWin32Header.h"

#include "simpleVectors.h"


class VTK_QUERYATLAS_EXPORT vtkTextureFont : public vtkImageData
{

public:
    void PrintSelf(ostream& os, vtkIndent indent);
    vtkTypeRevisionMacro(vtkTextureFont,vtkImageData);

    static vtkTextureFont *New();

    int CreateFont();
    int CreateRasterizer(char *fontpath);

//BTX
    Vector2D<vtkFloatingPointType> GetChararacterPosition(int c);
    Vector2D<vtkFloatingPointType> GetChararacterSize(int c);
//ETX

    vtkGetMacro(Width, int);
    vtkGetMacro(Height, int);

    vtkGetObjectMacro(FreetypeRasterizer,vtkFreetypeRasterizer);
    vtkGetObjectMacro(FontParameters,vtkFontParameters);
    vtkGetMacro(Initialized,int);
    vtkGetMacro(Error,int);


protected:
    vtkTextureFont();
    ~vtkTextureFont();

//BTX
    Vector2D<vtkFloatingPointType> *mCharPositions;
    Vector2D<vtkFloatingPointType> *mCharSizes;
//ETX
    vtkFontParameters *FontParameters;
    vtkFreetypeRasterizer *FreetypeRasterizer;

//    char *FontFileName;
//    char *FontDirectory;

    int    Width;
    int Height;

    int Initialized;
    int Error;

    vtkTextureFont *loadFont( char *fontname, int blur = 0);   
    vtkTextureFont *filter_image(unsigned char *src, unsigned char *dst, int wd, int ht, int filter); 
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
