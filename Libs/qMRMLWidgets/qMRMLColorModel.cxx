/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QMimeData>
#include <QStringList>
#include <QSharedPointer>
#include <QVector>
#include <QMap>

// qMRML includes
#include "qMRMLColorModel.h"
#include "qMRMLColorModel_p.h"
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLColorNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>

//------------------------------------------------------------------------------
qMRMLColorModelPrivate::qMRMLColorModelPrivate(qMRMLColorModel& object)
  : q_ptr(&object)
{
  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
  this->NoneEnabled = false;
}

//------------------------------------------------------------------------------
qMRMLColorModelPrivate::~qMRMLColorModelPrivate()
{
  if (this->MRMLColorNode)
    {
    this->MRMLColorNode->RemoveObserver(this->CallBack);
    }
}

//------------------------------------------------------------------------------
void qMRMLColorModelPrivate::init()
{
  Q_Q(qMRMLColorModel);
  this->CallBack->SetClientData(q);
  this->CallBack->SetCallback(qMRMLColorModel::onMRMLNodeEvent);
  q->setColumnCount(2);
  q->setHorizontalHeaderLabels(QStringList() << "Color" << "Opacity");
  
  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)),
                   q, SLOT(onItemChanged(QStandardItem*)));
}

//------------------------------------------------------------------------------
// qMRMLColorModel
//------------------------------------------------------------------------------
qMRMLColorModel::qMRMLColorModel(QObject *_parent)
  : QStandardItemModel(_parent)
  , d_ptr(new qMRMLColorModelPrivate(*this))
{
  Q_D(qMRMLColorModel);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLColorModel::qMRMLColorModel(qMRMLColorModelPrivate* pimpl, QObject *parentObject)
  : QStandardItemModel(parentObject)
  , d_ptr(pimpl)
{
  Q_D(qMRMLColorModel);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLColorModel::~qMRMLColorModel()
{
}

//------------------------------------------------------------------------------
void qMRMLColorModel::setMRMLColorNode(vtkMRMLColorNode* colorNode)
{
  Q_D(qMRMLColorModel);
  if (d->MRMLColorNode)
    {
    d->MRMLColorNode->RemoveObserver(d->CallBack);
    }
  if (colorNode)
    {
    colorNode->AddObserver(vtkCommand::ModifiedEvent, d->CallBack);
    }
  d->MRMLColorNode = colorNode;
  this->updateNode();
}

//------------------------------------------------------------------------------
vtkMRMLColorNode* qMRMLColorModel::mrmlColorNode()const
{
  Q_D(const qMRMLColorModel);
  return d->MRMLColorNode;
}

//------------------------------------------------------------------------------
void qMRMLColorModel::setNoneEnabled(bool enable)
{
  Q_D(qMRMLColorModel);
  if (this->noneEnabled() == enable)
    {
    return;
    }
  d->NoneEnabled = enable;
  if (enable)
    {
    this->insertRow(0, new QStandardItem(tr("None")));
    }
  else
    {
    this->removeRow(0);
    }
  this->updateNode();
}

//------------------------------------------------------------------------------
bool qMRMLColorModel::noneEnabled()const
{
  Q_D(const qMRMLColorModel);
  return d->NoneEnabled;
}

//------------------------------------------------------------------------------
int qMRMLColorModel::colorFromItem(QStandardItem* colorItem)const
{
  Q_D(const qMRMLColorModel);
  // TODO: fasten by saving the pointer into the data
  if (d->MRMLColorNode == 0 || colorItem == 0)
    {
    return -1;
    }
  QVariant colorIndex = colorItem->data(Qt::UserRole);
  if (!colorIndex.isValid())
    {
    return -1;
    }
  return colorIndex.toInt();
}

//------------------------------------------------------------------------------
QStandardItem* qMRMLColorModel::itemFromColor(int color, int column)const
{
  //Q_D(const qMRMLColorModel);
  if (color == -1)
    {
    return 0;
    }
  QModelIndexList indexes = this->match(QModelIndex(), Qt::UserRole,
                                      color, 1,
                                      Qt::MatchExactly | Qt::MatchRecursive);
  while (indexes.size())
    {
    if (indexes[0].column() == column)
      {
      return this->itemFromIndex(indexes[0]);
      }
    indexes = this->match(indexes[0], Qt::UserRole, color, 1,
                          Qt::MatchExactly | Qt::MatchRecursive);
    }
  return 0;
}

//------------------------------------------------------------------------------
QModelIndexList qMRMLColorModel::indexes(int color)const
{
  return this->match(QModelIndex(), Qt::UserRole, color, -1,
                     Qt::MatchExactly | Qt::MatchRecursive);
}

//------------------------------------------------------------------------------
void qMRMLColorModel::updateNode()
{
  Q_D(qMRMLColorModel);

  if (d->MRMLColorNode == 0)
    {
    this->setRowCount(this->noneEnabled() ? 1 : 0);
    return;
    }

  QObject::disconnect(this, SIGNAL(itemChanged(QStandardItem*)),
                      this, SLOT(onItemChanged(QStandardItem*)));

  this->setRowCount(
    d->MRMLColorNode->GetNumberOfColors() + (this->noneEnabled() ? 1 : 0));
  int startIndex = (this->noneEnabled() ? 1 : 0);
  for (int color = 0; color < d->MRMLColorNode->GetNumberOfColors(); ++color)
    {
    for (int j= 0; j < this->columnCount(); ++j)
      {
      QStandardItem* colorItem = this->invisibleRootItem()->child(
        color + startIndex, j);
      if (!colorItem)
        {
        colorItem = new QStandardItem();
        this->invisibleRootItem()->setChild(color + startIndex,j,colorItem);
        }
      this->updateItemFromColor(colorItem, color, j);
      }
    }

  QObject::connect(this, SIGNAL(itemChanged(QStandardItem*)),
                   this, SLOT(onItemChanged(QStandardItem*)),
                   Qt::UniqueConnection);
}
/*
//------------------------------------------------------------------------------
void qMRMLColorModel::populateNode()
{
  Q_D(qMRMLColorModel);
  Q_ASSERT(d->MRMLColorNode);
  // Add nodes
  for (int i = 0; i < d->MRMLColorNode->GetNumberOfColors(); ++i)
    {
    this->insertColor(i);
    }
}

//------------------------------------------------------------------------------
void qMRMLColorModel::insertColor(int color)
{
  Q_D(qMRMLColorModel);
  Q_ASSERT(color >= 0);
  QStandardItem* parent = this->mrmlColorNodeItem();
  
  QList<QStandardItem*> items;
  for (int i= 0; i < this->columnCount(); ++i)
    {
    QStandardItem* newItem = new QStandardItem();
    this->updateItemFromColor(newItem, color, i);
    items.append(newItem);
    }

  this->mrmlColorNodeItem()->appendRow(items);
}
*/
//------------------------------------------------------------------------------
void qMRMLColorModel::updateItemFromColor(QStandardItem* item, int color, int column)
{
  Q_D(qMRMLColorModel);
  if (color < 0)
    {
    return;
    }
  item->setData(color, Qt::UserRole);
  double rgba[4] = {0.,0.,0.,1.};
  bool validColor = d->MRMLColorNode->GetColor(color, rgba);
  switch (column)
    {
    case 0:
    default:
      {
      item->setText(QString(d->MRMLColorNode->GetColorName(color)));
      Q_ASSERT(!item->text().isEmpty());
      if (validColor)
        {
        // it works to set just a QColor but if the model gets into a QComboBox,
        // the currently selected item doesn't get a decoration
        //item->setData(QColor::fromRgbF(rgba[0], rgba[1], rgba[2]), Qt::DecorationRole);
        QIcon icon = qMRMLUtils::createColorPixmap(
          qApp->style(), QColor::fromRgbF(rgba[0], rgba[1], rgba[2]));
        item->setData(icon, Qt::DecorationRole);
        }
      else
        {
        item->setData(QVariant(), Qt::DecorationRole);
        }
      break;
      }
    case 1:
      item->setText(QString::number(rgba[3]));
      break;
    }
}

//------------------------------------------------------------------------------
void qMRMLColorModel::updateColorFromItem(int color, QStandardItem* item)
{
  Q_D(qMRMLColorModel);
  if (color < 0)
    {
    return;
    }
  switch(item->column())
    {
    case 0:
    default:
      d->MRMLColorNode->SetColorName(color, item->text().toLatin1());
      break;
    case 1:
      //d->MRMLColor
      break;
    }
}

//-----------------------------------------------------------------------------
void qMRMLColorModel::onMRMLNodeEvent(vtkObject* vtk_obj, unsigned long event,
                                        void* client_data, void* call_data)
{
  vtkMRMLColorNode* colorNode = reinterpret_cast<vtkMRMLColorNode*>(vtk_obj);
  qMRMLColorModel* colorModel = reinterpret_cast<qMRMLColorModel*>(client_data);
  int color = reinterpret_cast<int>(call_data);
  Q_ASSERT(colorNode);
  Q_ASSERT(colorModel);
  switch(event)
    {
    default:
    case vtkCommand::ModifiedEvent:
      colorModel->onMRMLColorNodeModified(colorNode);
      break;
    }
}

//------------------------------------------------------------------------------
void qMRMLColorModel::onMRMLColorNodeModified(vtkObject* node)
{
  Q_D(qMRMLColorModel);
  vtkMRMLColorNode* colorNode = vtkMRMLColorNode::SafeDownCast(node);
  Q_ASSERT(colorNode == d->MRMLColorNode);
  this->updateNode();
}

//------------------------------------------------------------------------------
void qMRMLColorModel::onItemChanged(QStandardItem * item)
{
  if (item == this->invisibleRootItem())
    {
    return;
    }
  int color = this->colorFromItem(item);
  this->updateColorFromItem(color, item);
}
