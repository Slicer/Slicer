#include "qMRMLListWidget.h"
//#include "qMRMLItemModel.h"
#include "qMRMLSceneModel.h"
#include "qMRMLTransformProxyModel.h"

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
  p->QListView::setModel(transformModel);
  new qCTKModelTester(p->model(), p);
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
  qMRMLTransformProxyModel* proxyModel = qobject_cast<qMRMLTransformProxyModel*>(this->model());
  
  //qMRMLItemModel* mrmlModel = qobject_cast<qMRMLItemModel*>(this->model());
  //qMRMLSceneModel* mrmlModel = qobject_cast<qMRMLSceneModel*>(this->model());
  qMRMLSceneModel* mrmlModel = qobject_cast<qMRMLSceneModel*>(proxyModel->sourceModel());
  Q_ASSERT(mrmlModel);
  mrmlModel->setMRMLScene(scene);
  //if (mrmlModel->topLevelScene())
    {
    this->setRootIndex(proxyModel->index(0, 0));
    }
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLListWidget::mrmlScene()const
{
  Q_ASSERT(qobject_cast<const qMRMLTreeProxyModel*>(this->model()));
  return qobject_cast<const qMRMLTreeProxyModel*>(this->model())->mrmlScene();
}
