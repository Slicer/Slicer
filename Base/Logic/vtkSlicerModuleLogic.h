/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
///  vtkSlicerModuleLogic - superclass for slicer module logic classes
/// 
/// Superclass for all slicer m,odule logic classes
/// There must be a corresponding vtkSlicerGUI subclass corresponding 
/// to each logic class that handles all GUI interaction (no GUI code
/// goes in the logic class).

#ifndef __vtkSlicerModuleLogic_h
#define __vtkSlicerModuleLogic_h

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include <vtkMRMLAbstractLogic.h>
#include "vtkSlicerApplicationLogic.h"

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerModuleLogic : public vtkMRMLAbstractLogic
{
  public:
  
  /// The Usual vtk class functions
  static vtkSlicerModuleLogic *New();
  vtkTypeRevisionMacro(vtkSlicerModuleLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// Get access to overall application state
  vtkGetObjectMacro(ApplicationLogic, vtkSlicerApplicationLogic);
  vtkSetObjectMacro(ApplicationLogic, vtkSlicerApplicationLogic);

  ///  
  /// The name of the Module
  vtkGetStringMacro(ModuleName);
  vtkSetStringMacro(ModuleName);
  
  /// 
  /// Set/Get the location of this module (if it was loaded dynamically
  /// this could be the full path to the dynamic library, or the full path
  /// to the pkgIndex.tcl file if the module is a scripted module, etc).
  vtkGetStringMacro(ModuleLocation);
  vtkSetStringMacro(ModuleLocation);

  /// 
  /// Get the path to the module's resources directory.
  /// This is the location on disk where resources (data, support files)
  /// associated to this module can be found.
  /// The resources directory will be computed from ModuleLocation
  /// if it was set (i.e. if Slicer3 is loading this module from a
  /// user defined location outside or even inside Slicer3), or relative to
  /// Slicer_HOME otherwise (i.e. if the module was built directory by
  /// Slicer3 and not dynamically loaded).
  virtual const char* GetModuleShareDirectory();

  /// 
  /// Get the path to the module's library directory.
  /// This is the location on disk where the module library was found. 
  /// The lib directory will be computed from ModuleLocation
  /// if it was set (i.e. if Slicer3 is loading this module from a
  /// user defined location outside or even inside Slicer3), or relative to
  /// Slicer_HOME otherwise (i.e. if the module was built directory by
  /// Slicer3 and not dynamically loaded).
  virtual const char* GetModuleLibDirectory();

  /// Load any default parameter sets into the specified scene
  static void LoadDefaultParameterSets(vtkMRMLScene *);

protected:

  vtkSlicerModuleLogic();
  virtual ~vtkSlicerModuleLogic();

  vtkSlicerApplicationLogic * ApplicationLogic;
  char *                      ModuleLocation;
  char *                      ModuleName;

  virtual int IsModuleShareDirectoryDefined();
  virtual int IsModuleLibDirectoryDefined();

  vtkSetStringMacro(ModuleShareDirectory);
  vtkSetStringMacro(ModuleLibDirectory);

private:

  vtkSlicerModuleLogic(const vtkSlicerModuleLogic&); // Not implemented
  void operator=(const vtkSlicerModuleLogic&);       // Not implemented

  char *ModuleShareDirectory;
  char *ModuleLibDirectory;
};

#endif

