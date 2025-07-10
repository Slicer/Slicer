/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, EBATINCA, S.L.
  and was funded by by Murat Maga (Seattle Children's Research Institute).

==============================================================================*/

#include "qMRMLSortFilterColorProxyModel.h"

// MRML include
#include "vtkMRMLColorNode.h"
#include "vtkMRMLScene.h"

// MRMLWidgets includes
#include "qMRMLColorModel.h"

// Qt includes
#include <QDebug>
#include <QStandardItem>


// -----------------------------------------------------------------------------
// qMRMLSortFilterColorProxyModelPrivate

// -----------------------------------------------------------------------------
class qMRMLSortFilterColorProxyModelPrivate
{
public:
  qMRMLSortFilterColorProxyModelPrivate();

  public:
    bool ShowEmptyColors;
};

// -----------------------------------------------------------------------------
qMRMLSortFilterColorProxyModelPrivate::qMRMLSortFilterColorProxyModelPrivate()
  : ShowEmptyColors(true)
{
}

//------------------------------------------------------------------------------
qMRMLSortFilterColorProxyModel::qMRMLSortFilterColorProxyModel(QObject* vparent)
 : QSortFilterProxyModel(vparent)
 , d_ptr(new qMRMLSortFilterColorProxyModelPrivate)
{
  this->setDynamicSortFilter(true);
}

//------------------------------------------------------------------------------
qMRMLSortFilterColorProxyModel::~qMRMLSortFilterColorProxyModel() = default;

//-----------------------------------------------------------------------------
vtkMRMLColorNode* qMRMLSortFilterColorProxyModel::mrmlColorNode()const
{
  qMRMLColorModel* model = qobject_cast<qMRMLColorModel*>(this->sourceModel());
  if (!model)
  {
    return nullptr;
  }
  return model->mrmlColorNode();
}

//-----------------------------------------------------------------------------
bool qMRMLSortFilterColorProxyModel::showEmptyColors()const
{
  Q_D(const qMRMLSortFilterColorProxyModel);
  return d->ShowEmptyColors;
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterColorProxyModel::setShowEmptyColors(bool show)
{
  Q_D(qMRMLSortFilterColorProxyModel);
  if (d->ShowEmptyColors == show)
  {
    return;
  }
  d->ShowEmptyColors = show;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
QStandardItem* qMRMLSortFilterColorProxyModel::sourceItem(const QModelIndex& sourceIndex)const
{
  qMRMLColorModel* model = qobject_cast<qMRMLColorModel*>(this->sourceModel());
  if (!model)
  {
    return nullptr;
  }
  return sourceIndex.isValid() ? model->itemFromIndex(sourceIndex) : model->invisibleRootItem();
}
//------------------------------------------------------------------------------
bool qMRMLSortFilterColorProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent)const
{
  QStandardItem* parentItem = this->sourceItem(sourceParent);
  if (!parentItem)
  {
    return false;
  }
  QStandardItem* item = nullptr;

  // Sometimes the row is not complete (DnD), search for a non null item
  for (int childIndex = 0; childIndex < parentItem->columnCount(); ++childIndex)
  {
    item = parentItem->child(sourceRow, childIndex);
    if (item)
    {
      break;
    }
  }
  if (item == nullptr)
  {
    return false;
  }

  qMRMLColorModel* model = qobject_cast<qMRMLColorModel*>(this->sourceModel());
  int colorIndex = model->colorFromItem(item);
  return this->filterAcceptsItem(colorIndex);
}

//------------------------------------------------------------------------------
bool qMRMLSortFilterColorProxyModel::filterAcceptsItem(int colorIndex)const
{
  Q_D(const qMRMLSortFilterColorProxyModel);

  vtkMRMLColorNode* colorNode = this->mrmlColorNode();

  return d->ShowEmptyColors || colorNode->GetColorDefined(colorIndex);
}
