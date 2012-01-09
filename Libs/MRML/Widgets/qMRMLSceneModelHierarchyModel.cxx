/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes

// qMRML includes
#include "qMRMLSceneModelHierarchyModel.h"
#include "qMRMLSceneDisplayableModel_p.h"

// MRMLLogic includes
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelHierarchyLogic.h>
#include <vtkMRMLModelHierarchyNode.h>

// MRML includes

// VTK includes

//------------------------------------------------------------------------------
class qMRMLSceneModelHierarchyModelPrivate: public qMRMLSceneDisplayableModelPrivate
{
protected:
  Q_DECLARE_PUBLIC(qMRMLSceneModelHierarchyModel);
public:
  typedef qMRMLSceneDisplayableModelPrivate Superclass;
  qMRMLSceneModelHierarchyModelPrivate(qMRMLSceneModelHierarchyModel& object);

  virtual vtkMRMLHierarchyNode* CreateHierarchyNode()const;

};

//------------------------------------------------------------------------------
qMRMLSceneModelHierarchyModelPrivate
::qMRMLSceneModelHierarchyModelPrivate(qMRMLSceneModelHierarchyModel& object)
  : Superclass(object)
{
}

//------------------------------------------------------------------------------
vtkMRMLHierarchyNode* qMRMLSceneModelHierarchyModelPrivate::CreateHierarchyNode()const
{
  return vtkMRMLModelHierarchyNode::New();
}

//----------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLSceneModelHierarchyModel::qMRMLSceneModelHierarchyModel(QObject *vparent)
  :Superclass(new qMRMLSceneModelHierarchyModelPrivate(*this), vparent)
{
  Q_D(qMRMLSceneModelHierarchyModel);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLSceneModelHierarchyModel::~qMRMLSceneModelHierarchyModel()
{
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneModelHierarchyModel::parentNode(vtkMRMLNode* node)const
{
  return vtkMRMLModelHierarchyNode::SafeDownCast(
    this->Superclass::parentNode(node));
}

//------------------------------------------------------------------------------
bool qMRMLSceneModelHierarchyModel::canBeAChild(vtkMRMLNode* node)const
{
  if (!node)
    {
    return false;
    }
  return node->IsA("vtkMRMLModelHierarchyNode") || node->IsA("vtkMRMLModelNode");
}

//------------------------------------------------------------------------------
bool qMRMLSceneModelHierarchyModel::canBeAParent(vtkMRMLNode* node)const
{
  vtkMRMLModelHierarchyNode* hnode = vtkMRMLModelHierarchyNode::SafeDownCast(node);
  if (hnode && hnode->GetModelNodeID() == 0)
    {
    return true;
    }
  return false;
}
