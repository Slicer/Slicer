/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

///  vtkSlicerModelsLogic - slicer logic class for volumes manipulation
/// 
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the volumes


#ifndef __vtkSlicerModelsLogic_h
#define __vtkSlicerModelsLogic_h

#include "vtkSlicerBaseLogic.h"
#include "vtkMRMLAbstractLogic.h"

#include "vtkMRML.h"
#include "vtkMRMLModelNode.h"

class vtkMRMLStorageNode;

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerModelsLogic : public vtkMRMLAbstractLogic 
{
  public:
  
  /// The Usual vtk class functions
  static vtkSlicerModelsLogic *New();
  vtkTypeRevisionMacro(vtkSlicerModelsLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// The currently active mrml volume node 
  vtkGetObjectMacro (ActiveModelNode, vtkMRMLModelNode);
  void SetActiveModelNode (vtkMRMLModelNode *ActiveModelNode);

  /// 
  /// Create new mrml model node and
  /// read it's polydata from a specified file
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

protected:
  vtkSlicerModelsLogic();
  ~vtkSlicerModelsLogic();
  vtkSlicerModelsLogic(const vtkSlicerModelsLogic&);
  void operator=(const vtkSlicerModelsLogic&);

  /// Reimplemented to make sure the singleton vtkMRMLClipModelsNode is
  /// instantiated
  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);

  /// Reimplemented to delete the storage/display nodes when a displayable
  /// node is being removed.
  virtual void OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* /*node*/);

  /// 
  //
  vtkMRMLModelNode *ActiveModelNode;
};

#endif

