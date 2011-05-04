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
#include <QAction>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollBar>
#include <QMessageBox>

// CTK includes
//#include "ctkModelTester.h"

// qMRML includes
#include "qMRMLSceneModel.h"
#include "qMRMLSceneDisplayableModel.h"
#include "qMRMLSceneModelHierarchyModel.h"
#include "qMRMLSceneTransformModel.h"
#include "qMRMLSortFilterModelHierarchyProxyModel.h"
#include "qMRMLSortFilterProxyModel.h"
#include "qMRMLTreeWidget.h"

// MRML includes
#include <vtkMRMLScene.h>

//------------------------------------------------------------------------------
class qMRMLTreeWidgetPrivate
{
  Q_DECLARE_PUBLIC(qMRMLTreeWidget);
protected:
  qMRMLTreeWidget* const q_ptr;
public:
  qMRMLTreeWidgetPrivate(qMRMLTreeWidget& object);
  void init();
  void setSceneModel(qMRMLSceneModel* newModel);
  void setSortFilterProxyModel(qMRMLSortFilterProxyModel* newSortModel);
  QSize sizeHint()const;
  void recomputeSizeHint(bool forceUpdate = false);

  qMRMLSceneModel*           SceneModel;
  qMRMLSortFilterProxyModel* SortFilterModel;
  QString                    SceneModelType;
  bool                       FitSizeToVisibleIndexes;
  mutable QSize              TreeViewSizeHint;
  
  QMenu*                     NodeMenu;
  vtkMRMLNode*               CurrentNode;
};

//------------------------------------------------------------------------------
qMRMLTreeWidgetPrivate::qMRMLTreeWidgetPrivate(qMRMLTreeWidget& object)
  : q_ptr(&object)
{
  this->SceneModel = 0;
  this->SortFilterModel = 0;
  this->FitSizeToVisibleIndexes = true;
  this->NodeMenu = 0;
  this->CurrentNode = 0;
  this->TreeViewSizeHint = QSize();
}

//------------------------------------------------------------------------------
void qMRMLTreeWidgetPrivate::init()
{
  Q_Q(qMRMLTreeWidget);
  this->setSortFilterProxyModel(new qMRMLSortFilterProxyModel(q));
  q->setSceneModelType("Transform");
  
  //ctkModelTester * tester = new ctkModelTester(p);
  //tester->setModel(this->SortFilterModel);
  //QObject::connect(q, SIGNAL(activated(const QModelIndex&)),
  //                 q, SLOT(onActivated(const QModelIndex&)));
  //QObject::connect(q, SIGNAL(clicked(const QModelIndex&)),
  //                 q, SLOT(onActivated(const QModelIndex&)));

  q->setUniformRowHeights(true);
  
  QObject::connect(q, SIGNAL(collapsed(const QModelIndex&)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));
  QObject::connect(q, SIGNAL(expanded(const QModelIndex&)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));
//QObject::connect(q->header(), SIGNAL(sectionResized(int, int, int)),
  //                  q, SLOT(onSectionResized()));
  q->horizontalScrollBar()->installEventFilter(q);
  
  this->NodeMenu = new QMenu(q);
  QAction* deleteAction = new QAction("Delete",this->NodeMenu);
  this->NodeMenu->addAction(deleteAction);
  QObject::connect(deleteAction, SIGNAL(triggered()),
                   q, SLOT(deleteCurrentNode()));
}

//------------------------------------------------------------------------------
void qMRMLTreeWidgetPrivate::setSceneModel(qMRMLSceneModel* newModel)
{
  Q_Q(qMRMLTreeWidget);
  if (!newModel)
    {
    return;
    }

  newModel->setMRMLScene(q->mrmlScene());

  this->SceneModel = newModel;
  this->SortFilterModel->setSourceModel(this->SceneModel);

  q->expandToDepth(2);
}

//------------------------------------------------------------------------------
void qMRMLTreeWidgetPrivate::setSortFilterProxyModel(qMRMLSortFilterProxyModel* newSortModel)
{
  Q_Q(qMRMLTreeWidget);
  if (newSortModel == this->SortFilterModel)
    {
    return;
    }
  
  // delete the previous filter
  delete this->SortFilterModel;
  this->SortFilterModel = newSortModel;
  // Set the input of the view
  // if no filter is given then let's show the scene model directly
  q->QTreeView::setModel(this->SortFilterModel
    ? static_cast<QAbstractItemModel*>(this->SortFilterModel)
    : static_cast<QAbstractItemModel*>(this->SceneModel));
  // Setting a new model to the view resets the selection model
  QObject::connect(q->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
                   q, SLOT(onCurrentRowChanged(const QModelIndex&)));
  if (!this->SortFilterModel)
    {
    return;
    }
  this->SortFilterModel->setParent(q);
  // Set the input of the filter
  this->SortFilterModel->setSourceModel(this->SceneModel);

  // resize the view if new rows are added/removed
  QObject::connect(this->SortFilterModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));
  QObject::connect(this->SortFilterModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));

  q->expandToDepth(2);
  q->onNumberOfVisibleIndexChanged();
}

