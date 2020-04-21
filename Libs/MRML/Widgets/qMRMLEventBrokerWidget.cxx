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
#include <QTreeWidget>
#include <QVBoxLayout>

// CTK includes
#include <ctkVTKConnection_p.h>

// qMRML includes
#include "qMRMLEventBrokerWidget.h"
#include "qMRMLWidgetsConfigure.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLNode.h>
#include <vtkObservation.h>

// VTK includes
#include <vtkCallbackCommand.h>

// STD includes
#ifdef HAVE_STDINT_H
# include <stdint.h> // For intptr_t
#endif

namespace
{
  enum ColumnType
  {
    NameColumn = 0,
    ElapsedTimeColumn,
    TotalTimeColumn,
    CommentColumn
  };
}

//-----------------------------------------------------------------------------
class qMRMLEventBrokerWidgetPrivate
{
public:
  qMRMLEventBrokerWidgetPrivate();
  void setupUi(QWidget*);

  vtkObservation* observation(QTreeWidgetItem* item)const;

  QTreeWidgetItem* itemFromSubject(vtkObject* subject)const;
  QTreeWidgetItem* itemFromEvent(QTreeWidgetItem* subjectItem, unsigned long long event)const;
  QTreeWidgetItem* itemFromObservation(vtkObservation* observation)const;

  vtkObject* objectFromItem(QTreeWidgetItem*)const;

  void setObjectToItem(QTreeWidgetItem* item, vtkObject* object)const;
  void showItem(QTreeWidgetItem* item);

  void addObservation(vtkObservation* observation);

  QTreeWidget* ConnectionsTreeWidget;
};

//------------------------------------------------------------------------------
qMRMLEventBrokerWidgetPrivate::qMRMLEventBrokerWidgetPrivate()
{
  this->ConnectionsTreeWidget = nullptr;
}

//------------------------------------------------------------------------------
vtkObservation* qMRMLEventBrokerWidgetPrivate::observation(QTreeWidgetItem* item)const
{
  if (!item || !item->parent() || !item->parent()->parent())
    {
    return nullptr;
    }
  QTreeWidgetItem* eventItem = item->parent();
  unsigned long event = eventItem->text(NameColumn).toInt();

  QTreeWidgetItem* subjectItem = eventItem->parent();
  vtkObject* subject = static_cast<vtkObject*>(
    reinterpret_cast<void *>(
      subjectItem->data(0, Qt::UserRole).toLongLong()));

  vtkObject* observer = static_cast<vtkObject*>(
    reinterpret_cast<void *>(
      item->data(0, Qt::UserRole).toLongLong()));

  vtkEventBroker::ObservationVector observations =
    vtkEventBroker::GetInstance()->GetObservations(subject, event, observer, nullptr, 1);
  Q_ASSERT(observations.size());
  return observations.size() ? (*observations.begin()) : 0;
}

//------------------------------------------------------------------------------
QTreeWidgetItem* qMRMLEventBrokerWidgetPrivate::itemFromSubject(vtkObject* subject)const
{
  if (!subject)
    {
    return nullptr;
    }

  for (int i = 0; i < this->ConnectionsTreeWidget->topLevelItemCount(); ++i)
    {
    QTreeWidgetItem* topLevelItem = this->ConnectionsTreeWidget->topLevelItem(i);
    if (static_cast<vtkObject*>(reinterpret_cast<void *>(
      topLevelItem->data(0, Qt::UserRole).toLongLong())) == subject)
      {
      return topLevelItem;
      }
    }
  QTreeWidgetItem* subjectItem = new QTreeWidgetItem;
  this->setObjectToItem(subjectItem, subject);
  this->ConnectionsTreeWidget->addTopLevelItem(subjectItem);
  return subjectItem;
}

//------------------------------------------------------------------------------
QTreeWidgetItem* qMRMLEventBrokerWidgetPrivate::itemFromEvent(QTreeWidgetItem* subjectItem, unsigned long long event)const
{
  if (!subjectItem)
    {
    return nullptr;
    }

  QString eventString = QString::number(event);

  for (int i = 0; i < subjectItem->childCount(); ++i)
    {
    QTreeWidgetItem* eventItem = subjectItem->child(i);
    if (eventItem->text(NameColumn) == eventString)
      {
      return eventItem;
      }
    }

  QTreeWidgetItem* eventItem = new QTreeWidgetItem();
  eventItem->setText(NameColumn, eventString);
  eventItem->setToolTip(NameColumn, vtkCommand::GetStringFromEventId(event));
  subjectItem->addChild(eventItem);
  return eventItem;
}

