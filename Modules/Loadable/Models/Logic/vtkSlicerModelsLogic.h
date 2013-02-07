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

class vtkMRMLModelNode;
class vtkMRMLStorageNode;
class vtkMRMLTransformNode;
class vtkPolyData;

class VTK_SLICER_MODELS_MODULE_LOGIC_EXPORT vtkSlicerModelsLogic
  : public vtkSlicerModuleLogic
{
  public:
  
  /// The Usual vtk class functions
  static vtkSlicerModelsLogic *New();
  vtkTypeRevisionMacro(vtkSlicerModelsLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// The color logic is used to retrieve the default color node ID for
  /// model nodes.
  virtual void SetColorLogic(vtkMRMLColorLogic* colorLogic);
  vtkGetObjectMacro(ColorLogic, vtkMRMLColorLogic);

  /// 
  /// The currently active mrml volume node 
  vtkGetObjectMacro (ActiveModelNode, vtkMRMLModelNode);
  void SetActiveModelNode (vtkMRMLModelNode *ActiveModelNode);

  ///
  /// Add into the scene a new mrml model node with an existing polydata
  /// A display node is also added into the scene.
  /// \tbd Add a storage node ?
  vtkMRMLModelNode* AddModel(vtkPolyData* polyData = 0);

  /// 
  /// Add into the scene a new mrml model node and
  /// read it's polydata from a specified file
  /// A display node and a storage node are also added into the scene
  vtkMRMLModelNode* AddModel (const char* filename);

  /// 
  /// Create model nodes and
  /// read their polydata from a specified directory
  int AddModels (const char* dirname, const char* suffix );

  /// 
  /// Write model's polydata  to a specified file
  int SaveModel (const char* filename, vtkMRMLModelNode *modelNode);

  /// 
  /// Read in a scalar overlay and add it to the model node
  vtkMRMLStorageNode* AddScalar(const char* filename, vtkMRMLModelNode *modelNode);

  /// Transfor models's polydata
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
  ~vtkSlicerModelsLogic();
  vtkSlicerModelsLogic(const vtkSlicerModelsLogic&);
  void operator=(const vtkSlicerModelsLogic&);

  /// Reimplemented to:
  ///  - make sure the singleton vtkMRMLClipModelsNode is instantiated
  ///  - observe the scene
  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);

  ///
  /// This property controls whether the removal from the scene of a model node
  /// automatically removes its display and storage nodes or not.
  /// This automatic behavior (true by default) is used to prevent the user
  /// from seeing a model (display node) in a 3D view after a model node
  /// has been deleted/removed (delete from a tree view qt widget).
  /// If the nodes were not removed/deleted, the display and storage nodes
  /// would be zombie nodes in the scene with no one pointing on them.
  bool AutoRemoveDisplayAndStorageNodes;

  //
  vtkMRMLModelNode *ActiveModelNode;

  /// Color logic
  vtkMRMLColorLogic* ColorLogic;

};

#endif

