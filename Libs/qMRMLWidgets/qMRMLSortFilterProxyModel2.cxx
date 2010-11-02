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
#include <QDebug>
#include <QMap>
#include <QMimeData>
#include <QSharedPointer>
#include <QStack>
#include <QStringList>
#include <QVector>

// qMRML includes
#include "qMRMLSceneModel2.h"
#include "qMRMLUtils.h"

#include "qMRMLSortFilterProxyModel2.h"

// VTK includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// -----------------------------------------------------------------------------
// qMRMLSortFilterProxyModel2Private

// -----------------------------------------------------------------------------
class qMRMLSortFilterProxyModel2Private
{
public:
  qMRMLSortFilterProxyModel2Private();

  QStringList                      NodeTypes;
  bool                             ShowHidden;
  bool                             ShowChildNodeTypes;
  QStringList                      HideChildNodeTypes;
  typedef QPair<QString, QVariant> AttributeType;
  QHash<QString, AttributeType>    Attributes;
};

// -----------------------------------------------------------------------------
qMRMLSortFilterProxyModel2Private::qMRMLSortFilterProxyModel2Private()
{
  this->ShowHidden = false;
  this->ShowChildNodeTypes = true;
}

// -----------------------------------------------------------------------------
// qMRMLSortFilterProxyModel2

//------------------------------------------------------------------------------
qMRMLSortFilterProxyModel2::qMRMLSortFilterProxyModel2(QObject *vparent)
  :QSortFilterProxyModel(vparent)
  , d_ptr(new qMRMLSortFilterProxyModel2Private)
{
}

//------------------------------------------------------------------------------
qMRMLSortFilterProxyModel2::~qMRMLSortFilterProxyModel2()
{
}

