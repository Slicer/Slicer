#include "qMRMLTreeWidget.h"
//#include "qMRMLItemModel.h"
#include "qMRMLSceneModel.h"
#include "qMRMLTransformProxyModel.h"
#include <QDebug>
#include "qCTKModelTester.h"

//------------------------------------------------------------------------------
class qMRMLTreeWidgetPrivate: public qCTKPrivate<qMRMLTreeWidget>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLTreeWidget);
  void init();
};

//------------------------------------------------------------------------------
void qMRMLTreeWidgetPrivate::init()
{
  QCTK_P(qMRMLTreeWidget);
  //p->QTreeView::setModel(new qMRMLItemModel(p));
  qMRMLSceneModel* sceneModel = new qMRMLSceneModel(p);
  qMRMLTransformProxyModel* transformModel = new qMRMLTransformProxyModel(p);
  transformModel->setSourceModel(sceneModel);
  p->QTreeView::setModel(transformModel);
  
  //qCTKModelTester* modelTester = new qCTKModelTester(0, p);
  //modelTester->setModel(sceneModel);
  //new qCTKModelTester(sceneModel, p);
  //modelTester->setModel(transformModel);
  new qCTKModelTester(transformModel, p);
  
}

//------------------------------------------------------------------------------
qMRMLTreeWidget::qMRMLTreeWidget(QWidget *_parent)
  :QTreeView(_parent)
{
  QCTK_INIT_PRIVATE(qMRMLTreeWidget);
  qctk_d()->init();
}

//------------------------------------------------------------------------------
qMRMLTreeWidget::~qMRMLTreeWidget()
{

}

//------------------------------------------------------------------------------
void qMRMLTreeWidget::setMRMLScene(vtkMRMLScene* scene)
{
  qMRMLTransformProxyModel* proxyModel = qobject_cast<qMRMLTransformProxyModel*>(this->model());
  
  //qMRMLItemModel* mrmlModel = qobject_cast<qMRMLItemModel*>(this->model());
  //qMRMLSceneModel* mrmlModel = qobject_cast<qMRMLSceneModel*>(this->model());
  qMRMLSceneModel* mrmlModel = qobject_cast<qMRMLSceneModel*>(proxyModel->sourceModel());
  Q_ASSERT(mrmlModel);
  mrmlModel->setMRMLScene(scene);

}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLTreeWidget::mrmlScene()const
{
  Q_ASSERT(qobject_cast<const qMRMLTreeProxyModel*>(this->model()));
  return qobject_cast<const qMRMLTreeProxyModel*>(this->model())->mrmlScene();
}

