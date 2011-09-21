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

// CTK includes
#include <ctkLogger.h>

// VTK includes
#include <vtkMRMLHierarchyNode.h>

static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLSortFilterHierarchyProxyModel");

// -----------------------------------------------------------------------------
// qMRMLSortFilterHierarchyProxyModelPrivate

// -----------------------------------------------------------------------------
class qMRMLSortFilterHierarchyProxyModelPrivate
{
public:
  qMRMLSortFilterHierarchyProxyModelPrivate();
};

// -----------------------------------------------------------------------------
qMRMLSortFilterHierarchyProxyModelPrivate::qMRMLSortFilterHierarchyProxyModelPrivate()
{
}

// -----------------------------------------------------------------------------
// qMRMLSortFilterHierarchyProxyModel

//------------------------------------------------------------------------------
qMRMLSortFilterHierarchyProxyModel::qMRMLSortFilterHierarchyProxyModel(QObject *vparent)
  : qMRMLSortFilterProxyModel(vparent)
  , d_ptr(new qMRMLSortFilterHierarchyProxyModelPrivate)
{
}

//------------------------------------------------------------------------------
qMRMLSortFilterHierarchyProxyModel::~qMRMLSortFilterHierarchyProxyModel()
{
}

//------------------------------------------------------------------------------
bool qMRMLSortFilterHierarchyProxyModel
::filterAcceptsRow(int source_row, const QModelIndex &source_parent)const
{
  //Q_D(const qMRMLSortFilterHierarchyProxyModel);
  bool res = this->Superclass::filterAcceptsRow(source_row, source_parent);
  if (!res)
    {
    return res;
    }
  // shouldn't fail because Superclass::filterAcceptsRow returned true 
  QStandardItem* parentItem = this->sourceItem(source_parent);
  Q_ASSERT(parentItem);
  // shouldn't fail because Superclass::filterAcceptsRow returned true
  QStandardItem* item = 0;
  // Sometimes the row is not complete, search for a non null item
  for (int childIndex = 0; childIndex < parentItem->columnCount(); ++childIndex)
    {
    item = parentItem->child(source_row, childIndex);
    if (item)
      {
      break;
      }
    }
  Q_ASSERT(item);
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(
    this->sourceModel());
  vtkMRMLNode* node = sceneModel->mrmlNodeFromItem(item);
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
    return false;
    }
  return res;
}
