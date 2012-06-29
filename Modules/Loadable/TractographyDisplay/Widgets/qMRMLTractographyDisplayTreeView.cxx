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
#include <QDebug>
#include <QHeaderView>
#include <QMessageBox>
#include <QMouseEvent>

// CTK includes
//#include "ctkModelTester.h"

// qMRML includes
//#include "qMRMLSortFilterProxyModel.h"
//#include "qMRMLSceneTransformModel.h"
//#include "qMRMLTreeView.h"

// TractographyDisplays includes
#include "qMRMLTractographyDisplayTreeView.h"
#include "qMRMLSceneTractographyDisplayModel.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkMRMLFiberBundleLineDisplayNode.h"
#include "vtkMRMLFiberBundleGlyphDisplayNode.h"
#include "vtkMRMLFiberBundleTubeDisplayNode.h"

// MRML includes


//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TractographyDisplay
class qMRMLTractographyDisplayTreeViewPrivate
{
  Q_DECLARE_PUBLIC(qMRMLTractographyDisplayTreeView);
protected:
  qMRMLTractographyDisplayTreeView* const q_ptr;
public:
  qMRMLTractographyDisplayTreeViewPrivate(qMRMLTractographyDisplayTreeView& object);
  void init();

  qMRMLSceneTractographyDisplayModel*           SceneModel;
  qMRMLSortFilterProxyModel* SortFilterModel;
};

//------------------------------------------------------------------------------
qMRMLTractographyDisplayTreeViewPrivate::qMRMLTractographyDisplayTreeViewPrivate(qMRMLTractographyDisplayTreeView& object)
  : q_ptr(&object)
{
  this->SceneModel = 0;
  this->SortFilterModel = 0;
}

//------------------------------------------------------------------------------
void qMRMLTractographyDisplayTreeViewPrivate::init()
{
  Q_Q(qMRMLTractographyDisplayTreeView);
  //p->qMRMLTreeView::setModel(new qMRMLItemModel(p));
  //this->SceneModel = new qMRMLSceneTractographyDisplayModel(q);
  //this->SceneModel->setColumnCount(6);

  this->SceneModel = new qMRMLSceneTractographyDisplayModel(q);
  q->setSceneModel(this->SceneModel, "TractographyDisplay");

  //this->SortFilterModel = new qMRMLSortFilterProxyModel(q);
  // we only want to show vtkMRMLTractographyDisplayNodes and vtkMRMLTractographyDisplayHierarchyNodes
  QStringList nodeTypes = QStringList();
  nodeTypes.append("vtkMRMLFiberBundleNode");
  //nodeTypes.append("vtkMRMLTractographyDisplayHierarchyNode");

  //this->SortFilterModel->setNodeTypes(nodeTypes);
  q->setNodeTypes(nodeTypes);
  this->SortFilterModel = q->sortFilterProxyModel();

  q->header()->setStretchLastSection(false);
  q->header()->setResizeMode(QHeaderView::ResizeToContents);
  q->header()->setResizeMode(0, QHeaderView::Stretch);

  //this->SortFilterModel->setSourceModel(this->SceneModel);
  //q->qMRMLTreeView::setModel(this->SortFilterModel);

  //ctkModelTester * tester = new ctkModelTester(p);
  //tester->setModel(this->SortFilterModel);

  QObject::connect(q, SIGNAL(clicked(QModelIndex)),
                   q, SLOT(onClicked(QModelIndex)));

  /***
  QObject::connect( q->selectionModel(),
        SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
        q,
        SLOT(onSelectionChanged(QItemSelection,QItemSelection)),
        Qt::DirectConnection );
  ***/

  q->setUniformRowHeights(true);
}

//------------------------------------------------------------------------------
qMRMLTractographyDisplayTreeView::qMRMLTractographyDisplayTreeView(QWidget *_parent)
  :qMRMLTreeView(_parent)
  , d_ptr(new qMRMLTractographyDisplayTreeViewPrivate(*this))
{
  Q_D(qMRMLTractographyDisplayTreeView);
  d->init();

  // we need to enable mouse tracking to set the appropriate cursor while mouseMove occurs
  this->setMouseTracking(true);
}

