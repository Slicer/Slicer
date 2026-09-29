/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==============================================================================*/

#ifndef __vtkMRMLNodeCleanup_h
#define __vtkMRMLNodeCleanup_h

#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"

#include <vtkSmartPointer.h>

/// Remove a node from the specified scene when this guard leaves scope.
/// The scene and node are kept alive until the guard is destroyed. A node
/// removed earlier or moved to another scene is left alone.
class vtkMRMLNodeCleanup
{
public:
  vtkMRMLNodeCleanup(vtkMRMLScene* scene, vtkMRMLNode* node)
    : Scene(scene)
    , Node(node)
  {
  }

  vtkMRMLNodeCleanup(const vtkMRMLNodeCleanup&) = delete;
  vtkMRMLNodeCleanup& operator=(const vtkMRMLNodeCleanup&) = delete;

  ~vtkMRMLNodeCleanup()
  {
    if (this->Scene && this->Node && this->Node->GetScene() == this->Scene.GetPointer())
    {
      this->Scene->RemoveNode(this->Node);
    }
  }

private:
  vtkSmartPointer<vtkMRMLScene> Scene;
  vtkSmartPointer<vtkMRMLNode> Node;
};

#endif
