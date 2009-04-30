/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxApplication.cxx,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.18.4.2 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "vtkKWMimxApplication.h"

#include "vtkObjectFactory.h"
#include "vtkSlicerTheme.h"

#include <vtksys/SystemTools.hxx>

#include "vtkKWFrameWithLabel.h"
#include "vtkKWRegistryHelper.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWWindowBase.h"

#include "vtkKWMimxMainWindow.h"


//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMimxApplication );
vtkCxxRevisionMacro(vtkKWMimxApplication, "$Revision: 1.18.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxApplication::vtkKWMimxApplication()
{
  vtkKWFrameWithLabel::SetDefaultLabelFontWeightToNormal( );
  this->SlicerTheme = vtkSlicerTheme::New ( );
  
  strcpy ( this->ApplicationFontSize,   "small" );
  strcpy ( this->ApplicationFontFamily, "Arial" );
  
  /* Default Auto Save Settings */
    this->AutoSaveFlag = true;
    this->AutoSaveTime = 5;
    this->AutoSaveWorkDirFlag = true;
    this->AverageElementLength = 1.0;
    this->ABAQUSPrecision = 2;
    strcpy(this->WorkingDirectory,"");
    
    /* Need to add a OS Type Check */
#if defined(WIN32)
    strcpy(this->AutoSaveDirectory,"/tmp");
#else
    strcpy(this->AutoSaveDirectory,"C:/Windows/Temp");
#endif

  /* Set the Default Render Window Colors */
  this->TextColor[0] = this->TextColor[1] = this->TextColor[2] = 1.0;
    this->BackgroundColor[0] = this->BackgroundColor[1] = this->BackgroundColor[2] = 0.0;
    
    std::string tmpDate =  "Mesh-" + vtksys::SystemTools::GetCurrentDateTime("%Y%m%d-%H%M");
    strcpy(this->DateTimeString, tmpDate.c_str()); 
    //std::cout << "Default Dir: " << tmpDate << std::endl;
}

vtkKWMimxApplication::~vtkKWMimxApplication ( ) 
{
  if ( this->SlicerTheme )
    {
    this->SlicerTheme->Delete ( );
    this->SlicerTheme = NULL;
    }
}

void vtkKWMimxApplication::InstallDefaultTheme ( )
{
  InstallTheme( this->SlicerTheme );
}

//---------------------------------------------------------------------------
void vtkKWMimxApplication::InstallTheme ( vtkKWTheme *theme )
{
  if ( theme != NULL ) 
  {
    if ( vtkSlicerTheme::SafeDownCast (theme) == this->SlicerTheme ) {
        this->SetTheme (this->SlicerTheme );
    } else {
        this->SetTheme ( theme );
    }
  }
}

//---------------------------------------------------------------------------
vtkSlicerTheme *vtkKWMimxApplication::GetSlicerTheme ( )
{
  return this->SlicerTheme;
}

//----------------------------------------------------------------------------
void vtkKWMimxApplication::SetApplicationFontFamily ( const char *font)
{
  char localFont[32];
  strcpy(localFont, font);
    
  if ( this->SlicerTheme )
    {
    this->SlicerTheme->SetFontFamily ( localFont );
    this->Script ( "font configure %s -family %s", this->SlicerTheme->GetApplicationFont2(), localFont );
    this->Script ( "font configure %s -family %s", this->SlicerTheme->GetApplicationFont1(), localFont );
    this->Script ( "font configure %s -family %s", this->SlicerTheme->GetApplicationFont0(), localFont );
    strcpy ( this->ApplicationFontFamily, localFont );
    this->SetRegistryValue(1, "Font", "Family", localFont);
    }

}
//----------------------------------------------------------------------------
const char *vtkKWMimxApplication::GetApplicationFontFamily () const
{
  return this->ApplicationFontFamily;
}


//----------------------------------------------------------------------------
void vtkKWMimxApplication::SetApplicationFontSize ( const char *size)
{
  char localSize[32];
  strcpy(localSize, size);
  
  if (this->SlicerTheme)
    {
    vtkSlicerFont *font = this->SlicerTheme->GetSlicerFonts();
    if ( font )
      {
      // check to see if m has a valid value:
      if ( font->IsValidFontSize ( localSize ) )
        {
        int f2 = font->GetFontSize2( localSize );
        int f1 = font->GetFontSize1( localSize );
        int f0 = font->GetFontSize0( localSize );
        
        this->Script ( "font configure %s -size %d", this->SlicerTheme->GetApplicationFont2(), f2);
        this->Script ( "font configure %s -size %d", this->SlicerTheme->GetApplicationFont1(), f1);
        this->Script ( "font configure %s -size %d", this->SlicerTheme->GetApplicationFont0(), f0);
        
        strcpy (this->ApplicationFontSize, localSize );
        this->SetRegistryValue(1, "Font", "Size", localSize);
        }
      }
    }    
}
//----------------------------------------------------------------------------
const char *vtkKWMimxApplication::GetApplicationFontSize () const
{
  return this->ApplicationFontSize;
}


//----------------------------------------------------------------------------
void vtkKWMimxApplication::AddAboutCopyrights(ostream &os)
{
  os << "IA-FEMesh is developed by the ";
  os << "Musculoskeletal Imaging, Modelling and Experimentation (MIMX) Program" << std::endl;
  os << "Center for Computer Aided Design" << std::endl;
  os << "The University of Iowa" << std::endl;
  os << "Iowa City, IA 52242" << std::endl;

  os << "See http://www.ccad.uiowa.edu/mimx/ for Copyright Information" << std::endl << std::endl;
  os << "IA-FEMesh is built upon:" << std::endl;
  os << "  Slicer3 http://www.na-mic.org/" << std::endl;
  os << "  VTK http://www.vtk.org/copyright.php" << std::endl;
  os << "  ITK http://www.itk.org/HTML/Copyright.htm" << std::endl;
  os << "  KWWidgets http://www.kitware.com/Copyright.htm" << std::endl;
  os << "  Tcl/Tk http://www.tcl.tk" << std::endl << std::endl;
  os << "The MIMX Program gratefully acknowledges financial support provided by awards" << std::endl;
  os << "R21EB001501 and R01EB005973 from the National Institute of Biomedical Imaging" << std::endl;
  os << "and Bioengineering, National Institutes of Health." << std::endl;

#if 0
  // example of the extra detail needed:
  //
     << tcl_major << "." << tcl_minor << "." << tcl_patch_level << endl
     << "  - Copyright (c) 1989-1994 The Regents of the University of "
     << "California." << endl
     << "  - Copyright (c) 1994 The Australian National University." << endl
     << "  - Copyright (c) 1994-1998 Sun Microsystems, Inc." << endl
     << "  - Copyright (c) 1998-2000 Ajuba Solutions." << endl;
#endif
}

//----------------------------------------------------------------------------
bool vtkKWMimxApplication::GetAutoSaveFlag( )
{
  return this->AutoSaveFlag;
}

//----------------------------------------------------------------------------
void vtkKWMimxApplication::SetAutoSaveFlag(bool saveFlag)
{
  this->AutoSaveFlag = saveFlag;
  this->GetApplication()->SetRegistryValue(1, "AutoSave", "SaveFlag", "%d", this->AutoSaveFlag);
  
}

//----------------------------------------------------------------------------
int vtkKWMimxApplication::GetAutoSaveTime( )
{
  return this->AutoSaveTime;
}

//----------------------------------------------------------------------------
void vtkKWMimxApplication::SetAutoSaveTime(int saveTime)
{
  this->AutoSaveTime = saveTime;
  this->GetApplication()->SetRegistryValue(1, "AutoSave", "SaveFrequency", "%d", this->AutoSaveTime);
}

//----------------------------------------------------------------------------
double vtkKWMimxApplication::GetAverageElementLength( )
{
  return this->AverageElementLength;
}

//----------------------------------------------------------------------------
void vtkKWMimxApplication::SetAverageElementLength(double length)
{
  this->AverageElementLength = length;
  this->GetApplication()->SetRegistryValue(1, "MeshProperties", "AverageElementLength", "%f", this->AverageElementLength);
}

//----------------------------------------------------------------------------
int vtkKWMimxApplication::GetABAQUSPrecision( )
{
  return this->ABAQUSPrecision;
}

//----------------------------------------------------------------------------
void vtkKWMimxApplication::SetABAQUSPrecision(int precision)
{
  this->ABAQUSPrecision = precision;
  this->GetApplication()->SetRegistryValue(1, "MeshProperties", "ABAQUSPrecision", "%d", this->ABAQUSPrecision);
}

//----------------------------------------------------------------------------
bool vtkKWMimxApplication::GetAutoSaveWorkDirFlag( )
{
  return this->AutoSaveWorkDirFlag;
}

//----------------------------------------------------------------------------
void vtkKWMimxApplication::SetAutoSaveWorkDirFlag(bool saveFlag)
{
  this->AutoSaveWorkDirFlag = saveFlag;
  this->GetApplication()->SetRegistryValue(1, "AutoSave", "UseWorkingDirectory", "%d", this->AutoSaveWorkDirFlag);
}

//----------------------------------------------------------------------------
const char *vtkKWMimxApplication::GetAutoSaveDirectory() const
{
  return this->AutoSaveDirectory;
}

//----------------------------------------------------------------------------
void vtkKWMimxApplication::SetAutoSaveDirectory(const char *dirName)
{
  strcpy(this->AutoSaveDirectory, dirName);
  this->GetApplication()->SetRegistryValue(1, "AutoSave", "SaveDirectory", this->AutoSaveDirectory);
}

//----------------------------------------------------------------------------------------------
void vtkKWMimxApplication::SetTextColor(double color[3])
{
  this->TextColor[0] = color[0];
  this->TextColor[1] = color[1];
  this->TextColor[2] = color[2];
  
  this->SaveColorRegistryValue(1, "ViewerText", this->TextColor);
  
}

//----------------------------------------------------------------------------------------------
double *vtkKWMimxApplication::GetTextColor( )
{
  return this->TextColor;
}

//----------------------------------------------------------------------------------------------
void vtkKWMimxApplication::SetBackgroundColor(double color[3])
{
  this->BackgroundColor[0] = color[0];
  this->BackgroundColor[1] = color[1];
  this->BackgroundColor[2] = color[2];
  
  this->SaveColorRegistryValue(1, "ViewerBackground", this->BackgroundColor);
}

//----------------------------------------------------------------------------------------------
double *vtkKWMimxApplication::GetBackgroundColor( )
{
  return this->BackgroundColor;
}

//----------------------------------------------------------------------------
const char *vtkKWMimxApplication::GetWorkingDirectory() const
{
  return this->WorkingDirectory;
}

//----------------------------------------------------------------------------
void vtkKWMimxApplication::SetWorkingDirectory(const char *dirName)
{
  strcpy(this->WorkingDirectory, dirName);
  
}

//----------------------------------------------------------------------------
void vtkKWMimxApplication::InitializeWorkingDirectory(const char *dirName)
{
  if ( strlen(this->WorkingDirectory) == 0 ) 
  {
    SetWorkingDirectory(dirName);
  }
}

//----------------------------------------------------------------------------
void vtkKWMimxApplication::LoadRegistryApplicationSettings()
{
  int registryFlag;
  
  registryFlag = this->HasRegistryValue (1, "AutoSave", "SaveFlag");
  if ( registryFlag )
  {
  this->AutoSaveFlag = (this->GetIntRegistryValue(1, "AutoSave", "SaveFlag"))?true:false;
  }
  
  registryFlag = this->HasRegistryValue (1, "AutoSave", "SaveFrequency");
  if ( registryFlag )
  {
    this->AutoSaveTime = this->GetIntRegistryValue(1, "AutoSave", "SaveFrequency");
  }
  
  registryFlag = this->HasRegistryValue (1, "AutoSave", "UseWorkingDirectory");
  if ( registryFlag )
  {
  this->AutoSaveWorkDirFlag = (this->GetIntRegistryValue(1, "AutoSave", "UseWorkingDirectory"))?true:false;
  }
  
  registryFlag = this->HasRegistryValue (1, "AutoSave", "SaveDirectory");
  if ( registryFlag )
  {
    this->GetRegistryValue(1, "AutoSave", "SaveDirectory", this->AutoSaveDirectory);
  }
  
  registryFlag = this->HasRegistryValue (1, "ViewerBackground", "Colors");
  if ( registryFlag )
  {
    this->RetrieveColorRegistryValue(1, "ViewerBackground", this->BackgroundColor);
  }
  
  registryFlag = this->HasRegistryValue (1, "ViewerText", "Colors");
  if ( registryFlag )
  {
    this->RetrieveColorRegistryValue(1, "ViewerText", this->TextColor);
  }
  
  registryFlag = this->HasRegistryValue (1, "Font", "Family");
  if ( registryFlag )
  {
    this->GetRegistryValue(1, "Font", "Family", this->ApplicationFontFamily);
  }
  
  registryFlag = this->HasRegistryValue (1, "Font", "Size");
  if ( registryFlag )
  {
    this->GetRegistryValue(1, "Font", "Size", this->ApplicationFontSize);
  } 
  
  registryFlag = this->HasRegistryValue (1, "MeshProperties", "ABAQUSPrecision");
  if ( registryFlag )
  {
    this->ABAQUSPrecision = static_cast<int> ( 
          this->GetIntRegistryValue(1, "MeshProperties", "ABAQUSPrecision") );
  } 
  
  registryFlag = this->HasRegistryValue (1, "MeshProperties", "AverageElementLength");
  if ( registryFlag )
  {
    this->AverageElementLength = static_cast<double> ( 
          this->GetFloatRegistryValue(1, "MeshProperties", "AverageElementLength") );
  } 
  
}

//----------------------------------------------------------------------------
const char *vtkKWMimxApplication::GetSaveDirectory()
{
  if ( this->AutoSaveWorkDirFlag )
    sprintf(this->SaveDirectory, "%s/%s", this->WorkingDirectory, this->DateTimeString);
  else
    sprintf(this->SaveDirectory, "%s/%s", this->AutoSaveDirectory, this->DateTimeString);
  
  return this->SaveDirectory;
}
//-----------------------------------------------------------------------------




