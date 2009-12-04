#include "qMRMLTreeWidget.h"
#include "qMRMLItemModel.h"

#include <QDebug>

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
  p->QTreeView::setModel(new qMRMLItemModel(p));
}

//------------------------------------------------------------------------------
qMRMLTreeWidget::qMRMLTreeWidget(QWidget *parent)
  :QTreeView(parent)
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
  Q_ASSERT(qobject_cast<qMRMLItemModel*>(this->model()));
  qobject_cast<qMRMLItemModel*>(this->model())->setMRMLScene(scene);
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLTreeWidget::mrmlScene()const
{
  Q_ASSERT(qobject_cast<const qMRMLItemModel*>(this->model()));
  return qobject_cast<const qMRMLItemModel*>(this->model())->mrmlScene();
}

//------------------------------------------------------------------------------
void qMRMLTreeWidget::rowsInserted(const QModelIndex & parent, int start, int end)
{
  qDebug() << "qMRMLTreeWidget::rowsInserted " << parent << " " << start << " " << end;
  this->QTreeView::rowsInserted(parent,start,end);
}

//------------------------------------------------------------------------------
void qMRMLTreeWidget::rowsRemoved(const QModelIndex & parent, int start, int end)
{
  qDebug() << "qMRMLTreeWidget::rowsRemoved " << parent << " " << start << " " << end;
  this->QTreeView::rowsRemoved(parent,start,end);
}
