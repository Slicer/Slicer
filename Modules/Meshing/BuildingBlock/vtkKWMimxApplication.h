/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxApplication.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.11.4.1 $

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

// .NAME vtkKWMimxApplication - Class containing all application settings interface.
//
// .SECTION Description - vtkKWMimxApplication contains application settings such as
// Font size and color, auto save parameters, Registry settings etc.

#ifndef __vtkKWMimxApplication_h
#define __vtkKWMimxApplication_h

#include "vtkKWApplication.h"
#include "vtkKWRegistryHelper.h"
#include "vtkBuildingBlockWin32Header.h"

class vtkSlicerTheme;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxApplication : public vtkKWApplication
{
public:
  static vtkKWMimxApplication* New();
  vtkTypeRevisionMacro(vtkKWMimxApplication,vtkKWApplication);
 
  virtual void AddAboutCopyrights(ostream &);
  virtual void InstallTheme ( vtkKWTheme *theme );
  virtual void InstallDefaultTheme ( );
  vtkSlicerTheme *GetSlicerTheme ( );
  
  // Description:
  // Set/Get the application font family
  void SetApplicationFontFamily ( const char *family);
  const char *GetApplicationFontFamily ( ) const;

  // Description:
  // Set/Get the application font size
  void SetApplicationFontSize ( const char *size );
  const char *GetApplicationFontSize ( ) const;
  
  // Description:
  // Set/Get the application Auto Save Flag
  bool GetAutoSaveFlag( );
  void SetAutoSaveFlag(bool saveFlag);
  
  // Description:
  // Set/Get the application Auto Save Frequency
  int  GetAutoSaveTime( );
  void SetAutoSaveTime(int saveTime);
  
  // Description:
  // Set/Get the Auto Save Working Directory Flag
  bool GetAutoSaveWorkDirFlag( );
  void SetAutoSaveWorkDirFlag(bool saveFlag);
  
  // Description:
  // Set/Get the Auto Save Directory
  const char *GetAutoSaveDirectory() const;
  void SetAutoSaveDirectory(const char *dirName);
  
  // Description:
  // Set/Get the Current Render Window Text Color
    void SetTextColor(double color[3]);
    double *GetTextColor();
    
    // Description:
  // Set/Get the Current Render Window Background Color
    void SetBackgroundColor(double color[3]);
    double *GetBackgroundColor();
    
    // Description:
  // Set/Get the Auto Save Directory
  const char *GetWorkingDirectory() const;
  void SetWorkingDirectory(const char *dirName);
  void InitializeWorkingDirectory(const char *dirName);
    
    // Description:
  // Get the directory to be used for automated backups of the work
  const char *GetSaveDirectory ( );
  
  // Description:
  // Set/Get the default average edge length for building blocks when created
  double GetAverageElementLength( );
  void SetAverageElementLength(double length);
  
  // Description:
  // Set/Get the default precision for material properties when written to ABAQUS
  int GetABAQUSPrecision( );
  void SetABAQUSPrecision(int precision);
  
  // Description:
  // Load the previous set Registry Settings
    void LoadRegistryApplicationSettings();
  
protected:
  vtkKWMimxApplication();
  ~vtkKWMimxApplication();
  
  vtkSlicerTheme *SlicerTheme;

  char ApplicationFontSize [vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char ApplicationFontFamily [vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  bool AutoSaveFlag;
    int  AutoSaveTime;
    char AutoSaveDirectory[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
    bool AutoSaveWorkDirFlag; 
    char WorkingDirectory[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
    char DateTimeString[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
    char SaveDirectory[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
    double AverageElementLength;
    int ABAQUSPrecision;
    
    double TextColor[3];
    double BackgroundColor[3];
    
private:
  vtkKWMimxApplication(const vtkKWMimxApplication&);   // Not implemented.
  void operator=(const vtkKWMimxApplication&);  // Not implemented.
  
  
};

#endif