//------------------------------------------------------------------------------
qMRMLTractographyDisplayTreeView::~qMRMLTractographyDisplayTreeView()
{
}

//------------------------------------------------------------------------------
//
// Click and selected event handling
//
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
void qMRMLTractographyDisplayTreeView::onClicked(const QModelIndex& index)
{
/**
  Q_D(qMRMLTractographyDisplayTreeView);

  vtkMRMLNode *mrmlNode = d->SortFilterModel->mrmlNodeFromIndex(index);
  if (!mrmlNode)
    {
    return;
    }
  // if the user clicked on a hierarchy, set this as the active one
  // this means, new TractographyDisplays or new user-created hierarchies will be created
  // as childs of this one

  if(mrmlNode->IsA("vtkMRMLTractographyDisplayHierarchyNode"))
    {
    this->m_Logic->SetActiveHierarchyNodeID(mrmlNode->GetID());
    }
  else
    {
    // if the user clicked on a row that isn't a hierarchy node, reset the
    // active hierarchy to the parent hierarchy of this node (going via the
    // hierarchy node associated with this node)
    if(mrmlNode &&
       !mrmlNode->IsA("vtkMRMLTractographyDisplayHierarchyNode"))
      {
      vtkMRMLHierarchyNode *hnode = vtkMRMLTractographyDisplayHierarchyNode::GetAssociatedHierarchyNode(this->mrmlScene(), mrmlNode->GetID());
      if (hnode && hnode->GetParentNode())
        {
        this->m_Logic->SetActiveHierarchyNodeID(hnode->GetParentNode()->GetID());
        }
      }
    }

  // check if user clicked on icon, this can happen even after we marked a
  // hierarchy as active
  if (index.column() == qMRMLSceneTractographyDisplayModel::CheckedColumn)
    {
    // Let the superclass view to handle the event, it will update the item
    // which will update the node.
    }
  else if (index.column() == qMRMLSceneTractographyDisplayModel::VisibilityColumn)
    {
    // user wants to toggle the visibility of the TractographyDisplay
    this->onVisibilityColumnClicked(mrmlNode);
    }
  else if (index.column() == qMRMLSceneTractographyDisplayModel::LockColumn)
    {
    // user wants to toggle the un-/lock of the TractographyDisplay
    this->onLockColumnClicked(mrmlNode);
    }
  else if (index.column() == qMRMLSceneTractographyDisplayModel::EditColumn)
    {
    // user wants to edit the properties of this TractographyDisplay
    // signal the widget
    this->onPropertyEditButtonClicked(QString(mrmlNode->GetID()));
//    this->m_Widget->propertyEditButtonClicked(QString(mrmlNode->GetID()));
    }
    **/

}



//------------------------------------------------------------------------------
//
// MouseMove event handling
//
//------------------------------------------------------------------------------

#ifndef QT_NO_CURSOR
//------------------------------------------------------------------------------
bool qMRMLTractographyDisplayTreeView::viewportEvent(QEvent* e)
{

  // reset the cursor if we leave the viewport
  if(e->type() == QEvent::Leave)
    {
    setCursor(QCursor());
    }

  return QTreeView::viewportEvent(e);
}

//------------------------------------------------------------------------------
void qMRMLTractographyDisplayTreeView::mouseMoveEvent(QMouseEvent* e)
{
  this->QTreeView::mouseMoveEvent(e);
/**
  // get the index of the current column
  QModelIndex index = indexAt(e->pos());

  if (index.column() == qMRMLSceneTractographyDisplayModel::VisibilityColumn || index.column() == qMRMLSceneTractographyDisplayModel::LockColumn || index.column() == qMRMLSceneTractographyDisplayModel::EditColumn)
    {
    // we are over a column with a clickable icon
    // let's change the cursor
    QCursor handCursor(Qt::PointingHandCursor);
    this->setCursor(handCursor);
    // .. and bail out
    return;
    }
  else if(this->cursor().shape() == Qt::PointingHandCursor)
    {
    // if we are NOT over such a column and we already have a changed cursor,
    // reset it!
    this->setCursor(QCursor());
    }
**/
}
#endif

