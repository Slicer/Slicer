/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTextureFontManager.h,v $
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


#ifndef __vtkTextureFontManager_h
#define __vtkTextureFontManager_h

#include "vtkTextureFont.h"

#include <vtkQueryAtlasConfigure.h>


class vtkFontParameters;


class VTK_QUERYATLAS_EXPORT vtkTextureFontManager : public vtkObject
{
public:
    void PrintSelf(ostream& os, vtkIndent indent);
    vtkTypeRevisionMacro(vtkTextureFontManager,vtkObject);

    static vtkTextureFontManager *New();

    static vtkTextureFontManager *GetDefaultManager();

    vtkTextureFont *GetTextureFont(vtkFontParameters *p);

    vtkGetObjectMacro(TextureFonts, vtkCollection);

    static char *GetDefaultFreetypeDirectory();
    static void SetDefaultFreetypeDirectory(char *dirname);
    static char *GetDefaultFreetypeFileName();
    static void SetDefaultFreetypeFileName(char *dirname);

    int IsDefaultSettings(vtkFontParameters *p);
    void SetToDefaultSettings(vtkFontParameters *p);

protected:
    vtkTextureFontManager();
    ~vtkTextureFontManager();

    static vtkTextureFontManager *sDefaultManager;

    vtkCollection *TextureFonts;

    int Initialized;

    static char *sDefaultFreetypeDirectory;
    static char *sDefaultFreetypeFileName;
};

#endif
