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

class vtkMRMLColorLegendDisplayNode;
class vtkMRMLDisplayableNode;
class vtkMRMLDisplayNode;

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

  /// Create a color legend display node for a displayable node and observe it.
  /// If there is already a color legend display node for the first
  /// (non-color-legend) display node then that is returned.
  /// @param displayableNode - displayable node for which color legend should be created.
  /// @return the newly created (or already existing) color legend display node. Nullptr in case of an error.
  static vtkMRMLColorLegendDisplayNode* AddDefaultColorLegendDisplayNode(vtkMRMLDisplayableNode* displayableNode);

  /// Create a color legend display node for a display node and observe it.
  /// If there is already a color legend display node for that display node then a new node is not created
  /// but the existing node is returned.
  /// @param displayNode - display node for which color legend should be created
  /// @return the newly created (or already existing) color legend display node. Nullptr in case of an error.
  static vtkMRMLColorLegendDisplayNode* AddDefaultColorLegendDisplayNode(vtkMRMLDisplayNode* displayNode);

  /// Return n-th color legend display node.
  /// \param displayableNode displayable node with color legend display nodes
  /// \param n color legend node index
  /// \return already existing color legend display node (or nullptr if there is no such display node)
  /// \sa GetNumberOfColorLegendDisplayNodes
  static vtkMRMLColorLegendDisplayNode* GetNthColorLegendDisplayNode(vtkMRMLDisplayableNode* displayableNode, int n);

  /// Return number of color legend display nodes.
  /// \sa GetNthColorLegendDisplayNode
  static int GetNumberOfColorLegendDisplayNodes(vtkMRMLDisplayableNode* displayableNode);

  /// Get color legend node corresponding to the first valid display node of the input displayable node.
  /// Valid display node is any non-color-legend display node.
  /// If not found then nullptr is returned.
  static vtkMRMLColorLegendDisplayNode* GetColorLegendDisplayNode(vtkMRMLDisplayableNode* displayableNode);

  /// Get color legend node corresponding to a display node.
  /// If the displayable node has a designated primary display node then the corresponding color legend display
  /// node will be returned. Otherwise the first color legend display node will be returned that does not have
  /// any designated primary display node.
  static vtkMRMLColorLegendDisplayNode* GetColorLegendDisplayNode(vtkMRMLDisplayNode* displayNode);

protected:
  vtkSlicerColorLogic();
  ~vtkSlicerColorLogic() override;
  vtkSlicerColorLogic(const vtkSlicerColorLogic&);
  void operator=(const vtkSlicerColorLogic&);

  static vtkMRMLDisplayNode* GetFirstNonColorLegendDisplayNode(vtkMRMLDisplayableNode* displayableNode);

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;

  std::vector<std::string> FindColorFiles(const std::vector<std::string>& directories);
};

#endif

