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

// qMRML includes
#include "qMRMLSceneTractographyDisplayModel.h"
#include "qMRMLSceneDisplayableModel_p.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLFiberBundleNode.h>
#include <vtkMRMLFiberBundleDisplayNode.h>
#include <vtkMRMLFiberBundleLineDisplayNode.h>
#include <vtkMRMLFiberBundleTubeDisplayNode.h>
#include <vtkMRMLFiberBundleGlyphDisplayNode.h>


class qMRMLSceneTractographyDisplayModelPrivate: public qMRMLSceneDisplayableModelPrivate
{
protected:
  Q_DECLARE_PUBLIC(qMRMLSceneTractographyDisplayModel);
public:
  typedef qMRMLSceneDisplayableModelPrivate Superclass;
  qMRMLSceneTractographyDisplayModelPrivate(qMRMLSceneTractographyDisplayModel& object);
  virtual void init();

  int ColorColumn;
  int LineVisibilityColumn;
  int TubeVisibilityColumn;
  int TubeIntersectionVisibilityColumn;
  int GlyphVisibilityColumn;

};

//------------------------------------------------------------------------------
qMRMLSceneTractographyDisplayModelPrivate
::qMRMLSceneTractographyDisplayModelPrivate(qMRMLSceneTractographyDisplayModel& object)
  : Superclass(object)
{
  this->LineVisibilityColumn = -1;
  this->TubeVisibilityColumn = -1;
  this->TubeIntersectionVisibilityColumn = -1;
  this->GlyphVisibilityColumn = -1;
  this->ColorColumn = -1;
}

//------------------------------------------------------------------------------
void qMRMLSceneTractographyDisplayModelPrivate::init()
{
  Q_Q(qMRMLSceneTractographyDisplayModel);

  //q->setVisibilityColumn(q->nameColumn());
}


//----------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLSceneTractographyDisplayModel::qMRMLSceneTractographyDisplayModel(QObject *vparent)
  :Superclass(new qMRMLSceneTractographyDisplayModelPrivate(*this), vparent)
{
  Q_D(qMRMLSceneTractographyDisplayModel);
  d->init();

  this->setListenNodeModifiedEvent(true);
  this->setVisibilityColumn(-1);
  this->setOpacityColumn(-1);
  this->setCheckableColumn(-1);

  this->setNameColumn(0);
  this->setLineVisibilityColumn(1);
  this->setTubeVisibilityColumn(2);
  this->setTubeIntersectionVisibilityColumn(3);
  this->setGlyphVisibilityColumn(4);
  this->setColorColumn(5);

  this->setColumnCount(6);
  this->setHorizontalHeaderLabels(
    QStringList() << "Name" << "Lines" << "Tubes" << "Tubes Slice" << "Glyphs" << "Tubes");

}

//------------------------------------------------------------------------------
qMRMLSceneTractographyDisplayModel::qMRMLSceneTractographyDisplayModel(
  qMRMLSceneTractographyDisplayModelPrivate* pimpl, QObject *vparent)
  :Superclass(pimpl, vparent)
{
}

//------------------------------------------------------------------------------
qMRMLSceneTractographyDisplayModel::~qMRMLSceneTractographyDisplayModel()
{
}


/**
//------------------------------------------------------------------------------
void qMRMLSceneTractographyDisplayModel::observeNode(vtkMRMLNode* node)
{
  this->Superclass::observeNode(node);

  if (node->IsA("vtkMRMLDisplayableNode"))
    {
    qvtkConnect(node, vtkMRMLDisplayableNode::DisplayModifiedEvent,
                this, SLOT(onMRMLNodeModified(vtkObject*)));
    }
}
**/
//------------------------------------------------------------------------------
QFlags<Qt::ItemFlag> qMRMLSceneTractographyDisplayModel::nodeFlags(vtkMRMLNode* node, int column)const
{
  Q_D(const qMRMLSceneTractographyDisplayModel);
  QFlags<Qt::ItemFlag> flags = this->Superclass::nodeFlags(node, column);
  if (column == this->lineVisibilityColumn() &&
      d->displayNode(node) != 0)
    {
    flags |= Qt::ItemIsEditable;
    }
  if (column == this->tubeVisibilityColumn() &&
      d->displayNode(node) != 0)
    {
    flags |= Qt::ItemIsEditable;
    }
  if (column == this->tubeIntersectionVisibilityColumn() &&
      d->displayNode(node) != 0)
    {
    flags |= Qt::ItemIsEditable;
    }
  if (column == this->glyphVisibilityColumn() &&
      d->displayNode(node) != 0)
    {
    flags |= Qt::ItemIsEditable;
    }
  if (column == this->colorColumn() &&
      d->displayNode(node) != 0)
    {
    flags |= Qt::ItemIsEditable;
    }

  return flags;
}


