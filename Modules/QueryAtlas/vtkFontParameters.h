/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFontParameters.h,v $
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


#ifndef __vtkFontParameters_h
#define __vtkFontParameters_h

#include <vtkObject.h>
#include <vtkObjectFactory.h>

//#include "vtkTextureText.h"
//#include "vtkTextureFont.h"

class vtkTextureText;
class vtkTextureFont;

#include "vtkQueryAtlasWin32Header.h"


enum {
    VTK_TEXTURE_TEXT_STYLE_PLAIN = 0,
    VTK_TEXTURE_TEXT_STYLE_OUTLINE = 1,
    VTK_TEXTURE_TEXT_STYLE_SHADOW = 2
};


class VTK_QUERYATLAS_EXPORT vtkFontParameters : public vtkObject
{
public:
    void PrintSelf(ostream& os, vtkIndent indent);
    vtkTypeRevisionMacro(vtkFontParameters,vtkObject);

    static vtkFontParameters *New();

    void Modified();
    vtkTextureFont *RequestTextureFont();

    vtkGetMacro(Resolution, int);
    vtkSetMacro(Resolution, int);

    vtkGetMacro(Blur, int);
    vtkSetMacro(Blur, int);

    vtkGetMacro(Style, int);
    vtkSetMacro(Style, int);

    vtkGetMacro(StyleBorderSize, int);
    vtkSetMacro(StyleBorderSize, int);

    vtkGetStringMacro(FontFileName);
    vtkSetStringMacro(FontFileName);
    vtkGetStringMacro(FontDirectory);
    
    // use a method, that adds a trailing '/' if necessary
    void SetFontDirectory(char *dirname);


    vtkGetObjectMacro(TextureText, vtkTextureText);
    // NOTE - for some reason, I can't use the Set macro w/ these classes - seemingly because the header files are cross-referenced.
    //vtkSetObjectMacro(TextureText, vtkTextureText);

    vtkGetObjectMacro(TextureFont, vtkTextureFont);
    //vtkSetObjectMacro(TextureFont, vtkTextureFont);


    // TODO - make private, and provide the Set methods
    vtkTextureText *TextureText;
    vtkTextureFont *TextureFont;


protected:
    vtkFontParameters();
    ~vtkFontParameters();

    int Blur;
    int Style;
    int StyleBorderSize;
    int Resolution;
    char *FontFileName;
    char *FontDirectory;
};

#endif
