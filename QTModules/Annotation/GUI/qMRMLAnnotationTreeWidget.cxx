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
#include <QMouseEvent>

// CTK includes
#include "ctkModelTester.h"

// qMRML includes
#include "qMRMLSceneModel.h"
#include "qMRMLSortFilterProxyModel.h"
#include "qMRMLSceneTransformModel.h"
#include "qMRMLSceneAnnotationModel.h"
#include "qMRMLTreeWidget.h"

#include "qMRMLAnnotationTreeWidget.h"

// MRML includes
#include "vtkMRMLNode.h"

//------------------------------------------------------------------------------
class qMRMLAnnotationTreeWidgetPrivate
{
  Q_DECLARE_PUBLIC(qMRMLAnnotationTreeWidget);
protected:
  qMRMLAnnotationTreeWidget* const q_ptr;
public:
  qMRMLAnnotationTreeWidgetPrivate(qMRMLAnnotationTreeWidget& object);
  void init();

  qMRMLSceneAnnotationModel*           SceneModel;
  qMRMLSortFilterProxyModel* SortFilterModel;
};

//------------------------------------------------------------------------------
qMRMLAnnotationTreeWidgetPrivate::qMRMLAnnotationTreeWidgetPrivate(qMRMLAnnotationTreeWidget& object)
  : q_ptr(&object)
{
  this->SceneModel = 0;
  this->SortFilterModel = 0;
}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidgetPrivate::init()
{
  Q_Q(qMRMLAnnotationTreeWidget);
  //p->qMRMLTreeWidget::setModel(new qMRMLItemModel(p));
  this->SceneModel = new qMRMLSceneAnnotationModel(q);
  this->SceneModel->setColumnCount(6);

  this->SceneModel->setAndObserveTreeView(q);

  //this->SceneModel->setListenNodeModifiedEvent(true);

  this->SortFilterModel = new qMRMLSortFilterProxyModel(q);
  // we only want to show vtkMRMLAnnotationNodes and vtkMRMLAnnotationHierarchyNodes
  QStringList nodeTypes = QStringList();
  nodeTypes.append("vtkMRMLAnnotationNode");
  nodeTypes.append("vtkMRMLAnnotationHierarchyNode");
  nodeTypes.append("vtkMRMLAnnotationSnapshotNode");

  this->SortFilterModel->setNodeTypes(nodeTypes);

  this->SortFilterModel->setSourceModel(this->SceneModel);
  q->qMRMLTreeWidget::setModel(this->SortFilterModel);

  //ctkModelTester * tester = new ctkModelTester(p);
  //tester->setModel(this->SortFilterModel);
  QObject::connect(q, SIGNAL(activated(const QModelIndex&)),
                   q, SLOT(onActivated(const QModelIndex&)));
  QObject::connect(q, SIGNAL(clicked(const QModelIndex&)),
                   q, SLOT(onActivated(const QModelIndex&)));


  QObject::connect(q, SIGNAL(clicked(const QModelIndex& )),
                   q, SLOT(onClicked(const QModelIndex&)));

  QObject::connect(this->SceneModel, SIGNAL(dataChanged ( const QModelIndex&, const QModelIndex&)),
                   q, SLOT(onDataChanged(const QModelIndex&, const QModelIndex&)));

  q->setUniformRowHeights(true);
}

//------------------------------------------------------------------------------
qMRMLAnnotationTreeWidget::qMRMLAnnotationTreeWidget(QWidget *_parent)
  :qMRMLTreeWidget(_parent)
  , d_ptr(new qMRMLAnnotationTreeWidgetPrivate(*this))
{
  Q_D(qMRMLAnnotationTreeWidget);
  d->init();

  // we need to enable mouse tracking to set the appropriate cursor while mouseMove occurs
  this->setMouseTracking(true);
}

//------------------------------------------------------------------------------
qMRMLAnnotationTreeWidget::~qMRMLAnnotationTreeWidget()
{
}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLAnnotationTreeWidget);
  Q_ASSERT(d->SortFilterModel);
  // only qMRMLSceneModel needs the scene, the other proxies don't care.
  d->SceneModel->setMRMLScene(scene);

  this->expandAll();
}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::onActivated(const QModelIndex& index)
{
  Q_D(qMRMLAnnotationTreeWidget);
  Q_ASSERT(d->SortFilterModel);

  emit currentNodeChanged(d->SortFilterModel->mrmlNode(index));
}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::onDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{

  Q_D(qMRMLAnnotationTreeWidget);
  // loop through all columns and rows which fit to the modified MRML node
  for (int i = topLeft.row(); i <= bottomRight.row(); ++i)
    {
    for (int j = topLeft.column(); j <= bottomRight.column(); ++j)
      {

      QStandardItem* newNodeItem = new QStandardItem();
      d->SceneModel->updateItemFromNode(newNodeItem, d->SortFilterModel->mrmlNode(topLeft.sibling(i,j)), j);

      }
    }
}


