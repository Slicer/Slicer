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
#include <QHeaderView>

// CTK includes
//#include "ctkModelTester.h"

// qMRML includes
#include "qMRMLSceneModel.h"
#include "qMRMLSceneDisplayableModel.h"
#include "qMRMLSceneModelHierarchyModel.h"
#include "qMRMLSceneTransformModel.h"
#include "qMRMLSortFilterProxyModel.h"
#include "qMRMLTreeWidget.h"

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
  QSize computeSizeHint()const;

  qMRMLSceneModel*           SceneModel;
  qMRMLSortFilterProxyModel* SortFilterModel;
  QString                    SceneModelType;
  bool                       FitSizeToVisibleIndexes;
};

//------------------------------------------------------------------------------
qMRMLTreeWidgetPrivate::qMRMLTreeWidgetPrivate(qMRMLTreeWidget& object)
  : q_ptr(&object)
{
  this->SceneModel = 0;
  this->SortFilterModel = 0;
  this->FitSizeToVisibleIndexes = true;
}

//------------------------------------------------------------------------------
void qMRMLTreeWidgetPrivate::init()
{
  Q_Q(qMRMLTreeWidget);
  this->SceneModel = new qMRMLSceneTransformModel(q);
  this->SortFilterModel = new qMRMLSortFilterProxyModel(q);
  q->setSceneModelType("Transform");
  q->QTreeView::setModel(this->SortFilterModel);

  //ctkModelTester * tester = new ctkModelTester(p);
  //tester->setModel(this->SortFilterModel);
  QObject::connect(q, SIGNAL(activated(const QModelIndex&)),
                   q, SLOT(onActivated(const QModelIndex&)));
  QObject::connect(q, SIGNAL(clicked(const QModelIndex&)),
                   q, SLOT(onActivated(const QModelIndex&)));

  q->setUniformRowHeights(true);
  
  QObject::connect(q, SIGNAL(collapsed(const QModelIndex&)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));
  QObject::connect(q, SIGNAL(expanded(const QModelIndex&)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));
  QObject::connect(this->SortFilterModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));
  QObject::connect(this->SortFilterModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                   q, SLOT(onNumberOfVisibleIndexChanged()));
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
QSize qMRMLTreeWidgetPrivate::computeSizeHint()const
{
  Q_Q(const qMRMLTreeWidget);
  int visibleIndexCount = 0;
  for(QModelIndex index = this->SortFilterModel->mrmlSceneIndex();
      index.isValid();
      index = q->indexBelow(index))
    {
    ++visibleIndexCount;
    }

  QSize treeViewSizeHint = q->QTreeView::sizeHint();
  treeViewSizeHint.setHeight(
    q->frameWidth()
    + (q->isHeaderHidden() ? 0 : q->header()->sizeHint().height())
    + visibleIndexCount * q->sizeHintForRow(0)
    + q->frameWidth());
  return treeViewSizeHint;
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
    }
  if (newModel) 
    {
    d->SceneModelType = modelName;
    newModel->setListenNodeModifiedEvent(this->listenNodeModifiedEvent());
    }
  d->setSceneModel(newModel);
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
void qMRMLTreeWidget::onActivated(const QModelIndex& index)
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
  Q_D(const qMRMLTreeWidget);
  return d->SortFilterModel->nodeTypes();
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
  QSize minSize = this->QTreeView::minimumSizeHint();
  return minSize.expandedTo(d->computeSizeHint());
}

//--------------------------------------------------------------------------
QSize qMRMLTreeWidget::sizeHint()const
{
  Q_D(const qMRMLTreeWidget);
  QSize size = this->QTreeView::minimumSizeHint();
  return size.expandedTo(d->computeSizeHint());
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
  if (d->FitSizeToVisibleIndexes)
    {
    this->updateGeometry();
    }
}

//--------------------------------------------------------------------------
void qMRMLTreeWidget::setFitSizeToVisibleIndexes(bool enable)
{
  Q_D(qMRMLTreeWidget);
  d->FitSizeToVisibleIndexes = enable;
  if (d->FitSizeToVisibleIndexes)
    {
    this->updateGeometry();
    }
}

//--------------------------------------------------------------------------
bool qMRMLTreeWidget::fitSizeToVisibleIndexes()const
{
  Q_D(const qMRMLTreeWidget);
  return d->FitSizeToVisibleIndexes;
}
