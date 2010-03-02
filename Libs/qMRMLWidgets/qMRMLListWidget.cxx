#include "qMRMLListWidget.h"
//#include "qMRMLItemModel.h"
#include "qMRMLSceneModel.h"
#include "qMRMLTransformProxyModel.h"
#include <QSortFilterProxyModel>
#include "qCTKModelTester.h"

//------------------------------------------------------------------------------
class qMRMLListWidgetPrivate: public qCTKPrivate<qMRMLListWidget>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLListWidget);
  void init();
};

//------------------------------------------------------------------------------
void qMRMLListWidgetPrivate::init()
{
  QCTK_P(qMRMLListWidget);
  //p->QListView::setModel(new qMRMLItemModel(p));
  //p->QListView::setModel(new qMRMLSceneModel(p));
  ///new qCTKModelTester(p->model(), p);
  
  qMRMLSceneModel* sceneModel = new qMRMLSceneModel(p);
  qMRMLTransformProxyModel* transformModel = new qMRMLTransformProxyModel(p);
  transformModel->setSourceModel(sceneModel);
  QSortFilterProxyModel* sortModel = new QSortFilterProxyModel(p);
  sortModel->setSourceModel(transformModel);
  sortModel->setDynamicSortFilter(true);
  p->QListView::setModel(sortModel);
  p->setWrapping(true);
  p->setResizeMode(QListView::Adjust);
  p->setFlow(QListView::TopToBottom);
  // We have a problem when the model is reset (qMRMLSceneModel::setMRMLScene(0)), 
  // the QSortFilterProxyModel doesn't realize that the rows have disappeared 
  // and QSortFilterProxyModel::rowCount(QModelIndex) returns 1(the mrmlscene), which
  // is eventually called by the qCTKModelTester slot connected to QSortFilterProxyModel
  // signal layoutAboutToBeChanged() which eventually calls testData on the valid QModelIndex
  //new qCTKModelTester(p->model(), p);
  new qCTKModelTester(transformModel,p);
}

//------------------------------------------------------------------------------
qMRMLListWidget::qMRMLListWidget(QWidget *_parent)
  :QListView(_parent)
{
  QCTK_INIT_PRIVATE(qMRMLListWidget);
  qctk_d()->init();
}

//------------------------------------------------------------------------------
qMRMLListWidget::~qMRMLListWidget()
{

}

//------------------------------------------------------------------------------
void qMRMLListWidget::setMRMLScene(vtkMRMLScene* scene)
{
  QSortFilterProxyModel* sortModel = qobject_cast<QSortFilterProxyModel*>(this->model());
  qMRMLTransformProxyModel* proxyModel = qobject_cast<qMRMLTransformProxyModel*>(sortModel->sourceModel());
  
  //qMRMLItemModel* mrmlModel = qobject_cast<qMRMLItemModel*>(this->model());
  //qMRMLSceneModel* mrmlModel = qobject_cast<qMRMLSceneModel*>(this->model());
  qMRMLSceneModel* mrmlModel = qobject_cast<qMRMLSceneModel*>(proxyModel->sourceModel());
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
  Q_ASSERT(qobject_cast<const qMRMLTreeProxyModel*>(sortModel->sourceModel()));
  return qobject_cast<const qMRMLTreeProxyModel*>(sortModel->sourceModel())->mrmlScene();
}
