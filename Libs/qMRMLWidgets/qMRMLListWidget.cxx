#include "qMRMLListWidget.h"
#include "qMRMLItemModel.h"

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
  p->QListView::setModel(new qMRMLItemModel(p));
  // view signals
  /*
  QObject::connect(q, SIGNAL(pressed(QModelIndex)), q, SLOT(_q_emitItemPressed(QModelIndex)));
  QObject::connect(q, SIGNAL(clicked(QModelIndex)), q, SLOT(_q_emitItemClicked(QModelIndex)));
  QObject::connect(q, SIGNAL(doubleClicked(QModelIndex)),
                   q, SLOT(_q_emitItemDoubleClicked(QModelIndex)));
  QObject::connect(q, SIGNAL(activated(QModelIndex)),
                   q, SLOT(_q_emitItemActivated(QModelIndex)));
  QObject::connect(q, SIGNAL(entered(QModelIndex)), q, SLOT(_q_emitItemEntered(QModelIndex)));
  QObject::connect(model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   q, SLOT(_q_emitItemChanged(QModelIndex)));
  QObject::connect(q->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                   q, SLOT(_q_emitCurrentItemChanged(QModelIndex,QModelIndex)));
  QObject::connect(q->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                   q, SIGNAL(itemSelectionChanged()));
  QObject::connect(model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   q, SLOT(_q_dataChanged(QModelIndex,QModelIndex)));
  QObject::connect(model(), SIGNAL(columnsRemoved(QModelIndex,int,int)), q, SLOT(_q_sort()));
  */
}

//------------------------------------------------------------------------------
qMRMLListWidget::qMRMLListWidget(QWidget *parent)
  :QListView(parent)
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
  Q_ASSERT(qobject_cast<qMRMLItemModel*>(this->model()));
  qobject_cast<qMRMLItemModel*>(this->model())->setMRMLScene(scene);
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLListWidget::mrmlScene()const
{
  Q_ASSERT(qobject_cast<const qMRMLItemModel*>(this->model()));
  return qobject_cast<const qMRMLItemModel*>(this->model())->mrmlScene();
}

