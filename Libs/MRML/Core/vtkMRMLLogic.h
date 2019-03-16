/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

#ifndef __vtkMRMLLogic_h
#define __vtkMRMLLogic_h

// MRML includes
#include "vtkMRML.h"
class vtkMRMLScene;

// VTK includes
#include <vtkObject.h>

/// \brief Class that manages adding and deleting of observers with events.
///
/// Class that manages adding and deleting of obserevers with events
/// This class keeps track of obserevers and events added to each vtk object.
/// It caches tags returned by AddObserver method so that observers can be removed properly.
class VTK_MRML_EXPORT vtkMRMLLogic : public vtkObject
{
public:
  /// The Usual vtk class functions
  static vtkMRMLLogic *New();
  vtkTypeMacro(vtkMRMLLogic,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override { this->Superclass::PrintSelf(os, indent); }

  vtkMRMLScene* GetScene() {return this->Scene;};
  void SetScene(vtkMRMLScene* scene) {this->Scene = scene;};

  void RemoveUnreferencedStorageNodes();

  void RemoveUnreferencedDisplayNodes();

  /// Get application home directory.
  /// The path is retrieved from the environment variable defined by MRML_APPLICATION_HOME_DIR_ENV.
  static std::string GetApplicationHomeDirectory();

  /// Get application share subdirectory.
  /// The path is constructed by concatenating application home directory and MRML_APPLICATION_SHARE_SUBDIR.
  static std::string GetApplicationShareDirectory();

protected:
  vtkMRMLLogic();
  ~vtkMRMLLogic() override;
  vtkMRMLLogic(const vtkMRMLLogic&);
  void operator=(const vtkMRMLLogic&);

  vtkMRMLScene *Scene;
};

#endif