//------------------------------------------------------------------------------
//
// Layout and behavior customization
//
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//
// In-Place Editing of TractographyDisplays
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void qMRMLTractographyDisplayTreeView::onVisibilityColumnClicked(vtkMRMLNode* node)
{

  if (!node)
    {
    // no node found!
    return;
    }
/**
  vtkMRMLTractographyDisplayNode* TractographyDisplayNode = vtkMRMLTractographyDisplayNode::SafeDownCast(node);

  if (TractographyDisplayNode)
    {
    // this is a valid TractographyDisplayNode
    TractographyDisplayNode->SetVisible(!TractographyDisplayNode->GetVisible());

    }

  // taking out the switch for hierarchy nodes, do it via the buttons above
/*
  vtkMRMLTractographyDisplayHierarchyNode* hierarchyNode = vtkMRMLTractographyDisplayHierarchyNode::SafeDownCast(node);

  if (hierarchyNode)
    {
    this->m_Logic->SetHierarchyTractographyDisplaysVisibleFlag(hierarchyNode, true);
    } // if hierarchyNode
    **/
}



//------------------------------------------------------------------------------
void qMRMLTractographyDisplayTreeView::mousePressEvent(QMouseEvent* event)
{
  // skip qMRMLTreeView
  this->QTreeView::mousePressEvent(event);
}

//------------------------------------------------------------------------------
//
// Connections to other classes
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Set and observe the logic
//-----------------------------------------------------------------------------
void qMRMLTractographyDisplayTreeView::setLogic(vtkSlicerFiberBundleLogic* logic)
{
  if (!logic)
    {
    return;
    }

  Q_D(qMRMLTractographyDisplayTreeView);

  this->m_Logic = logic;

  // propagate down to model
  //d->SceneModel->setLogic(this->m_Logic);

}

//------------------------------------------------------------------------------
void qMRMLTractographyDisplayTreeView::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  this->setRootIndex(this->sortFilterProxyModel()->mrmlSceneIndex());
}

//------------------------------------------------------------------------------
bool qMRMLTractographyDisplayTreeView::clickDecoration(const QModelIndex& index)
{
  bool res = false;
  int type = -1;
  QModelIndex sourceIndex = this->sortFilterProxyModel()->mapToSource(index);

  vtkMRMLFiberBundleNode* fbNode = vtkMRMLFiberBundleNode::SafeDownCast(
          this->sortFilterProxyModel()->mrmlNodeFromIndex(index));

  qMRMLSceneTractographyDisplayModel* model = dynamic_cast< qMRMLSceneTractographyDisplayModel* >(this->sceneModel());
  
  if (!(sourceIndex.flags() & Qt::ItemIsEnabled))
    {
    res = false;
    }
  else if (sourceIndex.column() == model->lineVisibilityColumn())
    {
    type = 0;
    vtkMRMLFiberBundleDisplayNode* displayNode = fbNode->GetLineDisplayNode();
    if (displayNode)
      {
      displayNode->SetVisibility(displayNode->GetVisibility() ? 0 : 1);
      res = true;
      }
  }
  else if (sourceIndex.column() == model->tubeVisibilityColumn())
    {
    type = 1;
    vtkMRMLFiberBundleDisplayNode* displayNode = fbNode->GetTubeDisplayNode();
    if (displayNode)
      {
      displayNode->SetVisibility(displayNode->GetVisibility() ? 0 : 1);
      res = true;
      }
  }
  else if (sourceIndex.column() == model->tubeIntersectionVisibilityColumn())
    {
    vtkMRMLFiberBundleDisplayNode* displayNode = fbNode->GetTubeDisplayNode();
    if (displayNode)
      {
      displayNode->SetSliceIntersectionVisibility(displayNode->GetSliceIntersectionVisibility() ? 0 : 1);
      res = true;
      }
  }
  else if (sourceIndex.column() == model->glyphVisibilityColumn())
    {
    type = 2;
    vtkMRMLFiberBundleDisplayNode* displayNode = fbNode->GetGlyphDisplayNode();
    if (displayNode)
      {
      displayNode->SetVisibility(displayNode->GetVisibility() ? 0 : 1);
      res = true;
      }
  }

  if (res)
    {
    emit decorationClicked(index);
    }
  if (type > -1)
    {
    emit visibilityChanged(type);
    }
  return res;
}