// -----------------------------------------------------------------------------
QStandardItem* qMRMLSortFilterProxyModel2::sourceItem(const QModelIndex& sourceIndex)const
{
  qMRMLSceneModel2* sceneModel = qobject_cast<qMRMLSceneModel2*>(this->sourceModel());
  Q_ASSERT(sceneModel);
  qDebug() << sourceIndex << sceneModel->itemFromIndex(sourceIndex);
  return sourceIndex.isValid() ? sceneModel->itemFromIndex(sourceIndex) : sceneModel->invisibleRootItem();
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qMRMLSortFilterProxyModel2::mrmlScene()const
{
  qMRMLSceneModel2* sceneModel = qobject_cast<qMRMLSceneModel2*>(this->sourceModel());
  return sceneModel->mrmlScene();
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qMRMLSortFilterProxyModel2::mrmlNode(const QModelIndex& proxyIndex)const
{
  qMRMLSceneModel2* sceneModel = qobject_cast<qMRMLSceneModel2*>(this->sourceModel());
  return sceneModel->mrmlNodeFromIndex(this->mapToSource(proxyIndex));
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterProxyModel2::addAttribute(const QString& nodeType,
                                              const QString& attributeName,
                                              const QVariant& attributeValue)
{
  Q_D(qMRMLSortFilterProxyModel2);
  if (!d->NodeTypes.contains(nodeType) ||
      (d->Attributes[nodeType].first == attributeName &&
       d->Attributes[nodeType].second == attributeValue))
    {
    return;
    }
  d->Attributes[nodeType] =
    qMRMLSortFilterProxyModel2Private::AttributeType(attributeName, attributeValue);
  this->invalidateFilter();
}

//------------------------------------------------------------------------------
//bool qMRMLSortFilterProxyModel2::filterAcceptsColumn(int source_column, const QModelIndex & source_parent)const;

//------------------------------------------------------------------------------
bool qMRMLSortFilterProxyModel2::filterAcceptsRow(int source_row, const QModelIndex &source_parent)const
{
  Q_D(const qMRMLSortFilterProxyModel2);
  qDebug() << "qMRMLSortFilterProxyModel2::filterAcceptsRow";
  QStandardItem* parentItem = this->sourceItem(source_parent);
  if (parentItem == 0)
    {
    qDebug() << source_parent.isValid();
    Q_ASSERT(parentItem);
    return false;
    }
  QStandardItem* item = parentItem->child(source_row, 0);
  if (item == 0)
    {
    Q_ASSERT(item);
    return false;
    }
  qMRMLSceneModel2* sceneModel = qobject_cast<qMRMLSceneModel2*>(this->sourceModel());
  vtkMRMLNode* node = sceneModel->mrmlNodeFromItem(item);
  if (!node)
    {
    return true;
    }
  if (!d->ShowHidden && node->GetHideFromEditors())
    {
    return false;
    }
  // Accept all the nodes if no type has been set
  if (d->NodeTypes.isEmpty())
    {
    return true;
    }
  foreach(const QString& nodeType, d->NodeTypes)
    {
    // filter by node type
    if (!node->IsA(nodeType.toAscii().data()))
      {
      continue;
      }
    // filter by excluded child node types
    if (!d->ShowChildNodeTypes && nodeType != node->GetClassName())
      {
      continue;
      }
    // filter by HideChildNodeType
    if (d->ShowChildNodeTypes)
      {
      foreach(const QString& hideChildNodeType, d->HideChildNodeTypes)
        {
        if (node->IsA(hideChildNodeType.toAscii().data()))
          {
          return false;
          }
        }
      }
    // filter by attributes
    if (d->Attributes.contains(nodeType))
      {
      QString nodeAttribute =
        node->GetAttribute(d->Attributes[nodeType].first.toLatin1().data());
      if (!nodeAttribute.isEmpty() &&
           nodeAttribute != d->Attributes[nodeType].second.toString())
        {
        return false;
        }
      }
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
QStringList qMRMLSortFilterProxyModel2::hideChildNodeTypes()const
{
  Q_D(const qMRMLSortFilterProxyModel2);
  return d->HideChildNodeTypes;
}

// --------------------------------------------------------------------------
QStringList qMRMLSortFilterProxyModel2::nodeTypes()const
{
  Q_D(const qMRMLSortFilterProxyModel2);
  return d->NodeTypes;
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterProxyModel2::setHideChildNodeTypes(const QStringList& _nodeTypes)
{
  Q_D(qMRMLSortFilterProxyModel2);
  if (_nodeTypes == d->HideChildNodeTypes)
    {
    return;
    }
  d->HideChildNodeTypes = _nodeTypes;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
void qMRMLSortFilterProxyModel2::setNodeTypes(const QStringList& _nodeTypes)
{
  Q_D(qMRMLSortFilterProxyModel2);
  if (d->NodeTypes == _nodeTypes)
    {
    return;
    }
  d->NodeTypes = _nodeTypes;
  this->invalidateFilter();
}

//-----------------------------------------------------------------------------
void qMRMLSortFilterProxyModel2::setShowChildNodeTypes(bool _show)
{
  Q_D(qMRMLSortFilterProxyModel2);
  if (_show == d->ShowChildNodeTypes)
    {
    return;
    }
  d->ShowChildNodeTypes = _show;
  invalidateFilter();
}

// --------------------------------------------------------------------------
void qMRMLSortFilterProxyModel2::setShowHidden(bool enable)
{
  Q_D(qMRMLSortFilterProxyModel2);
  if (enable == d->ShowHidden)
    {
    return;
    }
  d->ShowHidden = enable;
  this->invalidateFilter();
}

// --------------------------------------------------------------------------
bool qMRMLSortFilterProxyModel2::showHidden()const
{
  Q_D(const qMRMLSortFilterProxyModel2);
  return d->ShowHidden;
}

//-----------------------------------------------------------------------------
bool qMRMLSortFilterProxyModel2::showChildNodeTypes()const
{
  Q_D(const qMRMLSortFilterProxyModel2);
  return d->ShowChildNodeTypes;
}
