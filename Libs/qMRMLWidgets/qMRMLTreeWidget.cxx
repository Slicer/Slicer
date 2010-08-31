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

// CTK includes
#include "ctkModelTester.h"

// qMRML includes
#include "qMRMLSceneModel.h"
#include "qMRMLSortFilterProxyModel.h"
#include "qMRMLSceneTransformModel.h"
#include "qMRMLTreeWidget.h"

//------------------------------------------------------------------------------
class qMRMLTreeWidgetPrivate: public ctkPrivate<qMRMLTreeWidget>
{
public:
  CTK_DECLARE_PUBLIC(qMRMLTreeWidget);
  qMRMLTreeWidgetPrivate();
  void init();

  qMRMLSceneModel*           SceneModel;
  qMRMLSortFilterProxyModel* SortFilterModel;
};

//------------------------------------------------------------------------------
qMRMLTreeWidgetPrivate::qMRMLTreeWidgetPrivate()
{
  this->SceneModel = 0;
  this->SortFilterModel = 0;
}

//------------------------------------------------------------------------------
void qMRMLTreeWidgetPrivate::init()
{
  CTK_P(qMRMLTreeWidget);
  //p->QTreeView::setModel(new qMRMLItemModel(p));
  this->SceneModel = new qMRMLSceneTransformModel(p);
  this->SortFilterModel = new qMRMLSortFilterProxyModel(p);
  this->SortFilterModel->setSourceModel(this->SceneModel);
  p->QTreeView::setModel(this->SortFilterModel);

  //ctkModelTester * tester = new ctkModelTester(p);
  //tester->setModel(this->SortFilterModel);
  QObject::connect(p, SIGNAL(activated(const QModelIndex&)),
                   p, SLOT(onActivated(const QModelIndex&)));
  QObject::connect(p, SIGNAL(clicked(const QModelIndex&)),
                   p, SLOT(onActivated(const QModelIndex&)));

  p->setUniformRowHeights(true);
}

//------------------------------------------------------------------------------
qMRMLTreeWidget::qMRMLTreeWidget(QWidget *_parent)
  :QTreeView(_parent)
{
  CTK_INIT_PRIVATE(qMRMLTreeWidget);
  ctk_d()->init();
}

//------------------------------------------------------------------------------
qMRMLTreeWidget::~qMRMLTreeWidget()
{
}

//------------------------------------------------------------------------------
void qMRMLTreeWidget::setMRMLScene(vtkMRMLScene* scene)
{
  CTK_D(qMRMLTreeWidget);
  Q_ASSERT(d->SortFilterModel);
  // only qMRMLSceneModel needs the scene, the other proxies don't care.
  d->SceneModel->setMRMLScene(scene);
  this->expandToDepth(2);
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLTreeWidget::mrmlScene()const
{
  CTK_D(const qMRMLTreeWidget);
  Q_ASSERT(d->SceneModel); // can be removed
  return d->SceneModel->mrmlScene();
}

//------------------------------------------------------------------------------
void qMRMLTreeWidget::onActivated(const QModelIndex& index)
{
  CTK_D(qMRMLTreeWidget);
  Q_ASSERT(d->SortFilterModel);
  emit currentNodeChanged(d->SortFilterModel->mrmlNode(index));
}

//------------------------------------------------------------------------------
void qMRMLTreeWidget::setListenNodeModifiedEvent(bool listen)
{
  CTK_D(qMRMLTreeWidget);
  Q_ASSERT(d->SceneModel);
  d->SceneModel->setListenNodeModifiedEvent(listen);
}

//------------------------------------------------------------------------------
bool qMRMLTreeWidget::listenNodeModifiedEvent()const
{
  CTK_D(const qMRMLTreeWidget);
  Q_ASSERT(d->SceneModel);
  return d->SceneModel->listenNodeModifiedEvent();
}

//--------------------------------------------------------------------------
qMRMLSortFilterProxyModel* qMRMLTreeWidget::sortFilterProxyModel()const
{
  CTK_D(const qMRMLTreeWidget);
  Q_ASSERT(d->SortFilterModel);
  return d->SortFilterModel;
}

//--------------------------------------------------------------------------
void qMRMLTreeWidget::updateGeometries()
{
  if (!this->isVisible())
    {
    return;
    }
  this->QTreeView::updateGeometries();
}
