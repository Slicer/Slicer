/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QStandardItem>

// qMRML includes
#include "qMRMLSceneModel.h"
#include "qMRMLSortFilterModelHierarchyProxyModel.h"

// CTK includes
#include <ctkLogger.h>

// VTK includes
#include <vtkMRMLModelHierarchyNode.h>

static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLSortFilterModelHierarchyProxyModel");

// -----------------------------------------------------------------------------
// qMRMLSortFilterModelHierarchyProxyModelPrivate

// -----------------------------------------------------------------------------
class qMRMLSortFilterModelHierarchyProxyModelPrivate
{
public:
  qMRMLSortFilterModelHierarchyProxyModelPrivate();
};

// -----------------------------------------------------------------------------
qMRMLSortFilterModelHierarchyProxyModelPrivate::qMRMLSortFilterModelHierarchyProxyModelPrivate()
{
}

// -----------------------------------------------------------------------------
// qMRMLSortFilterModelHierarchyProxyModel

//------------------------------------------------------------------------------
qMRMLSortFilterModelHierarchyProxyModel::qMRMLSortFilterModelHierarchyProxyModel(QObject *vparent)
  : qMRMLSortFilterProxyModel(vparent)
  , d_ptr(new qMRMLSortFilterModelHierarchyProxyModelPrivate)
{
}

//------------------------------------------------------------------------------
qMRMLSortFilterModelHierarchyProxyModel::~qMRMLSortFilterModelHierarchyProxyModel()
{
}

//------------------------------------------------------------------------------
bool qMRMLSortFilterModelHierarchyProxyModel
::filterAcceptsRow(int source_row, const QModelIndex &source_parent)const
{
  //Q_D(const qMRMLSortFilterModelHierarchyProxyModel);
  bool res = this->Superclass::filterAcceptsRow(source_row, source_parent);
  if (!res)
    {
    return res;
    }
  // shouldn't fail because Superclass::filterAcceptsRow returned true 
  QStandardItem* parentItem = this->sourceItem(source_parent);
  Q_ASSERT(parentItem);
  // shouldn't fail because Superclass::filterAcceptsRow returned true
  QStandardItem* item = parentItem->child(source_row, 0);
  Q_ASSERT(item);
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(
    this->sourceModel());
  vtkMRMLModelHierarchyNode* node = vtkMRMLModelHierarchyNode::SafeDownCast(
    sceneModel->mrmlNodeFromItem(item));
  if (!node)
    {
    return res;
    }
  // Don't show vtkMRMLModelHierarchyNode if they are tied to a vtkMRMLModelNode
  // The only vtkMRMLModelHierarchyNode to display are the ones who reference other
  // vtkMRMLModelHierarchyNode (tree parent) or empty (tree parent to be)
  if (node->GetModelNode())
    {
    return false;
    }
  return res;
}
