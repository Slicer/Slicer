
// Qt includes
#include <QDebug>

// CTK includes
#include "ctkModelTester.h"

// qMRML includes
#include "qMRMLTreeWidget.h"
//#include "qMRMLItemModel.h"
#include "qMRMLSceneModel.h"
#include "qMRMLTransformProxyModel.h"

//------------------------------------------------------------------------------
class qMRMLTreeWidgetPrivate: public ctkPrivate<qMRMLTreeWidget>
{
public:
  CTK_DECLARE_PUBLIC(qMRMLTreeWidget);
  void init();
};

//------------------------------------------------------------------------------
void qMRMLTreeWidgetPrivate::init()
{
  CTK_P(qMRMLTreeWidget);
  //p->QTreeView::setModel(new qMRMLItemModel(p));
  qMRMLSceneModel* sceneModel = new qMRMLSceneModel(p);
  qMRMLTransformProxyModel* transformModel = new qMRMLTransformProxyModel(p);
  transformModel->setSourceModel(sceneModel);
  p->QTreeView::setModel(transformModel);
  
  //ctkModelTester * tester = new ctkModelTester(p);
  //tester->setModel(transformModel);
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
  qMRMLTransformProxyModel* proxyModel = qobject_cast<qMRMLTransformProxyModel*>(this->model());
  Q_ASSERT(proxyModel);
  proxyModel->setMRMLScene(scene);

}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLTreeWidget::mrmlScene()const
{
  Q_ASSERT(qobject_cast<const qMRMLTreeProxyModel*>(this->model()));
  return qobject_cast<const qMRMLTreeProxyModel*>(this->model())->mrmlScene();
}

