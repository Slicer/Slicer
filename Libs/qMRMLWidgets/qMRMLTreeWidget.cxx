
// Qt includes
#include <QDebug>

// CTK includes
#include "ctkModelTester.h"

// qMRML includes
#include "qMRMLSceneModel.h"
#include "qMRMLSortFilterProxyModel.h"
#include "qMRMLTransformProxyModel.h"
#include "qMRMLTreeWidget.h"

//------------------------------------------------------------------------------
class qMRMLTreeWidgetPrivate: public ctkPrivate<qMRMLTreeWidget>
{
public:
  CTK_DECLARE_PUBLIC(qMRMLTreeWidget);
  qMRMLTreeWidgetPrivate();
  void init();

  qMRMLSceneModel*           SceneModel;
  qMRMLTransformProxyModel*  TransformModel;
  qMRMLSortFilterProxyModel* SortFilterModel;
};

//------------------------------------------------------------------------------
qMRMLTreeWidgetPrivate::qMRMLTreeWidgetPrivate()
{
  this->SceneModel = 0;
  this->TransformModel = 0;
  this->SortFilterModel = 0;
}

//------------------------------------------------------------------------------
void qMRMLTreeWidgetPrivate::init()
{
  CTK_P(qMRMLTreeWidget);
  //p->QTreeView::setModel(new qMRMLItemModel(p));
  this->SceneModel = new qMRMLSceneModel(p);
  this->TransformModel = new qMRMLTransformProxyModel(p);
  this->TransformModel->setSourceModel(this->SceneModel);
  this->SortFilterModel = new qMRMLSortFilterProxyModel(p);
  this->SortFilterModel->setSourceModel(this->TransformModel);
  p->QTreeView::setModel(this->SortFilterModel);

  //ctkModelTester * tester = new ctkModelTester(p);
  //tester->setModel(this->SortFilterModel);
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
