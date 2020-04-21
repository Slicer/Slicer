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
#include <QApplication>

// qMRML includes
#include "qMRMLColorModel_p.h"
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>

// VTK includes

//------------------------------------------------------------------------------
qMRMLColorModelPrivate::qMRMLColorModelPrivate(qMRMLColorModel& object)
  : q_ptr(&object)
{
  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
  this->NoneEnabled = false;
  this->ColorColumn = 0;
  this->LabelColumn = 1;
  this->OpacityColumn = 2;
  this->CheckableColumn = -1;
  this->IsUpdatingWidgetFromMRML = false;
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
  this->updateColumnCount();
  QStringList headerLabels;
  for (int i = 0; i <= this->maxColumnId(); ++i)
    {
    headerLabels << "";
    }
  if (q->colorColumn() != -1)
    {
    headerLabels[q->colorColumn()] = "Color";
    }
  if (q->labelColumn() != -1)
    {
    headerLabels[q->labelColumn()] = "Label";
    }
  if (q->opacityColumn() != -1)
    {
    headerLabels[q->opacityColumn()] = "Opacity";
    }
  q->setHorizontalHeaderLabels(headerLabels);
  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)),
                   q, SLOT(onItemChanged(QStandardItem*)),
                   Qt::UniqueConnection);
}

//------------------------------------------------------------------------------
void qMRMLColorModelPrivate::updateColumnCount()
{
  Q_Q(qMRMLColorModel);
  const int max = this->maxColumnId();
  q->setColumnCount(max + 1);
}

