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
#include "qMRMLSceneModel.h"
#include "qMRMLSortFilterHierarchyProxyModel.h"

// VTK includes
#include <vtkMRMLHierarchyNode.h>

// -----------------------------------------------------------------------------
// qMRMLSortFilterHierarchyProxyModelPrivate

// -----------------------------------------------------------------------------
class qMRMLSortFilterHierarchyProxyModelPrivate
{
public:
  qMRMLSortFilterHierarchyProxyModelPrivate();
};

// -----------------------------------------------------------------------------
qMRMLSortFilterHierarchyProxyModelPrivate::qMRMLSortFilterHierarchyProxyModelPrivate() = default;

// -----------------------------------------------------------------------------
// qMRMLSortFilterHierarchyProxyModel

//------------------------------------------------------------------------------
qMRMLSortFilterHierarchyProxyModel::qMRMLSortFilterHierarchyProxyModel(QObject *vparent)
  : qMRMLSortFilterProxyModel(vparent)
  , d_ptr(new qMRMLSortFilterHierarchyProxyModelPrivate)
{
}

//------------------------------------------------------------------------------
qMRMLSortFilterHierarchyProxyModel::~qMRMLSortFilterHierarchyProxyModel() = default;

//------------------------------------------------------------------------------
qMRMLSortFilterProxyModel::AcceptType qMRMLSortFilterHierarchyProxyModel
::filterAcceptsNode(vtkMRMLNode* node)const
{
  //Q_D(const qMRMLSortFilterHierarchyProxyModel);
  AcceptType res = this->Superclass::filterAcceptsNode(node);
  if (res == Accept || res == AcceptButPotentiallyRejectable)
    {
    return res;
    }
  vtkMRMLHierarchyNode* hNode = vtkMRMLHierarchyNode::SafeDownCast(node);
  if (!hNode)
    {
    return res;
    }
  // Don't show vtkMRMLHierarchyNode if they are tied to a vtkMRMLModelNode
  // The only vtkMRMLHierarchyNode to display are the ones who reference other
  // vtkMRMLHierarchyNode (tree parent) or empty (tree parent to be)
  if (hNode->GetAssociatedNode())
    {
    return RejectButPotentiallyAcceptable;
    }
  return res;
}