//------------------------------------------------------------------------------
void qMRMLSceneTractographyDisplayModel
::updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  Q_D(qMRMLSceneTractographyDisplayModel);

  vtkMRMLFiberBundleNode *fbNode = vtkMRMLFiberBundleNode::SafeDownCast(node);
  if (!fbNode)
    {
    return;
    }

  if (column == this->colorColumn())
    {
    vtkMRMLFiberBundleDisplayNode* displayNode = fbNode->GetTubeDisplayNode();
    if (displayNode)
      {
      double* rgbF = displayNode->GetColor();
      QColor color = QColor::fromRgbF(rgbF[0], rgbF[1], rgbF[2],
                                      displayNode->GetOpacity());
      item->setData(color, Qt::DecorationRole);
      item->setToolTip("Color");
      if (displayNode->GetColorMode() == vtkMRMLFiberBundleDisplayNode::colorModeSolid)
        {
        item->setEnabled(true);
        }
      else
        {
        item->setEnabled(false);
        }
      }
    }
  else if (column == this->lineVisibilityColumn())
    {
    vtkMRMLFiberBundleDisplayNode* displayNode = fbNode->GetLineDisplayNode();
    if (displayNode)
      {
      this->updateVilibilityFromNode(item, displayNode, column);
      }
  }
  else if (column == this->tubeVisibilityColumn())
    {
    vtkMRMLFiberBundleDisplayNode* displayNode = fbNode->GetTubeDisplayNode();
    if (displayNode)
      {
      this->updateVilibilityFromNode(item, displayNode, column);
      }
    }
  else if (column == this->tubeIntersectionVisibilityColumn())
    {
    vtkMRMLFiberBundleDisplayNode* displayNode = fbNode->GetTubeDisplayNode();
    if (displayNode)
      {
      this->updateVilibilityFromNode(item, displayNode, column, true);
      }
    }
  else if (column == this->glyphVisibilityColumn())
    {
    vtkMRMLFiberBundleDisplayNode* displayNode = fbNode->GetGlyphDisplayNode();
    if (displayNode)
      {
      this->updateVilibilityFromNode(item, displayNode, column);
      }
    }
  else
    {
    this->Superclass::updateItemDataFromNode(item, node, column);
    }
}


void qMRMLSceneTractographyDisplayModel
::updateVilibilityFromNode(QStandardItem* item, vtkMRMLNode* node, int column, bool slice)
{
  Q_D(qMRMLSceneTractographyDisplayModel);

  vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(node);
  int visible = -1;
  if (displayNode)
    {
    if (slice)
      {
      visible = displayNode->GetSliceIntersectionVisibility();
      }
    else
      {
      visible = displayNode->GetVisibility();
      }
    }
  switch (visible)
    {
    case 0:
      // It should be fine to set the icon even if it is the same, but due
      // to a bug in Qt (http://bugreports.qt.nokia.com/browse/QTBUG-20248),
      // it would fire a superflous itemChanged() signal.
      if (item->icon().cacheKey() != d->HiddenIcon.cacheKey())
        {
        item->setIcon(d->HiddenIcon);
        }
      break;
    case 1:
      // It should be fine to set the icon even if it is the same, but due
      // to a bug in Qt (http://bugreports.qt.nokia.com/browse/QTBUG-20248),
      // it would fire a superflous itemChanged() signal.
      if (item->icon().cacheKey() != d->VisibleIcon.cacheKey())
        {
        item->setIcon(d->VisibleIcon);
        }
      break;
    case 2:
      // It should be fine to set the icon even if it is the same, but due
      // to a bug in Qt (http://bugreports.qt.nokia.com/browse/QTBUG-20248),
      // it would fire a superflous itemChanged() signal.
      if (item->icon().cacheKey() != d->PartiallyVisibleIcon.cacheKey())
        {
        item->setIcon(d->PartiallyVisibleIcon);
        }
      break;
    default:
      break;
    }
}

