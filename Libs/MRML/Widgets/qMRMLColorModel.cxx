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
#include "qMRMLItemDelegate.h"
#include "qMRMLUtils.h"

// MRML includes
#include <vtkCodedEntry.h>
#include <vtkMRMLColorTableNode.h>
#include "vtkMRMLI18N.h"

//------------------------------------------------------------------------------
qMRMLColorModelPrivate::qMRMLColorModelPrivate(qMRMLColorModel& object)
  : q_ptr(&object)
{
  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
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
    headerLabels[q->colorColumn()] = qMRMLColorModel::tr("Color");
  }
  if (q->labelColumn() != -1)
  {
    headerLabels[q->labelColumn()] = qMRMLColorModel::tr("Label");
  }
  if (q->opacityColumn() != -1)
  {
    headerLabels[q->opacityColumn()] = qMRMLColorModel::tr("Opacity");
  }
  if (q->terminologyColumn() != -1)
  {
    headerLabels[q->terminologyColumn()] = qMRMLColorModel::tr("Terminology");
  }

  q->setHorizontalHeaderLabels(headerLabels);
  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)), q, SLOT(onItemChanged(QStandardItem*)), Qt::UniqueConnection);
}

//------------------------------------------------------------------------------
void qMRMLColorModelPrivate::updateColumnCount()
{
  Q_Q(qMRMLColorModel);
  const int max = this->maxColumnId();
  q->setColumnCount(max + 1);
}

//------------------------------------------------------------------------------
int qMRMLColorModelPrivate::maxColumnId() const
{
  int maxId = 0; // information (scene, node uid... ) are stored in the 1st column
  maxId = qMax(maxId, this->ColorColumn);
  maxId = qMax(maxId, this->LabelColumn);
  maxId = qMax(maxId, this->OpacityColumn);
  maxId = qMax(maxId, this->TerminologyColumn);
  maxId = qMax(maxId, this->CheckableColumn);
  return maxId;
}

