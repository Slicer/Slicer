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
#include <QSortFilterProxyModel>

// CTK includes

// qMRML includes
#include "qMRMLListWidget.h"
//#include "qMRMLItemModel.h"
#include "qMRMLSceneTransformModel.h"

//------------------------------------------------------------------------------
class qMRMLListWidgetPrivate
{
  Q_DECLARE_PUBLIC(qMRMLListWidget);
protected:
  qMRMLListWidget* const q_ptr;
public:
  qMRMLListWidgetPrivate(qMRMLListWidget& object);
  void init();
};

//------------------------------------------------------------------------------
qMRMLListWidgetPrivate::qMRMLListWidgetPrivate(qMRMLListWidget& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qMRMLListWidgetPrivate::init()
{
  Q_Q(qMRMLListWidget);
  //p->QListView::setModel(new qMRMLItemModel(p));
  //p->QListView::setModel(new qMRMLSceneModel(p));
  ///new ctkModelTester(p->model(), p);

  qMRMLSceneTransformModel* sceneModel = new qMRMLSceneTransformModel(q);
  QSortFilterProxyModel* sortModel = new QSortFilterProxyModel(q);
  sortModel->setSourceModel(sceneModel);
  sortModel->setDynamicSortFilter(true);
  q->QListView::setModel(sortModel);
  q->setWrapping(true);
  q->setResizeMode(QListView::Adjust);
  q->setFlow(QListView::TopToBottom);
  // We have a problem when the model is reset (qMRMLSceneModel::setMRMLScene(0)),
  // the QSortFilterProxyModel doesn't realize that the rows have disappeared
  // and QSortFilterProxyModel::rowCount(QModelIndex) returns 1(the mrmlscene), which
  // is eventually called by the ctkModelTester slot connected to QSortFilterProxyModel
  // signal layoutAboutToBeChanged() which eventually calls testData on the valid QModelIndex
  //new ctkModelTester(p->model(), p);

  //ctkModelTester* tester = new ctkModelTester(p);
  //tester->setModel(transformModel);
}

//------------------------------------------------------------------------------
qMRMLListWidget::qMRMLListWidget(QWidget *_parent)
  : QListView(_parent)
  , d_ptr(new qMRMLListWidgetPrivate(*this))
{
  Q_D(qMRMLListWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLListWidget::~qMRMLListWidget() = default;

//------------------------------------------------------------------------------
void qMRMLListWidget::setMRMLScene(vtkMRMLScene* scene)
{
  QSortFilterProxyModel* sortModel = qobject_cast<QSortFilterProxyModel*>(this->model());
  qMRMLSceneModel* mrmlModel = qobject_cast<qMRMLSceneModel*>(sortModel->sourceModel());
  Q_ASSERT(mrmlModel);

  mrmlModel->setMRMLScene(scene);
  if (scene)
    {
    this->setRootIndex(sortModel->index(0, 0));
    sortModel->sort(0);
    sortModel->invalidate();
    }
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLListWidget::mrmlScene()const
{
  QSortFilterProxyModel* sortModel = qobject_cast<QSortFilterProxyModel*>(this->model());
  Q_ASSERT(qobject_cast<const qMRMLSceneModel*>(sortModel->sourceModel()));
  return qobject_cast<const qMRMLSceneModel*>(sortModel->sourceModel())->mrmlScene();
}