//------------------------------------------------------------------------------
void qMRMLTreeWidgetPrivate::recomputeSizeHint(bool force)
{
  Q_Q(qMRMLTreeWidget);
  this->TreeViewSizeHint = QSize();
  if ((this->FitSizeToVisibleIndexes || force) && q->isVisible())
    {
    // TODO: if the number of items changes often, don't update geometry,
    // it might be too expensive, maybe use a timer
    q->updateGeometry();
    }
}

//------------------------------------------------------------------------------
QSize qMRMLTreeWidgetPrivate::sizeHint()const
{
  Q_Q(const qMRMLTreeWidget);
  if (!this->FitSizeToVisibleIndexes)
    {
    return q->QTreeView::sizeHint();
    }
  if (this->TreeViewSizeHint.isValid())
    {
    return this->TreeViewSizeHint;
    }
  int visibleIndexCount = 0;
  for(QModelIndex index = this->SortFilterModel->mrmlSceneIndex();
      index.isValid();
      index = q->indexBelow(index))
    {
    ++visibleIndexCount;
    }

  this->TreeViewSizeHint = q->QTreeView::sizeHint();
  this->TreeViewSizeHint.setHeight(
    q->frameWidth()
    + (q->isHeaderHidden() ? 0 : q->header()->sizeHint().height())
    + visibleIndexCount * q->sizeHintForRow(0)
    + (q->horizontalScrollBar()->isVisibleTo(const_cast<qMRMLTreeWidget*>(q)) ? q->horizontalScrollBar()->height() : 0)
    + q->frameWidth());
  return this->TreeViewSizeHint;
}

//------------------------------------------------------------------------------
// qMRMLTreeWidget
//------------------------------------------------------------------------------
qMRMLTreeWidget::qMRMLTreeWidget(QWidget *_parent)
  :QTreeView(_parent)
  , d_ptr(new qMRMLTreeWidgetPrivate(*this))
{
  Q_D(qMRMLTreeWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLTreeWidget::~qMRMLTreeWidget()
{
}

//------------------------------------------------------------------------------
void qMRMLTreeWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLTreeWidget);
  Q_ASSERT(d->SortFilterModel);
  // only qMRMLSceneModel needs the scene, the other proxies don't care.
  d->SceneModel->setMRMLScene(scene);
  this->expandToDepth(2);
}

//------------------------------------------------------------------------------
QString qMRMLTreeWidget::sceneModelType()const
{
  Q_D(const qMRMLTreeWidget);
  return d->SceneModelType;
}

//------------------------------------------------------------------------------
void qMRMLTreeWidget::setSceneModelType(const QString& modelName)
{
  Q_D(qMRMLTreeWidget);

  qMRMLSceneModel* newModel = 0;
  qMRMLSortFilterProxyModel* newFilterModel = d->SortFilterModel;
  // switch on the incoming model name
  if (modelName == QString("Transform"))
    {
    newModel = new qMRMLSceneTransformModel(this);
    }
  else if (modelName == QString("Displayable"))
    {
    newModel = new qMRMLSceneDisplayableModel(this);
    }
  else if (modelName == QString("ModelHierarchy"))
    {
    newModel = new qMRMLSceneModelHierarchyModel(this);
    newFilterModel = new qMRMLSortFilterModelHierarchyProxyModel(this);
    }
  if (newModel)
    {
    d->SceneModelType = modelName;
    newModel->setListenNodeModifiedEvent(this->listenNodeModifiedEvent());
    }
  if (newFilterModel)
    {
    newFilterModel->setNodeTypes(this->nodeTypes());
    }
  d->setSceneModel(newModel);
  // typically a no op except for ModelHierarchy
  d->setSortFilterProxyModel(newFilterModel);
}

