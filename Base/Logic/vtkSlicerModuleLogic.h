/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerModuleLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/
// .NAME vtkSlicerModuleLogic - superclass for slicer module logic classes
// .SECTION Description
// Superclass for all slicer m,odule logic classes
// There must be a corresponding vtkSlicerGUI subclass corresponding 
// to each logic class that handles all GUI interaction (no GUI code
// goes in the logic class).

#ifndef __vtkSlicerModuleLogic_h
#define __vtkSlicerModuleLogic_h

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerLogic.h"
#include "vtkSlicerApplicationLogic.h"

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerModuleLogic : public vtkSlicerLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerModuleLogic *New();
  vtkTypeRevisionMacro(vtkSlicerModuleLogic,vtkSlicerLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get access to overall application state
  vtkGetObjectMacro(ApplicationLogic, vtkSlicerApplicationLogic);
  vtkSetObjectMacro(ApplicationLogic, vtkSlicerApplicationLogic);

  // Description: 
  // The name of the Module
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);
  
  // Description:
  // Set/Get the location of this module (if it was loaded dynamically
  // this could be the full path to the dynamic library, or the full path
  // to the pkgIndex.tcl file if the module is a scripted module, etc).
  vtkSetStringMacro(ModuleLocation);
  vtkGetStringMacro(ModuleLocation);

  // Description:
  // Get the path to the module's resources directory.
  // This is the location on disk where resources (data, support files)
  // associated to this module can be found.
  // The resources directory will be computed from ModuleLocation
  // if it was set (i.e. if Slicer3 is loading this module from a
  // user defined location outside or even inside Slicer3), or relative to
  // Slicer3_HOME otherwise (i.e. if the module was built directory by
  // Slicer3 and not dynamically loaded).
  virtual const char* GetModuleShareDirectory();

  // Description:
  // Get the path to the module's library directory.
  // This is the location on disk where the module library was found. 
  // The lib directory will be computed from ModuleLocation
  // if it was set (i.e. if Slicer3 is loading this module from a
  // user defined location outside or even inside Slicer3), or relative to
  // Slicer3_HOME otherwise (i.e. if the module was built directory by
  // Slicer3 and not dynamically loaded).
  virtual const char* GetModuleLibDirectory();

protected:
  vtkSlicerModuleLogic();
  ~vtkSlicerModuleLogic();
  vtkSlicerModuleLogic(const vtkSlicerModuleLogic&);
  void operator=(const vtkSlicerModuleLogic&);

  vtkSlicerApplicationLogic *ApplicationLogic;
  char *ModuleLocation;
  char *ModuleName;

  virtual int IsModuleShareDirectoryDefined();
  virtual int IsModuleLibDirectoryDefined();

  vtkSetStringMacro(ModuleShareDirectory);
  vtkSetStringMacro(ModuleLibDirectory);

private:

  char *ModuleShareDirectory;
  char *ModuleLibDirectory;
};

#endif

