/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

///  vtkSlicerModelHierarchyLogic - slicer logic class for volumes manipulation
/// 
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the volumes


#ifndef __vtkSlicerModelHierarchyLogic_h
#define __vtkSlicerModelHierarchyLogic_h

#include <stdlib.h>

#include "vtkSlicerBaseLogic.h"
#include "vtkMRMLAbstractLogic.h"

#include "vtkMRML.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelHierarchyNode.h"


class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerModelHierarchyLogic : public vtkMRMLAbstractLogic 
{
  public:
  
  /// The Usual vtk class functions
  static vtkSlicerModelHierarchyLogic *New();
  vtkTypeRevisionMacro(vtkSlicerModelHierarchyLogic,vtkObject);

  
  /// 
  /// Given model id return it's hierarchy 
  /// or NULL if it does not have one
  vtkMRMLModelHierarchyNode* GetModelHierarchyNode(const char *modelNodeID);
  
  /// 
  /// Return number of model that are in a hierarchy 
  int GetNumberOfModelsInHierarchy()
    {
    this->UpdateModelToHierarchyMap();
    return (int)this->ModelHierarchyNodes.size();
    };

//BTX
  /// 
  /// Given model hierarchy node returns all it's children recursively. 
  void GetHierarchyChildrenNodes(vtkMRMLModelHierarchyNode *parentNode,
                                 std::vector< vtkMRMLModelHierarchyNode *> &childrenNodes);
//ETX

  /// 
  /// Call this to update the cache when hierarchy is modified. 
  void HierarchyIsModified()
    {
    ModelHierarchyNodesMTime = 0;
    HierarchyChildrenNodesMTime = 0;
    }
  /// 
  /// Update logic state when MRML scene chenges
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                  unsigned long /*event*/, 
                                  void * /*callData*/ );    
protected:
  vtkSlicerModelHierarchyLogic();
  ~vtkSlicerModelHierarchyLogic();
  vtkSlicerModelHierarchyLogic(const vtkSlicerModelHierarchyLogic&);
  void operator=(const vtkSlicerModelHierarchyLogic&);
  
  /// 
  /// Create model to hierarchy map, 
  /// return number of model hierarchy nodes
  int UpdateModelToHierarchyMap();
  
  void UpdateHierarchyChildrenMap();

  //BTX
  std::map<std::string, vtkMRMLModelHierarchyNode *> ModelHierarchyNodes;
  std::map<std::string, std::vector< vtkMRMLModelHierarchyNode *> > HierarchyChildrenNodes;
  //ETX
  
  unsigned long ModelHierarchyNodesMTime;
  unsigned long HierarchyChildrenNodesMTime;

};

#endif

