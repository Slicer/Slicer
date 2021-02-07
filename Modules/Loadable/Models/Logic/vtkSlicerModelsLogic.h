/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

///  vtkSlicerModelsLogic - slicer logic class for models manipulation
///
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the models

#ifndef __vtkSlicerModelsLogic_h
#define __vtkSlicerModelsLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerModelsModuleLogicExport.h"

// MRML includes
#include "vtkMRMLStorageNode.h"

// VTK includes
#include <vtkVersion.h>

class vtkMRMLMessageCollection;
class vtkMRMLModelNode;
class vtkMRMLStorageNode;
class vtkMRMLTransformNode;
class vtkAlgorithmOutput;
class vtkPolyData;

class VTK_SLICER_MODELS_MODULE_LOGIC_EXPORT vtkSlicerModelsLogic
  : public vtkSlicerModuleLogic
{
  public:

  /// The Usual vtk class functions
  static vtkSlicerModelsLogic *New();
  vtkTypeMacro(vtkSlicerModelsLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Add into the scene a new mrml model node with an existing polydata
  /// A display node is also added into the scene.
  ///param polyData surface mesh in RAS coordinate system.
  vtkMRMLModelNode* AddModel(vtkPolyData* polyData = nullptr);

  /// Add into the scene a new mrml model node with an existing polydata
  /// A display node is also added into the scene.
  ///param polyData surface mesh algorithm output in RAS coordinate system.
  vtkMRMLModelNode* AddModel(vtkAlgorithmOutput* polyData = nullptr);

  /// Add into the scene a new mrml model node and
  /// read it's polydata from a specified file
  /// A display node and a storage node are also added into the scene
  /// \param coordinateSystem If coordinate system is not specified
  ///   in the file then this coordinate system is used. Default is LPS.
  /// \param userMessages User-displayable warning or error messages can be received if userMessages object is
  ///   specified.
  vtkMRMLModelNode* AddModel(const char* filename, int coordinateSystem = vtkMRMLStorageNode::CoordinateSystemLPS,
    vtkMRMLMessageCollection* userMessages = nullptr);

  /// Create model nodes and
  /// read their polydata from a specified directory
  /// \param coordinateSystem If coordinate system is not specified
  ///   in the file then this coordinate system is used. Default is LPS.
  /// \param userMessages User-displayable warning or error messages can be received if userMessages object is
  ///   specified.
  int AddModels(const char* dirname, const char* suffix, int coordinateSystem = vtkMRMLStorageNode::CoordinateSystemLPS,
    vtkMRMLMessageCollection* userMessages = nullptr);

  /// Write model's polydata  to a specified file
  /// \param coordinateSystem If coordinate system is not specified
  ///   in the file then this coordinate system is used. Default is -1, which means that
  ///   the coordinate system specified in the storage node will be used.
  /// \param userMessages User-displayable warning or error messages can be received if userMessages object is
  ///   specified.
  int SaveModel(const char* filename, vtkMRMLModelNode *modelNode, int coordinateSystem = vtkMRMLStorageNode::CoordinateSystemLPS,
    vtkMRMLMessageCollection* userMessages = nullptr);

  /// Transform models's polydata
  static void TransformModel(vtkMRMLTransformNode *tnode,
                              vtkMRMLModelNode *modelNode,
                              int transformNormals,
                              vtkMRMLModelNode *modelOut);

  /// Iterate through all models in the scene, find all their display nodes
  /// and set their visibility flag to flag. Does not touch model hierarchy
  /// nodes with display nodes
  void SetAllModelsVisibility(int flag);

protected:
  vtkSlicerModelsLogic();
  ~vtkSlicerModelsLogic() override;
  vtkSlicerModelsLogic(const vtkSlicerModelsLogic&);
  void operator=(const vtkSlicerModelsLogic&);

  /// Reimplemented to observe the NodeRemovedEvent scene event.
  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;

  /// Reimplemented to make sure the singleton vtkMRMLClipModelsNode is
  /// instantiated.
  void ObserveMRMLScene() override;

  void OnMRMLSceneEndImport() override;
};

#endif