//------------------------------------------------------------------------------
int qMRMLColorModelPrivate::maxColumnId()const
{
  int maxId = 0; // information (scene, node uid... ) are stored in the 1st column
  maxId = qMax(maxId, this->ColorColumn);
  maxId = qMax(maxId, this->LabelColumn);
  maxId = qMax(maxId, this->OpacityColumn);
  maxId = qMax(maxId, this->CheckableColumn);
  return maxId;
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
qMRMLColorModel::~qMRMLColorModel() = default;

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
int qMRMLColorModel::colorColumn()const
{
  Q_D(const qMRMLColorModel);
  return d->ColorColumn;
}

//------------------------------------------------------------------------------
void qMRMLColorModel::setColorColumn(int column)
{
  Q_D(qMRMLColorModel);
  d->ColorColumn = column;
  d->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLColorModel::labelColumn()const
{
  Q_D(const qMRMLColorModel);
  return d->LabelColumn;
}

//------------------------------------------------------------------------------
void qMRMLColorModel::setLabelColumn(int column)
{
  Q_D(qMRMLColorModel);
  d->LabelColumn = column;
  d->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLColorModel::opacityColumn()const
{
  Q_D(const qMRMLColorModel);
  return d->OpacityColumn;
}

//------------------------------------------------------------------------------
void qMRMLColorModel::setOpacityColumn(int column)
{
  Q_D(qMRMLColorModel);
  d->OpacityColumn = column;
  d->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLColorModel::checkableColumn()const
{
  Q_D(const qMRMLColorModel);
  return d->CheckableColumn;
}

//------------------------------------------------------------------------------
void qMRMLColorModel::setCheckableColumn(int column)
{
  Q_D(qMRMLColorModel);
  d->CheckableColumn = column;
  d->updateColumnCount();
}


//------------------------------------------------------------------------------
int qMRMLColorModel::colorFromItem(QStandardItem* colorItem)const
{
  Q_D(const qMRMLColorModel);
  // TODO: fasten by saving the pointer into the data
  if (d->MRMLColorNode == nullptr || colorItem == nullptr)
    {
    return -1;
    }
  QVariant colorIndex = colorItem->data(qMRMLColorModel::ColorEntryRole);
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
    return nullptr;
    }
  QModelIndexList indexes = this->match(this->index(0,0), qMRMLColorModel::ColorEntryRole,
                                      color, 1,
                                      Qt::MatchExactly | Qt::MatchRecursive);
  while (indexes.size())
    {
    if (indexes[0].column() == column)
      {
      return this->itemFromIndex(indexes[0]);
      }
    indexes = this->match(indexes[0], qMRMLColorModel::ColorEntryRole, color, 1,
                          Qt::MatchExactly | Qt::MatchRecursive);
    }
  return nullptr;
}

//------------------------------------------------------------------------------
QModelIndexList qMRMLColorModel::indexes(int color)const
{
  return this->match(this->index(0,0), qMRMLColorModel::ColorEntryRole, color, -1,
                     Qt::MatchExactly | Qt::MatchRecursive);
}

//------------------------------------------------------------------------------
QColor qMRMLColorModel::qcolorFromColor(int entry)const
{
  Q_D(const qMRMLColorModel);
  if (d->MRMLColorNode == nullptr || entry < 0)
    {
    return QColor();
    }
  double rgba[4];
  d->MRMLColorNode->GetColor(entry, rgba);
  return QColor::fromRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
}

//------------------------------------------------------------------------------
QString qMRMLColorModel::nameFromColor(int entry)const
{
  Q_D(const qMRMLColorModel);
  if (d->MRMLColorNode == nullptr || entry < 0)
    {
    return QString();
    }
  return QString(d->MRMLColorNode->GetColorName(entry));
}

//------------------------------------------------------------------------------
int qMRMLColorModel::colorFromName(const QString& name)const
{
  Q_D(const qMRMLColorModel);
  if (d->MRMLColorNode == nullptr)
    {
    return -1;
    }
  return d->MRMLColorNode->GetColorIndexByName(name.toUtf8());
}

//------------------------------------------------------------------------------
void qMRMLColorModel::updateNode()
{
  Q_D(qMRMLColorModel);

  if (d->IsUpdatingWidgetFromMRML)
    {
    // Updating widget from MRML is already in progress
    return;
    }
  d->IsUpdatingWidgetFromMRML = true;

  if (d->MRMLColorNode == nullptr)
    {
    this->setRowCount(this->noneEnabled() ? 1 : 0);
    d->IsUpdatingWidgetFromMRML = false;
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

  d->IsUpdatingWidgetFromMRML = false;
}

//------------------------------------------------------------------------------
void qMRMLColorModel::updateItemFromColor(QStandardItem* item, int color, int column)
{
  Q_D(qMRMLColorModel);
  if (color < 0)
    {
    return;
    }
  item->setData(color, qMRMLColorModel::ColorEntryRole);

  QString colorName = d->MRMLColorNode->GetNamesInitialised() ?
    d->MRMLColorNode->GetColorName(color) : "";
  if (column == d->ColorColumn)
    {
    QPixmap pixmap;
    double rgba[4] = { 0., 0., 0., 1. };
    const bool validColor = d->MRMLColorNode->GetColor(color, rgba);
    if (validColor)
      {
      // It works to set just a QColor but if the model gets into a QComboBox,
      // the currently selected item doesn't get a decoration.
      // TODO: Cache the pixmap as it is expensive to compute and it is done
      // for ALL the colors of the node anytime a color is changed.
      pixmap = qMRMLUtils::createColorPixmap(
        qApp->style(), QColor::fromRgbF(rgba[0], rgba[1], rgba[2]));
      item->setData(pixmap, Qt::DecorationRole);
      item->setData(QColor::fromRgbF(rgba[0], rgba[1], rgba[2]), qMRMLColorModel::ColorRole);
      }
    else
      {
      item->setData(QVariant(), Qt::DecorationRole);
      item->setData(QColor(), qMRMLColorModel::ColorRole);
      }
    item->setData(validColor && column != d->LabelColumn ?
      pixmap.size() : QVariant(), Qt::SizeHintRole);
    item->setToolTip(colorName);
    }
  if (column == d->LabelColumn)
    {
    item->setText(colorName);
    item->setToolTip("");
    }
  if (column == d->OpacityColumn)
    {
    double rgba[4] = { 0., 0., 0., 1. };
    d->MRMLColorNode->GetColor(color, rgba);
    item->setData(QString::number(rgba[3],'f',2), Qt::DisplayRole);
    }
  if (column == d->CheckableColumn)
    {
    item->setCheckable(true);
    }
}

//------------------------------------------------------------------------------
void qMRMLColorModel::updateColorFromItem(int color, QStandardItem* item)
{
  Q_D(qMRMLColorModel);
  vtkMRMLColorTableNode* colorTableNode = vtkMRMLColorTableNode::SafeDownCast(d->MRMLColorNode);
  if (color < 0 || !colorTableNode)
    {
    return;
    }
  if (item->column() == d->ColorColumn)
    {
    QColor rgba(item->data(qMRMLColorModel::ColorRole).value<QColor>());
    colorTableNode->SetColor(color, rgba.redF(), rgba.greenF(), rgba.blueF());
    }
  else if (item->column() == d->LabelColumn)
    {
    colorTableNode->SetColorName(color, item->text().toUtf8());
    }
  else if (item->column() == d->OpacityColumn)
    {
    colorTableNode->SetOpacity(color, item->data(Qt::DisplayRole).toDouble());
    }
}

//-----------------------------------------------------------------------------
void qMRMLColorModel::onMRMLNodeEvent(vtkObject* vtk_obj, unsigned long event,
                                      void* client_data, void* vtkNotUsed(call_data))
{
  vtkMRMLColorNode* colorNode = reinterpret_cast<vtkMRMLColorNode*>(vtk_obj);
  qMRMLColorModel* colorModel = reinterpret_cast<qMRMLColorModel*>(client_data);
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
  Q_UNUSED(colorNode);
  Q_UNUSED(d);
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

//------------------------------------------------------------------------------
QVariant qMRMLColorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  QVariant retval =  QStandardItemModel::headerData(section, orientation, role);

  if (orientation == Qt::Vertical &&
      role == Qt::DisplayRole)
    {
    // for the vertical header, decrement the row number by one, since the
    // rows start from 1 and the indices start from 0 in the color look up
    // table.
    retval = QVariant(retval.toInt() - 1);
    }

  return retval;

}
