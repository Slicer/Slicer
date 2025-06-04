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

// VTK includes
#include <vtkObject.h>

class VTK_MRML_EXPORT vtkMRMLLogic : public vtkObject
{
public:
  /// The Usual vtk class functions
  static vtkMRMLLogic *New();
  vtkTypeMacro(vtkMRMLLogic,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override { this->Superclass::PrintSelf(os, indent); }

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
};

#endif