//------------------------------------------------------------------------------
//
// Click and selected event handling
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::onClicked(const QModelIndex& index)
{

  Q_D(qMRMLAnnotationTreeWidget);

  if (index.column() == qMRMLSceneAnnotationModel::VisibilityColumn)
    {
    // user wants to toggle the visibility of the annotation
    this->onVisibilityColumnClicked(d->SortFilterModel->mrmlNode(index));
    }
  else if (index.column() == qMRMLSceneAnnotationModel::LockColumn)
    {
    // user wants to toggle the un-/lock of the annotation
    this->onLockColumnClicked(d->SortFilterModel->mrmlNode(index));
    }
  else if (index.column() == qMRMLSceneAnnotationModel::EditColumn)
    {
    // user wants to edit the properties of this annotation

    }

}

//------------------------------------------------------------------------------
const char* qMRMLAnnotationTreeWidget::getFirstSelectedNode()
{
  Q_D(qMRMLAnnotationTreeWidget);
  QModelIndexList selected = this->selectedIndexes();
  QModelIndex index = selected.first();

  if (!d->SceneModel->mrmlNodeFromIndex(index))
    {
    //std::cout << "no node found" << std::endl;
    return 0;
    }

  return d->SceneModel->mrmlNodeFromIndex(index)->GetID();
}

//------------------------------------------------------------------------------
//
// MouseMove event handling
//
//------------------------------------------------------------------------------

#ifndef QT_NO_CURSOR
//------------------------------------------------------------------------------
bool qMRMLAnnotationTreeWidget::viewportEvent(QEvent* e)
{

  // reset the cursor if we leave the viewport
  if(e->type() == QEvent::Leave)
    {
    setCursor(QCursor());
    }

  return QTreeView::viewportEvent(e);
}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::mouseMoveEvent(QMouseEvent* e)
{
  this->QTreeView::mouseMoveEvent(e);

  // get the index of the current column
  QModelIndex index = indexAt(e->pos());

  if (index.column() == qMRMLSceneAnnotationModel::VisibilityColumn || index.column() == qMRMLSceneAnnotationModel::LockColumn || index.column() == qMRMLSceneAnnotationModel::EditColumn)
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

}
#endif

//------------------------------------------------------------------------------
//
// Layout and behavior customization
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::refresh()
{
  // the first and the columns with icons should be resized

  this->resizeColumnToContents(0);
  this->resizeColumnToContents(1);
  this->resizeColumnToContents(2);
  this->resizeColumnToContents(3);
  this->resizeColumnToContents(4);
  this->resizeColumnToContents(5);

  this->header()->setResizeMode(0,QHeaderView::Fixed);
  this->header()->setResizeMode(1,QHeaderView::Fixed);
  this->header()->setResizeMode(2,QHeaderView::Fixed);
  this->header()->setResizeMode(3,QHeaderView::Fixed);

  // hide the strange columns
  this->hideColumn(6);
  this->hideColumn(7);
  this->hideColumn(8);
  this->hideColumn(9);
  this->hideColumn(10);
  this->hideColumn(11);



}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::hideScene()
{
  Q_D(qMRMLAnnotationTreeWidget);

  // first, we set the root index to the mrmlScene
  // this works also if the scene is not defined yet
  QModelIndex root = d->SceneModel->mrmlSceneIndex();

  if (this->m_Logic)
    {
    // if the logic is already registered, we look for the first annotationHierarchyNode
    vtkMRMLNode* toplevelNode = this->m_Logic->GetMRMLScene()->GetNthNodeByClass(0,"vtkMRMLAnnotationHierarchyNode");

    if (toplevelNode)
      {
      // if we find it, we use it as the root index
      root = d->SceneModel->indexes(toplevelNode)[0];
      }
    }

  this->setRootIndex(d->SortFilterModel->mapFromSource(root));
}

//------------------------------------------------------------------------------
//
// In-Place Editing of Annotations
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::onVisibilityColumnClicked(vtkMRMLNode* node)
{

  if (!node)
    {
    // no node found!
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);

  if (annotationNode)
    {
    // this is a valid annotationNode
    annotationNode->SetVisible(!annotationNode->GetVisible());

    }

  // TODO handle hierarchies

}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::onLockColumnClicked(vtkMRMLNode* node)
{

  if (!node)
    {
    // no node found!
    return;
    }

  vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);

  if (annotationNode)
    {
    // this is a valid annotationNode
    annotationNode->SetLocked(!annotationNode->GetLocked());

    }

  // TODO handle hierarchies

}

//------------------------------------------------------------------------------
//
// Connections to other classes
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Set and observe the GUI widget
//-----------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::setAndObserveWidget(qSlicerAnnotationModuleWidget* widget)
{
  if (!widget)
    {
    return;
    }

  Q_D(qMRMLAnnotationTreeWidget);

  this->m_Widget = widget;


  // propagate down to model
  d->SceneModel->setAndObserveWidget(this->m_Widget);

}

//-----------------------------------------------------------------------------
/// Set and observe the logic
//-----------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::setAndObserveLogic(vtkSlicerAnnotationModuleLogic* logic)
{
  if (!logic)
    {
    return;
    }

  Q_D(qMRMLAnnotationTreeWidget);

  this->m_Logic = logic;

  // propagate down to model
  d->SceneModel->setAndObserveLogic(this->m_Logic);

}