//------------------------------------------------------------------------------
QTreeWidgetItem* qMRMLEventBrokerWidgetPrivate::itemFromObservation(vtkObservation* observation)const
{
  if (!observation)
    {
    return nullptr;
    }

  QTreeWidgetItem* subjectItem  = this->itemFromSubject(observation->GetSubject());
  QTreeWidgetItem* eventItem = this->itemFromEvent(subjectItem, observation->GetEvent());

  for (int i = 0; i < eventItem->childCount(); ++i)
    {
    QTreeWidgetItem* observationItem = eventItem->child(i);
    unsigned long long userData = observationItem->data(NameColumn, Qt::UserRole).toLongLong() ;
    if ((observation->GetScript() && userData == 0 &&
         observationItem->toolTip(NameColumn) == observation->GetScript()) ||
        (observation->GetObserver() && userData != 0 &&
         static_cast<vtkObject*>(reinterpret_cast<void *>(userData)) == observation->GetObserver()))
      {
      return observationItem;
      }
    }
  return nullptr;
}

//------------------------------------------------------------------------------
void qMRMLEventBrokerWidgetPrivate::setObjectToItem(QTreeWidgetItem* item, vtkObject* object)const
{
  item->setData(NameColumn, Qt::UserRole,
    QVariant::fromValue(reinterpret_cast<long long>(object)));
  vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(object);
  if (node)
    {
    // vtkMRMLNode
    item->setText(NameColumn, node->GetName());
    item->setToolTip(NameColumn, node->GetID());
    }
  if (item->text(NameColumn).isEmpty() && object)
    {
    // vtkObject
    item->setText(NameColumn, object->GetClassName());
    }
  if (item->toolTip(NameColumn).isEmpty() && object)
    {
    // vtkObject
    item->setToolTip(NameColumn,
      "0x" + QString::number(reinterpret_cast<intptr_t>(object), 16));
    }
}

//------------------------------------------------------------------------------
vtkObject* qMRMLEventBrokerWidgetPrivate::objectFromItem(QTreeWidgetItem* item)const
{
  if (!item)
    {
    return nullptr;
    }
  return reinterpret_cast<vtkObject*>(item->data(NameColumn, Qt::UserRole).value<long long>());
}

//------------------------------------------------------------------------------
void qMRMLEventBrokerWidgetPrivate::showItem(QTreeWidgetItem* item)
{
  if (!item)
    {
    return;
    }
  this->ConnectionsTreeWidget->expandItem(item);
  this->showItem(item->parent());
}

//------------------------------------------------------------------------------
void qMRMLEventBrokerWidgetPrivate::addObservation(vtkObservation* observation)
{
  // Observation
  QTreeWidgetItem* observationItem = new QTreeWidgetItem;

  vtkObject* observer = observation->GetObserver();
  this->setObjectToItem(observationItem, observer);

  if (observation->GetScript())
    {
    // Script
    observationItem->setText(NameColumn, "Script");
    observationItem->setToolTip(NameColumn, observation->GetScript());
    }
  else if (!observation->GetObserver())
    {
    ctkVTKConnectionPrivate* connection = reinterpret_cast<ctkVTKConnectionPrivate*>(
      observation->GetCallbackCommand()->GetClientData());
    observationItem->setText(NameColumn, connection->QtObject->metaObject()->className());
    observationItem->setToolTip(NameColumn, "0x" + QString::number(reinterpret_cast<intptr_t>(connection->QtObject), 16));
    }
  // Elapsed Time
  observationItem->setText(ElapsedTimeColumn, QString::number(observation->GetLastElapsedTime()) + " s");
  observationItem->setToolTip(ElapsedTimeColumn, QString::number(1. / observation->GetLastElapsedTime()) + " fps");
  // Total Time
  observationItem->setText(TotalTimeColumn, QString::number(observation->GetTotalElapsedTime()) + " s");
  observationItem->setToolTip(TotalTimeColumn, QString::number(1. / observation->GetTotalElapsedTime()) + " fps");
  observationItem->setFlags(observationItem->flags() | Qt::ItemIsEditable);
  // Comments
  observationItem->setText(CommentColumn, observation->GetComment());

  // Event
  QTreeWidgetItem* subjectItem  = this->itemFromSubject(observation->GetSubject());
  QTreeWidgetItem* eventItem = this->itemFromEvent(subjectItem, observation->GetEvent());
  eventItem->addChild(observationItem);
}

