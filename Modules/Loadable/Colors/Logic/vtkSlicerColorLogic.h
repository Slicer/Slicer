/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

///  vtkSlicerColorLogic - slicer logic class for color manipulation
///
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the colors

#ifndef __vtkSlicerColorLogic_h
#define __vtkSlicerColorLogic_h

// MRMLLogic includes
#include <vtkMRMLColorLogic.h>
#include "vtkSlicerColorsModuleLogicExport.h"

class VTK_SLICER_COLORS_MODULE_LOGIC_EXPORT vtkSlicerColorLogic
  : public vtkMRMLColorLogic
{
  public:

  /// The Usual vtk class functions
  static vtkSlicerColorLogic *New();
  vtkTypeMacro(vtkSlicerColorLogic,vtkMRMLColorLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Return a default color node id for a label map
  const char * GetDefaultLabelMapColorNodeID() override;

  /// Return a default color node id for the editor
  const char * GetDefaultEditorColorNodeID() override;

  /// Return a default color node id for the charts
  const char * GetDefaultChartColorNodeID() override;

  /// look for color files in the Base/Logic/Resources/ColorFiles directory and
  /// put their names in the ColorFiles list. Look in any user defined color
  /// files paths and put them in the UserColorFiles list.
  std::vector<std::string> FindDefaultColorFiles() override;
  std::vector<std::string> FindUserColorFiles() override;

protected:
  vtkSlicerColorLogic();
  ~vtkSlicerColorLogic() override;
  vtkSlicerColorLogic(const vtkSlicerColorLogic&);
  void operator=(const vtkSlicerColorLogic&);
  std::vector<std::string> FindColorFiles(const std::vector<std::string>& directories);
};

#endif

