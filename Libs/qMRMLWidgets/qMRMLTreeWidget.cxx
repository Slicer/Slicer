
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
  qMRMLTreeWidgetPrivate();
  void init();

  qMRMLSceneModel*          SceneModel;
  qMRMLTransformProxyModel* TransformModel;
};

//------------------------------------------------------------------------------
qMRMLTreeWidgetPrivate::qMRMLTreeWidgetPrivate()
{
  SceneModel = 0;
  this->TransformModel = 0;
}

//------------------------------------------------------------------------------
void qMRMLTreeWidgetPrivate::init()
{
  CTK_P(qMRMLTreeWidget);
  //p->QTreeView::setModel(new qMRMLItemModel(p));
  this->SceneModel = new qMRMLSceneModel(p);
  this->TransformModel = new qMRMLTransformProxyModel(p);
  this->TransformModel->setSourceModel(this->SceneModel);
  p->QTreeView::setModel(this->TransformModel);

  //ctkModelTester * tester = new ctkModelTester(p);
  //tester->setModel(transformModel);
  QObject::connect(p, SIGNAL(activated(const QModelIndex&)),
                   p, SLOT(onActivated(const QModelIndex&)));
  QObject::connect(p, SIGNAL(clicked(const QModelIndex&)),
                   p, SLOT(onActivated(const QModelIndex&)));
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
  Q_ASSERT(d->TransformModel);
  d->TransformModel->setMRMLScene(scene);
  this->expandToDepth(2);
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLTreeWidget::mrmlScene()const
{
  CTK_D(const qMRMLTreeWidget);
  Q_ASSERT(d->TransformModel); // can be removed
  return d->TransformModel->mrmlScene();
}

//------------------------------------------------------------------------------
void qMRMLTreeWidget::onActivated(const QModelIndex& index)
{
  CTK_D(qMRMLTreeWidget);
  Q_ASSERT(d->TransformModel);
  emit currentNodeChanged(d->TransformModel->mrmlNode(index));
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
  d->SceneModel->listenNodeModifiedEvent();
}