//------------------------------------------------------------------------------
void qMRMLEventBrokerWidgetPrivate::setupUi(QWidget* parentWidget)
{
  this->ConnectionsTreeWidget = new QTreeWidget;

  QStringList headers;
  headers << "Object/Type"  << "Elapsed" << "Total" << "Comment";
  this->ConnectionsTreeWidget->setHeaderLabels(headers);

  QObject::connect(this->ConnectionsTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
                   parentWidget, SLOT(onItemChanged(QTreeWidgetItem*,int)));
  QObject::connect(this->ConnectionsTreeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
                   parentWidget, SLOT(onCurrentItemChanged(QTreeWidgetItem*)));

  QVBoxLayout* vBoxLayout = new QVBoxLayout;
  vBoxLayout->addWidget(this->ConnectionsTreeWidget);
  vBoxLayout->setContentsMargins(0, 0, 0, 0);
  parentWidget->setLayout(vBoxLayout);

}

//------------------------------------------------------------------------------
qMRMLEventBrokerWidget::qMRMLEventBrokerWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLEventBrokerWidgetPrivate)
{
  Q_D(qMRMLEventBrokerWidget);

  d->setupUi(this);
}

//------------------------------------------------------------------------------
qMRMLEventBrokerWidget::~qMRMLEventBrokerWidget() = default;

//------------------------------------------------------------------------------
void qMRMLEventBrokerWidget::refresh()
{
  Q_D(qMRMLEventBrokerWidget);
  d->ConnectionsTreeWidget->clear();
  vtkEventBroker* eventBroker = vtkEventBroker::GetInstance();
  if (!eventBroker)
    {
    return;
    }
  bool isSortingEnabled = d->ConnectionsTreeWidget->isSortingEnabled();
  d->ConnectionsTreeWidget->setSortingEnabled(false);

  for (int i = 0; i < eventBroker->GetNumberOfObservations(); ++i)
    {
    vtkObservation* observation = eventBroker->GetNthObservation(i);
    d->addObservation(observation);
    }
  d->ConnectionsTreeWidget->setSortingEnabled(isSortingEnabled);
  //d->ConnectionsTreeWidget->expandAll();
  d->ConnectionsTreeWidget->resizeColumnToContents(0);
}

//------------------------------------------------------------------------------
void qMRMLEventBrokerWidget::onItemChanged(QTreeWidgetItem* item, int column)
{
  Q_D(qMRMLEventBrokerWidget);
  vtkObservation* observation = d->observation(item);
  if (!observation)
    {
    return;
    }
  if (column == TotalTimeColumn || column == ElapsedTimeColumn)
    {
    QString newTotalTime = item->text(column);
    if (newTotalTime.endsWith("s"))
      {
      newTotalTime.remove(newTotalTime.size() - 1, 1);
      }
    bool ok = false;
    double newTotal = newTotalTime.toDouble(&ok);
    if (ok)
      {
      if ( column == TotalTimeColumn )
        {
        observation->SetTotalElapsedTime(newTotal);
        }
      else if (column == ElapsedTimeColumn )
        {
        observation->SetLastElapsedTime(newTotal);
        }
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLEventBrokerWidget::onCurrentItemChanged(QTreeWidgetItem* currentItem)
{
  Q_D(qMRMLEventBrokerWidget);
  emit this->currentObjectChanged(d->objectFromItem(currentItem));
}

//------------------------------------------------------------------------------
void qMRMLEventBrokerWidget::resetElapsedTimes()
{
  vtkEventBroker* eventBroker = vtkEventBroker::GetInstance();
  for (int i = 0; i < eventBroker->GetNumberOfObservations(); ++i)
    {
    vtkObservation* observation = eventBroker->GetNthObservation(i);
    observation->SetTotalElapsedTime(0.);
    observation->SetLastElapsedTime(0.);
    }
  this->refresh();
}

//------------------------------------------------------------------------------
void qMRMLEventBrokerWidget::expandElapsedTimeItems()
{
  Q_D(qMRMLEventBrokerWidget);
  this->refresh();
  vtkEventBroker* eventBroker = vtkEventBroker::GetInstance();
  for (int i = 0; i < eventBroker->GetNumberOfObservations(); ++i)
    {
    vtkObservation* observation = eventBroker->GetNthObservation(i);
    if (observation->GetTotalElapsedTime() > 0.0)
      {
      QTreeWidgetItem* observationItem = d->itemFromObservation(observation);
      d->showItem(observationItem);
      }
    }
}
