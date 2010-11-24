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
#include "qMRMLSceneAnnotationModel.h"
#include "qMRMLTreeWidget.h"

#include "qMRMLAnnotationTreeWidget.h"

//------------------------------------------------------------------------------
class qMRMLAnnotationTreeWidgetPrivate
{
  Q_DECLARE_PUBLIC(qMRMLAnnotationTreeWidget);
protected:
  qMRMLAnnotationTreeWidget* const q_ptr;
public:
  qMRMLAnnotationTreeWidgetPrivate(qMRMLAnnotationTreeWidget& object);
  void init();

  qMRMLSceneModel*           SceneModel;
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

  q->setUniformRowHeights(true);
}

//------------------------------------------------------------------------------
qMRMLAnnotationTreeWidget::qMRMLAnnotationTreeWidget(QWidget *_parent)
  :qMRMLTreeWidget(_parent)
  , d_ptr(new qMRMLAnnotationTreeWidgetPrivate(*this))
{
  Q_D(qMRMLAnnotationTreeWidget);
  d->init();
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
  this->hideColumn(1);
  this->expandAll();
}

//------------------------------------------------------------------------------
void qMRMLAnnotationTreeWidget::onActivated(const QModelIndex& index)
{
  Q_D(qMRMLAnnotationTreeWidget);
  Q_ASSERT(d->SortFilterModel);
  emit currentNodeChanged(d->SortFilterModel->mrmlNode(index));
}