//------------------------------------------------------------------------------
void qMRMLSceneTractographyDisplayModel
::updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item)
{
 
  Q_D(qMRMLSceneTractographyDisplayModel);

  vtkMRMLFiberBundleNode *fbNode = vtkMRMLFiberBundleNode::SafeDownCast(node);
  if (!fbNode)
    {
    return;
    }

  if (item->column() == this->colorColumn())
    {
    QColor color = item->data(Qt::DecorationRole).value<QColor>();
    // Invalid color can happen when the item hasn't been initialized yet
    if (color.isValid())
      {
      vtkMRMLFiberBundleDisplayNode* displayNode = fbNode->GetTubeDisplayNode();
      if (displayNode)
        {
        int wasModifying = displayNode->StartModify();
        // QColor looses precision, don't change color/opacity if not "really"
        // changed.
        QColor oldColor = QColor::fromRgbF(displayNode->GetColor()[0],
                                           displayNode->GetColor()[1],
                                           displayNode->GetColor()[2],
                                           displayNode->GetOpacity());
        if (oldColor != color)
          {
          displayNode->SetColor(color.redF(), color.greenF(), color.blueF());
          displayNode->SetOpacity(color.alphaF());
          }
        //displayNode->SetColorModeToSolid();
        displayNode->EndModify(wasModifying);
        }
      }
    }

  else if (item->column() == this->lineVisibilityColumn())
    {
    vtkMRMLFiberBundleDisplayNode* displayNode = fbNode->GetLineDisplayNode();
    if (displayNode)
      {
      this->updateVilibilityFromItem(item, displayNode);
      }
  }
  else if (item->column() == this->tubeVisibilityColumn())
    {
    vtkMRMLFiberBundleDisplayNode* displayNode = fbNode->GetTubeDisplayNode();
    if (displayNode)
      {
      this->updateVilibilityFromItem(item, displayNode);
      }
    }
  else if (item->column() == this->tubeIntersectionVisibilityColumn())
    {
    vtkMRMLFiberBundleDisplayNode* displayNode = fbNode->GetTubeDisplayNode();
    if (displayNode)
      {
      this->updateVilibilityFromItem(item, displayNode, true);
      }
    }
  else if (item->column() == this->glyphVisibilityColumn())
    {
    vtkMRMLFiberBundleDisplayNode* displayNode = fbNode->GetGlyphDisplayNode();
    if (displayNode)
      {
      this->updateVilibilityFromItem(item, displayNode);
      }
    }
  return this->Superclass::updateNodeFromItemData(node, item);
}

//------------------------------------------------------------------------------
void qMRMLSceneTractographyDisplayModel::
updateVilibilityFromItem(QStandardItem* item, vtkMRMLNode* node, bool slice)
{
  Q_D(qMRMLSceneTractographyDisplayModel);

  vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(node);

  int visible = -1;
  if (item->icon().cacheKey() == d->HiddenIcon.cacheKey())
    {
    visible = 0;
    }
  else if (item->icon().cacheKey() == d->VisibleIcon.cacheKey())
    {
    visible = 1;
    }
  else if (item->icon().cacheKey() == d->PartiallyVisibleIcon.cacheKey())
    {
    visible = 2;
    }
  if (displayNode)
    {
    if (slice)
      {
      displayNode->SetSliceIntersectionVisibility(visible);
      }
    else
      {
      displayNode->SetVisibility(visible);
      }
    }
  }

//------------------------------------------------------------------------------
int qMRMLSceneTractographyDisplayModel::colorColumn()const
{
  Q_D(const qMRMLSceneTractographyDisplayModel);
  return d->ColorColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneTractographyDisplayModel::setColorColumn(int column)
{
  Q_D(qMRMLSceneTractographyDisplayModel);
  d->ColorColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSceneTractographyDisplayModel::lineVisibilityColumn()const
{
  Q_D(const qMRMLSceneTractographyDisplayModel);
  return d->LineVisibilityColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneTractographyDisplayModel::setLineVisibilityColumn(int column)
{
  Q_D(qMRMLSceneTractographyDisplayModel);
  d->LineVisibilityColumn = column;
  this->updateColumnCount();
}
//------------------------------------------------------------------------------
int qMRMLSceneTractographyDisplayModel::tubeVisibilityColumn()const
{
  Q_D(const qMRMLSceneTractographyDisplayModel);
  return d->TubeVisibilityColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneTractographyDisplayModel::setTubeVisibilityColumn(int column)
{
  Q_D(qMRMLSceneTractographyDisplayModel);
  d->TubeVisibilityColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSceneTractographyDisplayModel::tubeIntersectionVisibilityColumn()const
{
  Q_D(const qMRMLSceneTractographyDisplayModel);
  return d->TubeIntersectionVisibilityColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneTractographyDisplayModel::setTubeIntersectionVisibilityColumn(int column)
{
  Q_D(qMRMLSceneTractographyDisplayModel);
  d->TubeIntersectionVisibilityColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLSceneTractographyDisplayModel::glyphVisibilityColumn()const
{
  Q_D(const qMRMLSceneTractographyDisplayModel);
  return d->GlyphVisibilityColumn;
}

//------------------------------------------------------------------------------
void qMRMLSceneTractographyDisplayModel::setGlyphVisibilityColumn(int column)
{
  Q_D(qMRMLSceneTractographyDisplayModel);
  d->GlyphVisibilityColumn = column;
  this->updateColumnCount();
}


//------------------------------------------------------------------------------
int qMRMLSceneTractographyDisplayModel::maxColumnId()const
{
  Q_D(const qMRMLSceneTractographyDisplayModel);
  int maxId = this->Superclass::maxColumnId();
  maxId = qMax(maxId, d->LineVisibilityColumn);
  maxId = qMax(maxId, d->TubeVisibilityColumn);
  maxId = qMax(maxId, d->TubeIntersectionVisibilityColumn);
  maxId = qMax(maxId, d->GlyphVisibilityColumn);
  maxId = qMax(maxId, d->ColorColumn);
  return maxId;
}
