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

// MRMLLogic includes
#include <vtkMRMLColorLogic.h>

// MRML includes
#include <vtkMRMLColorTableNode.h>

// VTK includes

//------------------------------------------------------------------------------
qMRMLColorModelPrivate::qMRMLColorModelPrivate(qMRMLColorModel& object)
  : q_ptr(&object)
{
  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
  this->NoneEnabled = false;
  this->LabelInColor = false;
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
  q->setColumnCount(3);
  if (this->LabelInColor)
    {
    q->setHorizontalHeaderLabels(QStringList() << "Color" << "Label" << "Opacity");
    }
  else
    {
    q->setHorizontalHeaderLabels(QStringList() << "" << "Label" << "Opacity");
    }
  QObject::connect(q, SIGNAL(itemChanged(QStandardItem*)),
                   q, SLOT(onItemChanged(QStandardItem*)),
                   Qt::UniqueConnection);
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
void qMRMLColorModel::setMRMLColorLogic(vtkMRMLColorLogic* colorLogic)
{
  Q_D(qMRMLColorModel);
  d->ColorLogic = colorLogic;
}

//------------------------------------------------------------------------------
vtkMRMLColorLogic* qMRMLColorModel::mrmlColorLogic()const
{
  Q_D(const qMRMLColorModel);
  return d->ColorLogic.GetPointer();
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
void qMRMLColorModel::setLabelInColorColumn(bool enable)
{
  Q_D(qMRMLColorModel);
  if (d->LabelInColor == enable)
    {
    return;
    }
  d->LabelInColor = enable;
  this->updateNode();
}

//------------------------------------------------------------------------------
bool qMRMLColorModel::isLabelInColorColumn()const
{
  Q_D(const qMRMLColorModel);
  return d->LabelInColor;
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
    return 0;
    }
  QModelIndexList indexes = this->match(QModelIndex(), qMRMLColorModel::ColorEntryRole,
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
  return 0;
}

//------------------------------------------------------------------------------
QModelIndexList qMRMLColorModel::indexes(int color)const
{
  return this->match(QModelIndex(), qMRMLColorModel::ColorEntryRole, color, -1,
                     Qt::MatchExactly | Qt::MatchRecursive);
}

//------------------------------------------------------------------------------
QColor qMRMLColorModel::qcolorFromColor(int entry)const
{
  Q_D(const qMRMLColorModel);
  if (d->MRMLColorNode == 0 || entry < 0)
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
  if (d->MRMLColorNode == 0 || entry < 0)
    {
    return QString();
    }
  return QString(d->MRMLColorNode->GetColorName(entry));
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

//------------------------------------------------------------------------------
void qMRMLColorModel::updateItemFromColor(QStandardItem* item, int color, int column)
{
  Q_D(qMRMLColorModel);
  if (color < 0)
    {
    return;
    }
  item->setData(color, qMRMLColorModel::ColorEntryRole);
  double rgba[4] = {0.,0.,0.,1.};
  bool validColor = d->MRMLColorNode->GetColor(color, rgba);
  QString colorName = d->MRMLColorNode->GetNamesInitialised() ?
    d->MRMLColorNode->GetColorName(color) : "";
  switch (column)
    {
    case qMRMLColorModel::ColorColumn:
    default:
      {
      QPixmap pixmap;
      if (validColor)
        {
        // It works to set just a QColor but if the model gets into a QComboBox,
        // the currently selected item doesn't get a decoration.
        // TODO: Cache the pixmap as it is expensive to compute and it is done
        // for ALL the colors of the node anytime a color is changed.
        //item->setData(QColor::fromRgbF(rgba[0], rgba[1], rgba[2]), Qt::DecorationRole);
        pixmap = qMRMLUtils::createColorPixmap(
          qApp->style(), QColor::fromRgbF(rgba[0], rgba[1], rgba[2]));
        item->setData(pixmap, Qt::DecorationRole);
        //item->setData(QColor::fromRgbF(rgba[0], rgba[1], rgba[2]), Qt::DecorationRole);
        item->setData(QColor::fromRgbF(rgba[0], rgba[1], rgba[2]), qMRMLColorModel::ColorRole);
        //item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        }
      else
        {
        item->setData(QVariant(), Qt::DecorationRole);
        item->setData(QColor(), qMRMLColorModel::ColorRole);
        }
      if (d->LabelInColor)
        {
        item->setText(colorName);
        item->setData(QVariant(),Qt::SizeHintRole);
        //item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        }
      else
        {
        item->setText(QString());
        item->setData((validColor ? pixmap.size() : QVariant()),Qt::SizeHintRole);
        //item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        }
      item->setToolTip(colorName);
      break;
      }
    case qMRMLColorModel::LabelColumn:
      item->setText(colorName);
      // check for terminology
      if (this->mrmlColorLogic() &&
          this->mrmlColorLogic()->TerminologyExists(d->MRMLColorNode->GetName()))
        {
        const char *lutName = d->MRMLColorNode->GetName();
        std::string category = this->mrmlColorLogic()->GetSegmentedPropertyCategoryCodeMeaning(color, lutName);
        std::string type = this->mrmlColorLogic()->GetSegmentedPropertyTypeCodeMeaning(color, lutName);
        std::string typeMod = this->mrmlColorLogic()->GetSegmentedPropertyTypeModifierCodeMeaning(color, lutName);
        std::string region = this->mrmlColorLogic()->GetAnatomicRegionCodeMeaning(color, lutName);
        std::string regionMod = this->mrmlColorLogic()->GetAnatomicRegionModifierCodeMeaning(color, lutName);
        QString terminology = QString("Terminology:");
        // only show the not empty terminology terms
        if (!category.empty())
          {
          terminology = terminology + QString("\nSegmentedPropertyCategory: %1").arg(category.c_str());
          }
        if (!type.empty())
          {
          terminology = terminology + QString("\nSegmentedPropertyType: %1").arg(type.c_str());
          }
        if (!typeMod.empty())
          {
          terminology = terminology + QString("\nSegmentedPropertyTypeModifier: %1").arg(typeMod.c_str());
          }
        if (!region.empty())
          {
          terminology = terminology + QString("\nAnatomicRegion: %1").arg(region.c_str());
          }
        if (!regionMod.empty())
          {
          terminology = terminology + QString("\nAnatomicRegionModifier: %1").arg(regionMod.c_str());
          }
        // check if no terminology was found and use an empty tool tip
        if (!terminology.compare("Terminology:"))
          {
          terminology = QString("");
          }
        item->setToolTip(terminology);
        }
      else
        {
        item->setToolTip("");
        }
      break;
    case qMRMLColorModel::OpacityColumn:
      item->setData(QString::number(rgba[3],'f',2), Qt::DisplayRole);
      break;
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
  switch(item->column())
    {
    case qMRMLColorModel::ColorColumn:
      {
      QColor rgba(item->data(qMRMLColorModel::ColorRole).value<QColor>());
      colorTableNode->SetColor(color, rgba.redF(), rgba.greenF(), rgba.blueF(), rgba.alphaF());
      break;
      }
    case qMRMLColorModel::LabelColumn:
      colorTableNode->SetColorName(color, item->text().toLatin1());
      break;
    case qMRMLColorModel::OpacityColumn:
      colorTableNode->SetOpacity(color, item->data(Qt::DisplayRole).toDouble());
      break;
    default:
      break;
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