//------------------------------------------------------------------------------
// qMRMLColorModel
//------------------------------------------------------------------------------
qMRMLColorModel::qMRMLColorModel(QObject* _parent)
  : QStandardItemModel(_parent)
  , d_ptr(new qMRMLColorModelPrivate(*this))
{
  Q_D(qMRMLColorModel);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLColorModel::qMRMLColorModel(qMRMLColorModelPrivate* pimpl, QObject* parentObject)
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
vtkMRMLColorNode* qMRMLColorModel::mrmlColorNode() const
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
  emit noneEnabledChanged(enable);
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
bool qMRMLColorModel::noneEnabled() const
{
  Q_D(const qMRMLColorModel);
  return d->NoneEnabled;
}

//------------------------------------------------------------------------------
int qMRMLColorModel::colorColumn() const
{
  Q_D(const qMRMLColorModel);
  return d->ColorColumn;
}

//------------------------------------------------------------------------------
void qMRMLColorModel::setColorColumn(int column)
{
  Q_D(qMRMLColorModel);
  d->ColorColumn = column;
  emit colorColumnChanged(column);
  d->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLColorModel::labelColumn() const
{
  Q_D(const qMRMLColorModel);
  return d->LabelColumn;
}

//------------------------------------------------------------------------------
void qMRMLColorModel::setLabelColumn(int column)
{
  Q_D(qMRMLColorModel);
  d->LabelColumn = column;
  emit labelColumnChanged(column);
  d->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLColorModel::opacityColumn() const
{
  Q_D(const qMRMLColorModel);
  return d->OpacityColumn;
}

//------------------------------------------------------------------------------
void qMRMLColorModel::setOpacityColumn(int column)
{
  Q_D(qMRMLColorModel);
  d->OpacityColumn = column;
  emit opacityColumnChanged(column);
  d->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLColorModel::terminologyColumn() const
{
  Q_D(const qMRMLColorModel);
  return d->TerminologyColumn;
}

//------------------------------------------------------------------------------
void qMRMLColorModel::setTerminologyColumn(int column)
{
  Q_D(qMRMLColorModel);
  d->TerminologyColumn = column;
  d->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLColorModel::checkableColumn() const
{
  Q_D(const qMRMLColorModel);
  return d->CheckableColumn;
}

//------------------------------------------------------------------------------
void qMRMLColorModel::setCheckableColumn(int column)
{
  Q_D(qMRMLColorModel);
  d->CheckableColumn = column;
  emit checkableColumnChanged(column);
  d->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLColorModel::colorFromItem(QStandardItem* colorItem) const
{
  Q_D(const qMRMLColorModel);
  // TODO: fasten by saving the pointer into the data
  if (d->MRMLColorNode == nullptr || colorItem == nullptr)
  {
    return -1;
  }
  QVariant colorIndex = colorItem->data(qMRMLItemDelegate::ColorEntryRole);
  if (!colorIndex.isValid())
  {
    return -1;
  }
  return colorIndex.toInt();
}

//------------------------------------------------------------------------------
QStandardItem* qMRMLColorModel::itemFromColor(int color, int column) const
{
  if (color == -1)
  {
    return nullptr;
  }
  QModelIndexList indexes = this->match(this->index(0, 0), qMRMLItemDelegate::ColorEntryRole, color, 1, Qt::MatchExactly | Qt::MatchRecursive);
  while (indexes.size())
  {
    if (indexes[0].column() == column)
    {
      return this->itemFromIndex(indexes[0]);
    }
    indexes = this->match(indexes[0], qMRMLItemDelegate::ColorEntryRole, color, 1, Qt::MatchExactly | Qt::MatchRecursive);
  }
  return nullptr;
}

//------------------------------------------------------------------------------
QModelIndexList qMRMLColorModel::indexes(int color) const
{
  return this->match(this->index(0, 0), qMRMLItemDelegate::ColorEntryRole, color, -1, Qt::MatchExactly | Qt::MatchRecursive);
}

//------------------------------------------------------------------------------
QColor qMRMLColorModel::qcolorFromColor(int entry) const
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
QString qMRMLColorModel::nameFromColor(int entry) const
{
  Q_D(const qMRMLColorModel);
  if (d->MRMLColorNode == nullptr || entry < 0)
  {
    return QString();
  }
  return QString(d->MRMLColorNode->GetColorName(entry));
}

//------------------------------------------------------------------------------
int qMRMLColorModel::colorFromName(const QString& name) const
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

  this->setRowCount(d->MRMLColorNode->GetNumberOfColors() + (this->noneEnabled() ? 1 : 0));

  // Note: Adding a signal blocker or temporarily disconnecting signals could improve update performance
  // (similarly to how it is done in qMRMLTableModel::updateModelFromMRML()).
  int startIndex = (this->noneEnabled() ? 1 : 0);
  for (int color = 0; color < d->MRMLColorNode->GetNumberOfColors(); ++color)
  {
    for (int j = 0; j < this->columnCount(); ++j)
    {
      this->updateRowForColor(color);
    }
  }

  d->IsUpdatingWidgetFromMRML = false;
}

//------------------------------------------------------------------------------
void qMRMLColorModel::updateRowForColor(int color)
{
  int startIndex = (this->noneEnabled() ? 1 : 0);
  for (int col = 0; col < this->columnCount(); ++col)
  {
    QStandardItem* colorItem = this->invisibleRootItem()->child(color + startIndex, col);
    if (!colorItem)
    {
      colorItem = new QStandardItem();
      this->invisibleRootItem()->setChild(color + startIndex, col, colorItem);
    }
    this->updateItemFromColor(colorItem, color, col);
  }
}

//------------------------------------------------------------------------------
void qMRMLColorModel::updateItemFromColor(QStandardItem* item, int color, int column)
{
  Q_D(qMRMLColorModel);
  if (color < 0)
  {
    return;
  }
  item->setData(color, qMRMLItemDelegate::ColorEntryRole);

  QString colorName = this->nameFromColor(color);

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
      pixmap = qMRMLUtils::createColorPixmap(qApp->style(), QColor::fromRgbF(rgba[0], rgba[1], rgba[2]));
      item->setData(pixmap, Qt::DecorationRole);
      item->setData(QColor::fromRgbF(rgba[0], rgba[1], rgba[2]), qMRMLItemDelegate::ColorRole);
    }
    else
    {
      item->setData(QVariant(), Qt::DecorationRole);
      item->setData(QColor(), qMRMLItemDelegate::ColorRole);
    }
    item->setData(validColor && column != d->LabelColumn ? pixmap.size() : QVariant(), Qt::SizeHintRole);
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
    item->setData(QString::number(rgba[3], 'f', 2), Qt::DisplayRole);
  }
  if (column == d->TerminologyColumn)
  {
    item->setText(qMRMLColorModel::terminologyTextForColor(d->MRMLColorNode, color));
    item->setData(QVariant::fromValue(reinterpret_cast<long long>(d->MRMLColorNode.GetPointer())), qMRMLItemDelegate::PointerRole);
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
  if (d->IsUpdatingWidgetFromMRML)
  {
    return; // Updating widget from MRML is in progress, do not do the reverse update
  }
  vtkMRMLColorTableNode* colorTableNode = vtkMRMLColorTableNode::SafeDownCast(d->MRMLColorNode);
  if (color < 0 || !colorTableNode)
  {
    return;
  }
  if (item->column() == d->ColorColumn)
  {
    QColor rgba(item->data(qMRMLItemDelegate::ColorRole).value<QColor>());
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
void qMRMLColorModel::onMRMLNodeEvent(vtkObject* vtk_obj, unsigned long event, void* client_data, void* vtkNotUsed(call_data))
{
  vtkMRMLColorNode* colorNode = reinterpret_cast<vtkMRMLColorNode*>(vtk_obj);
  qMRMLColorModel* colorModel = reinterpret_cast<qMRMLColorModel*>(client_data);
  Q_ASSERT(colorNode);
  Q_ASSERT(colorModel);
  switch (event)
  {
    default:
    case vtkCommand::ModifiedEvent: colorModel->onMRMLColorNodeModified(colorNode); break;
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
void qMRMLColorModel::onItemChanged(QStandardItem* item)
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
  QVariant retval = QStandardItemModel::headerData(section, orientation, role);

  if (orientation == Qt::Vertical && //
      role == Qt::DisplayRole)
  {
    // for the vertical header, decrement the row number by one, since the
    // rows start from 1 and the indices start from 0 in the color look up
    // table.
    retval = QVariant(retval.toInt() - 1);
  }

  return retval;
}

//------------------------------------------------------------------------------
QString qMRMLColorModel::terminologyTextForColor(vtkMRMLColorNode* colorNode, int colorIndex, bool simplified /*=false*/)
{
  if (colorNode == nullptr || colorIndex >= colorNode->GetNumberOfColors())
  {
    return QString();
  }

  // Get type (with modifier, if any)
  QString type;
  if (colorNode->GetTerminologyType(colorIndex) != nullptr //
      && colorNode->GetTerminologyType(colorIndex)->GetCodeMeaning() != nullptr)
  {
    type = colorNode->GetTerminologyType(colorIndex)->GetCodeMeaning();
    QString typeModifier;
    if (colorNode->GetTerminologyTypeModifier(colorIndex) != nullptr //
        && colorNode->GetTerminologyTypeModifier(colorIndex)->GetCodeMeaning() != nullptr)
    {
      typeModifier = colorNode->GetTerminologyTypeModifier(colorIndex)->GetCodeMeaning();
    }
    if (!typeModifier.isEmpty())
    {
      if (simplified)
      {
        //: For formatting of terminology entry with a modifier in simplified mode. %1 is structure name (e.g., "Kidney"), %2 is modifier (e.g., "Left")
        type = tr("%2 %1").arg(type).arg(typeModifier);
      }
      else
      {
        //: For formatting of terminology entry with a modifier. %1 is structure name (e.g., "Kidney"), %2 is modifier (e.g., "Left")
        type = tr("%1, %2").arg(type).arg(typeModifier);
      }
    }
  }

  // Get region (if any; with modifier, if any)
  QString region;
  if (colorNode->GetTerminologyRegion(colorIndex) != nullptr //
      && colorNode->GetTerminologyRegion(colorIndex)->GetCodeMeaning() != nullptr)
  {
    region = colorNode->GetTerminologyRegion(colorIndex)->GetCodeMeaning();
    QString regionModifier;
    if (colorNode->GetTerminologyRegionModifier(colorIndex) != nullptr //
        && colorNode->GetTerminologyRegionModifier(colorIndex)->GetCodeMeaning() != nullptr)
    {
      regionModifier = colorNode->GetTerminologyRegionModifier(colorIndex)->GetCodeMeaning();
    }
    if (!regionModifier.isEmpty())
    {
      if (simplified)
      {
        //: For formatting of terminology entry name in simplified mode. %1 is region name (e.g., "Kidney"), %2 is region modifier (e.g., "Left")
        region = tr("%2 %1").arg(region).arg(regionModifier);
      }
      else
      {
        //: For formatting of terminology entry name. %1 is region name (e.g., "Kidney"), %2 is region modifier (e.g., "Left")
        region = tr("%1, %2").arg(region).arg(regionModifier);
      }
    }
  }

  QString typeInRegion;
  if (!type.isEmpty())
  {
    if (!region.isEmpty())
    {
      //: For formatting of terminology entry name. %1 is type name (e.g., "Mass"), %2 is region name (e.g., "Liver").
      typeInRegion = tr("%1 in %2").arg(type).arg(region);
    }
    else
    {
      typeInRegion = type;
    }
  }
  else
  {
    if (!region.isEmpty())
    {
      //: For formatting of terminology entry name. %1 is region name (e.g., "Liver").
      typeInRegion = tr("Unknown in %1").arg(type).arg(region);
    }
  }

  QString typeInRegionWithCategory;
  QString category;
  // Skip category in simplified mode
  if (!simplified)
  {
    if (colorNode->GetTerminologyCategory(colorIndex) != nullptr //
        && colorNode->GetTerminologyCategory(colorIndex)->GetCodeMeaning() != nullptr)
    {
      category = colorNode->GetTerminologyCategory(colorIndex)->GetCodeMeaning();
    }
  }
  if (!category.isEmpty())
  {
    if (!typeInRegion.isEmpty())
    {
      //: For formatting of terminology entry name. %1 is category name (e.g., "Morphologically Altered Structure"), %2 is the type in region ("Mass in Liver")
      typeInRegionWithCategory = tr("%1: %2").arg(category).arg(typeInRegion);
    }
    else
    {
      typeInRegionWithCategory = category;
    }
  }
  else
  {
    typeInRegionWithCategory = typeInRegion;
  }

  return typeInRegionWithCategory;
}
