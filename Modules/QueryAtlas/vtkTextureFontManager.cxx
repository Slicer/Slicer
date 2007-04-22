/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTextureFontManager.cxx,v $
  Date:      $Date: 2006/05/26 19:59:44 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
/*
Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/


#include <math.h>

#include "vtkTextureFontManager.h"
#include "vtkFontParameters.h"

#include "vtkCollection.h"



// TODO - find a good method to auto determine a freetype font path.  
// if there isn't a standard, should use use an environment var?

#ifdef _WIN32 // WINDOWS
char *vtkTextureFontManager::sDefaultFreetypeDirectory = "C:/WINDOWS/Fonts/";
char *vtkTextureFontManager::sDefaultFreetypeFileName = "ARIAL.TTF";

#else // UNIX

// TODO - this is not a good unix font dir default - not really a good arial name, either

char *vtkTextureFontManager::sDefaultFreetypeDirectory = "/projects/mrrobot/nicole/cvs/Source/slicer2/Modules/vtkKinematics/cxx/";
char *vtkTextureFontManager::sDefaultFreetypeFileName = "arir____.ttf";
#endif


vtkTextureFontManager *vtkTextureFontManager::sDefaultManager = NULL;


vtkCxxRevisionMacro(vtkTextureFontManager, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkTextureFontManager);


vtkTextureFontManager::vtkTextureFontManager()
{
    this->TextureFonts = vtkCollection::New();
    //this->Error = 0;
    this->Initialized = 0;
}


vtkTextureFontManager::~vtkTextureFontManager()
{
    // clear/delete our list?
    // TODO - anything to delete?
}


vtkTextureFontManager *vtkTextureFontManager::GetDefaultManager() {
    if (vtkTextureFontManager::sDefaultManager == NULL) 
        vtkTextureFontManager::sDefaultManager = vtkTextureFontManager::New();
    return vtkTextureFontManager::sDefaultManager;
}


// NOTE - the DefaultSettings methods are in this class, since the default values are here, 
//   but the methods act on the vtkFontParameters.
int vtkTextureFontManager::IsDefaultSettings(vtkFontParameters *p)
{
    int b = 0;
    if (strcmp(p->GetFontFileName(), vtkTextureFontManager::sDefaultFreetypeFileName) == 0) b++;

    // TODO - there could be a complex compare needed here - e.g. for a trailing '/', or upper/lower case issues...
    if (strcmp(p->GetFontDirectory(), vtkTextureFontManager::sDefaultFreetypeDirectory) == 0) b++;
    if (b == 2) return 1;
    else return 0;
}


void vtkTextureFontManager::SetToDefaultSettings(vtkFontParameters *p)
{
    p->SetFontFileName(vtkTextureFontManager::sDefaultFreetypeFileName);
    p->SetFontDirectory(vtkTextureFontManager::sDefaultFreetypeDirectory);
}


vtkTextureFont *vtkTextureFontManager::GetTextureFont(vtkFontParameters *p)
{
    vtkTextureFont *f = NULL;

    // if name/dir is null, we copy in the default to FontParamaters    
    if (p->GetFontFileName() == NULL) p->SetFontFileName(vtkTextureFontManager::sDefaultFreetypeFileName);

    // TODO - should a FontFileName w/ a full path be OK, w/o a dir?
    if (p->GetFontDirectory() == NULL) p->SetFontDirectory(vtkTextureFontManager::sDefaultFreetypeDirectory);

    //printf("vtkTextureFontManager::GetTextureFont: request for: %s.  ", p->GetFontFileName());

    int i;
    for (i = 0; i < this->TextureFonts->GetNumberOfItems(); i++) {
        f = (vtkTextureFont *)this->TextureFonts->GetItemAsObject(i);
        if (f == NULL) continue;

        if ((strcmp(f->GetFontParameters()->GetFontFileName(), p->GetFontFileName()) == 0)  &&  
            (strcmp(f->GetFontParameters()->GetFontDirectory(), p->GetFontDirectory()) == 0)  &&  
            (f->GetFontParameters()->GetBlur() == p->GetBlur())  &&  
            (f->GetFontParameters()->GetStyle() == p->GetStyle())  &&
            (f->GetFontParameters()->GetStyleBorderSize() == p->GetStyleBorderSize())  &&
            (f->GetFontParameters()->GetResolution() == p->GetResolution())) {
                //printf("Exists.\n");
                return (f);
            }
    }
    
    f = vtkTextureFont::New();

    // TODO - should we copy over these parameters (we could make a Copy method for this), or SetFontParameters(p) ?
    f->GetFontParameters()->SetFontFileName(p->GetFontFileName());
    f->GetFontParameters()->SetFontDirectory(p->GetFontDirectory());
    f->GetFontParameters()->SetBlur(p->GetBlur());
    f->GetFontParameters()->SetStyle(p->GetStyle());
    f->GetFontParameters()->SetStyleBorderSize(p->GetStyleBorderSize());
    f->GetFontParameters()->SetResolution(p->GetResolution());
    f->CreateFont();

    // v48 - test for errors
    if (f->GetError()) {
        if (this->IsDefaultSettings(p)) {
            vtkErrorMacro("vtkTextureFontManager::GetTextureFont() - error during default font creation.\n");
            return NULL;
        }
        else {
            // try again with the font defaults
            vtkErrorMacro("vtkTextureFontManager::GetTextureFont() - error during font creation - trying default font.\n");
            this->SetToDefaultSettings(p);
            f->Delete();
            return this->GetTextureFont(p);
        }
    }

    if (!f->GetInitialized()) {
        vtkErrorMacro("vtkTextureFontManager::GetTextureFont() - error: font was not initialized.\n");
        return NULL;
    }
    this->TextureFonts->AddItem(f);

    //printf("New.\n");

    return f;
}


char *vtkTextureFontManager::GetDefaultFreetypeDirectory() {
    return sDefaultFreetypeDirectory;
}


void vtkTextureFontManager::SetDefaultFreetypeDirectory(char *dirname) {
    // allocate an extra char, in case we need to add an extra '/'
    sDefaultFreetypeDirectory = (char *)malloc(strlen(dirname) + 2);
    strcpy(sDefaultFreetypeDirectory, dirname);
    int l = strlen(sDefaultFreetypeDirectory);

    if (sDefaultFreetypeDirectory[l - 1] != '/' && sDefaultFreetypeDirectory[l - 1] != '\\')
        sDefaultFreetypeDirectory = strcat(sDefaultFreetypeDirectory, "/");
}


char *vtkTextureFontManager::GetDefaultFreetypeFileName() {
    return sDefaultFreetypeFileName;
}


void vtkTextureFontManager::SetDefaultFreetypeFileName(char *filename) {
    sDefaultFreetypeFileName = strdup(filename);
}


void vtkTextureFontManager::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    /*  TODO - do print
    os << indent << "X Resolution: " << this->XResolution << "\n";
    os << indent << "Y Resolution: " << this->YResolution << "\n";
    */
}