//------------------------------------------------------------------------------
void qMRMLTreeWidget::setSceneModel(qMRMLSceneModel* newSceneModel, const QString& modelType)
{
  Q_D(qMRMLTreeWidget);

  if (!newSceneModel) 
    {
    return;
    }
  d->SceneModelType = modelType;
  d->setSceneModel(newSceneModel);
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLTreeWidget::mrmlScene()const
{
  Q_D(const qMRMLTreeWidget);
  return d->SceneModel ? d->SceneModel->mrmlScene() : 0;
}

//------------------------------------------------------------------------------
void qMRMLTreeWidget::onCurrentRowChanged(const QModelIndex& index)
{
  Q_D(qMRMLTreeWidget);
  Q_ASSERT(d->SortFilterModel);
  emit currentNodeChanged(d->SortFilterModel->mrmlNodeFromIndex(index));
}

//------------------------------------------------------------------------------
void qMRMLTreeWidget::setListenNodeModifiedEvent(bool listen)
{
  Q_D(qMRMLTreeWidget);
  Q_ASSERT(d->SceneModel);
  d->SceneModel->setListenNodeModifiedEvent(listen);
}

//------------------------------------------------------------------------------
bool qMRMLTreeWidget::listenNodeModifiedEvent()const
{
  Q_D(const qMRMLTreeWidget);
  return d->SceneModel ? d->SceneModel->listenNodeModifiedEvent() : false;
}

// --------------------------------------------------------------------------
QStringList qMRMLTreeWidget::nodeTypes()const
{
  return this->sortFilterProxyModel()->nodeTypes();
}

// --------------------------------------------------------------------------
void qMRMLTreeWidget::setNodeTypes(const QStringList& _nodeTypes)
{
  this->sortFilterProxyModel()->setNodeTypes(_nodeTypes);
}

//--------------------------------------------------------------------------
qMRMLSortFilterProxyModel* qMRMLTreeWidget::sortFilterProxyModel()const
{
  Q_D(const qMRMLTreeWidget);
  Q_ASSERT(d->SortFilterModel);
  return d->SortFilterModel;
}

//--------------------------------------------------------------------------
qMRMLSceneModel* qMRMLTreeWidget::sceneModel()const
{
  Q_D(const qMRMLTreeWidget);
  Q_ASSERT(d->SceneModel);
  return d->SceneModel;
}

//--------------------------------------------------------------------------
QSize qMRMLTreeWidget::minimumSizeHint()const
{
  Q_D(const qMRMLTreeWidget);
  return d->sizeHint();
}

//--------------------------------------------------------------------------
QSize qMRMLTreeWidget::sizeHint()const
{
  Q_D(const qMRMLTreeWidget);
  return d->sizeHint();
}

//--------------------------------------------------------------------------
void qMRMLTreeWidget::updateGeometries()
{
  // don't update the geometries if it's not visible on screen
  // UpdateGeometries is for tree child widgets geometry
  if (!this->isVisible())
    {
    return;
    }
  this->QTreeView::updateGeometries();
}

//--------------------------------------------------------------------------
void qMRMLTreeWidget::onNumberOfVisibleIndexChanged()
{
  Q_D(qMRMLTreeWidget);
  d->recomputeSizeHint();
}

//--------------------------------------------------------------------------
void qMRMLTreeWidget::setFitSizeToVisibleIndexes(bool enable)
{
  Q_D(qMRMLTreeWidget);
  d->FitSizeToVisibleIndexes = enable;
  d->recomputeSizeHint(true);
}

//--------------------------------------------------------------------------
bool qMRMLTreeWidget::fitSizeToVisibleIndexes()const
{
  Q_D(const qMRMLTreeWidget);
  return d->FitSizeToVisibleIndexes;
}

//------------------------------------------------------------------------------
void qMRMLTreeWidget::mousePressEvent(QMouseEvent* e)
{
  Q_D(qMRMLTreeWidget);
  this->QTreeView::mousePressEvent(e);
  
  if (e->button() != Qt::RightButton)
    {
    return;
    }
  // get the index of the current column
  QModelIndex index = this->indexAt(e->pos());
  
  vtkMRMLNode* node = this->sortFilterProxyModel()->mrmlNodeFromIndex(index);
  
  if (!node)
    {
    return;
    }

  d->CurrentNode = node;
  d->NodeMenu->exec(e->globalPos());
  d->CurrentNode = 0;
}

//------------------------------------------------------------------------------
void qMRMLTreeWidget::deleteCurrentNode()
{
  Q_D(qMRMLTreeWidget);
  if (!d->CurrentNode)
    {
    qWarning() << "No node to delete";
    return;
    }
  this->mrmlScene()->RemoveNode(d->CurrentNode);
  d->CurrentNode = 0;
}

//------------------------------------------------------------------------------
bool qMRMLTreeWidget::eventFilter(QObject* object, QEvent* e)
{
  Q_D(qMRMLTreeWidget);
  bool res = this->QTreeView::eventFilter(object, e);
  // When the horizontal scroll bar is shown/hidden, the sizehint should be
  // updated ?
  if (d->FitSizeToVisibleIndexes &&
      object == this->horizontalScrollBar() &&
      (e->type() == QEvent::Show ||
       e->type() == QEvent::Hide))
    {
    d->recomputeSizeHint();
    }
  return res;
}
