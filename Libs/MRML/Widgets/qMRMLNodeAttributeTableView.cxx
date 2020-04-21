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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Qt includes
#include <QStringList>
#include <QItemSelectionModel>

// qMRML includes
#include "qMRMLNodeAttributeTableView.h"
#include "ui_qMRMLNodeAttributeTableView.h"

// MRML includes
#include <vtkMRMLNode.h>

// STL includes
#include <set>

// --------------------------------------------------------------------------
class qMRMLNodeAttributeTableViewPrivate: public Ui_qMRMLNodeAttributeTableView
{
  Q_DECLARE_PUBLIC(qMRMLNodeAttributeTableView);
protected:
  qMRMLNodeAttributeTableView* const q_ptr;
public:
  qMRMLNodeAttributeTableViewPrivate(qMRMLNodeAttributeTableView& object);
  void init();

  /// Sets table message and takes care of the visibility of the label
  void setMessage(const QString& message)
  {
    this->AttributeTableMessageLabel->setVisible(!message.isEmpty());
    this->AttributeTableMessageLabel->setText(message);
  };

public:
  /// MRML node to inspect
  vtkMRMLNode* InspectedNode;
};

// --------------------------------------------------------------------------
qMRMLNodeAttributeTableViewPrivate::qMRMLNodeAttributeTableViewPrivate(qMRMLNodeAttributeTableView& object)
  : q_ptr(&object)
{
  this->InspectedNode = nullptr;
}

// --------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewPrivate::init()
{
  Q_Q(qMRMLNodeAttributeTableView);
  this->setupUi(q);

  QObject::connect(this->NodeAttributesTable, SIGNAL(itemChanged(QTableWidgetItem*)),
          q, SLOT(onAttributeChanged(QTableWidgetItem*)));

  this->setMessage(QString());
}

// --------------------------------------------------------------------------
// qMRMLNodeAttributeTableView methods

// --------------------------------------------------------------------------
qMRMLNodeAttributeTableView::qMRMLNodeAttributeTableView(QWidget* _parent)
  : QWidget(_parent)
  , d_ptr(new qMRMLNodeAttributeTableViewPrivate(*this))
{
  Q_D(qMRMLNodeAttributeTableView);
  d->init();
  this->populateAttributeTable();
}

// --------------------------------------------------------------------------
qMRMLNodeAttributeTableView::~qMRMLNodeAttributeTableView() = default;

//-----------------------------------------------------------------------------
void qMRMLNodeAttributeTableView::setInspectedNode(vtkMRMLNode* node)
{
  Q_D(qMRMLNodeAttributeTableView);

  qvtkReconnect( d->InspectedNode, node, vtkCommand::ModifiedEvent,
                this, SLOT( populateAttributeTable() ) );

  d->InspectedNode = node;
  this->populateAttributeTable();
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeAttributeTableView::inspectedNode()
{
  Q_D(qMRMLNodeAttributeTableView);

  return d->InspectedNode;
}

//-----------------------------------------------------------------------------
void qMRMLNodeAttributeTableView::populateAttributeTable()
{
  Q_D(qMRMLNodeAttributeTableView);

  d->setMessage(QString());

  // Block signals so that onAttributeChanged function is not called when populating
  bool wasBlocked = d->NodeAttributesTable->blockSignals(true);

  d->NodeAttributesTable->clearContents();

  QStringList headerLabels;
  headerLabels << "Attribute name" << "Attribute value";
  d->NodeAttributesTable->setHorizontalHeaderLabels(headerLabels);
  d->NodeAttributesTable->setColumnWidth(0, d->NodeAttributesTable->width() / 2 - 10);
  d->NodeAttributesTable->setColumnWidth(1, d->NodeAttributesTable->width() / 2 - 10);

  if (!d->InspectedNode)
    {
    d->setMessage(tr("No node is selected"));
    d->NodeAttributesTable->setRowCount(0);
    d->NodeAttributesTable->blockSignals(wasBlocked);
    return;
    }

  std::vector< std::string > attributeNames = d->InspectedNode->GetAttributeNames();
  if (attributeNames.size() == 0)
    {
    d->setMessage(tr("Selected node has no attributes"));
    d->NodeAttributesTable->setRowCount(0);
    }
  else
    {
    d->NodeAttributesTable->setRowCount(attributeNames.size());

    int row = 0;
    for (std::vector< std::string >::iterator iter = attributeNames.begin();
        iter != attributeNames.end(); ++iter, ++row)
      {
      // attribute name
      QString attributeName = QString(iter->c_str());
      QTableWidgetItem* attributeNameItem = new QTableWidgetItem(attributeName);
      // save the attribute name as user data so that if the item is renamed
      // we know which attribute name to change
      attributeNameItem->setData(Qt::UserRole, attributeName);
      d->NodeAttributesTable->setItem( row, 0, attributeNameItem);

      // attribute value
      d->NodeAttributesTable->setItem( row, 1, new QTableWidgetItem(
        QString(d->InspectedNode->GetAttribute(iter->c_str())) ) );
      }
    }

  // Unblock signals
  d->NodeAttributesTable->blockSignals(wasBlocked);
}

//-----------------------------------------------------------------------------
void qMRMLNodeAttributeTableView::onAttributeChanged(QTableWidgetItem* changedItem)
{
  Q_D(qMRMLNodeAttributeTableView);

  d->setMessage(QString());

  if (!changedItem || !d->InspectedNode)
    {
    return;
    }

  if (changedItem->column() == 0)
    {
    // Attribute name has been changed
    QTableWidgetItem* valueItem = d->NodeAttributesTable->item( changedItem->row(), 1 );
    QString valueText;
    if (valueItem)
      {
      valueText = valueItem->text();
      }

    QString attributeNameBeforeEditing = changedItem->data(Qt::UserRole).toString();
    if (d->InspectedNode->GetAttribute(changedItem->text().toUtf8().constData()))
      {
      // Don't set if there is another attribute with the same name (would overwrite it),
      // revert to the original value.
      d->setMessage(tr("There is already an attribute with the same name"));
      bool wasBlocked = d->NodeAttributesTable->blockSignals(true);
      changedItem->setText(attributeNameBeforeEditing);
      d->NodeAttributesTable->blockSignals(wasBlocked);
      }
    else
      {
      int wasModifying = d->InspectedNode->StartModify();

      d->InspectedNode->RemoveAttribute(attributeNameBeforeEditing.toUtf8().constData());
      d->InspectedNode->SetAttribute(
        changedItem->text().toUtf8().constData(), valueText.toUtf8().constData());

      // Save the new attribute name
      bool wasBlocked = d->NodeAttributesTable->blockSignals(true);
      changedItem->setData(Qt::UserRole, changedItem->text());
      d->NodeAttributesTable->blockSignals(wasBlocked);

      d->InspectedNode->EndModify(wasModifying);
      }
    }
  else if (changedItem->column() == 1)
    {
    // Attribute value has been changed
    QTableWidgetItem* nameItem = d->NodeAttributesTable->item( changedItem->row(), 0 );
    QString nameText;
    if (nameItem)
      {
      nameText = nameItem->text();
      }
    d->InspectedNode->SetAttribute( nameText.toUtf8().constData(), changedItem->text().toUtf8().constData() );
    }
}

//-----------------------------------------------------------------------------
QString qMRMLNodeAttributeTableView::generateNewAttributeName() const
{
  Q_D(const qMRMLNodeAttributeTableView);

  QString newAttributeNameBase("NewAttributeName");
  QString newAttributeName(newAttributeNameBase);
  int i=0;
  while (d->InspectedNode->GetAttribute(newAttributeName.toUtf8().constData()))
    {
    newAttributeName = QString("%1%2").arg(newAttributeNameBase).arg(++i);
    }
  return newAttributeName;
}

//-----------------------------------------------------------------------------
void qMRMLNodeAttributeTableView::addAttribute()
{
  Q_D(qMRMLNodeAttributeTableView);

  if (!d->InspectedNode)
    {
    d->setMessage(tr("No node is selected"));
    return;
    }

  d->setMessage(QString());

  bool wasBlocked = d->NodeAttributesTable->blockSignals(true);
  int rowCountBefore = d->NodeAttributesTable->rowCount();
  d->NodeAttributesTable->insertRow( rowCountBefore );
  QString newAttributeName = this->generateNewAttributeName();
  QString newAttributeValue;
  QTableWidgetItem* attributeNameItem = new QTableWidgetItem(newAttributeName);
  // save the attribute name as user data so that if the item is renamed
  // we know which attribute name to change
  attributeNameItem->setData(Qt::UserRole, newAttributeName);
  d->NodeAttributesTable->setItem( rowCountBefore, 0, attributeNameItem);
  d->NodeAttributesTable->setItem( rowCountBefore, 1, new QTableWidgetItem(newAttributeValue) );
  d->NodeAttributesTable->blockSignals(wasBlocked);

  d->InspectedNode->SetAttribute(newAttributeName.toUtf8().constData(), newAttributeValue.toUtf8().constData());
}

//-----------------------------------------------------------------------------
void qMRMLNodeAttributeTableView::removeSelectedAttributes()
{
  Q_D(qMRMLNodeAttributeTableView);

  if (!d->InspectedNode)
    {
    d->setMessage(tr("No node is selected"));
    return;
    }

  d->setMessage(QString());

  // Extract selected row indices out of the selected table widget items list
  // (there may be more items selected in a row)
  QList<QTableWidgetItem*> selectedItems = d->NodeAttributesTable->selectedItems();
  QSet<int> affectedRowNumbers;
  foreach (QTableWidgetItem* item, selectedItems)
    {
    affectedRowNumbers.insert(item->row());
    }

  int wasModifying = d->InspectedNode->StartModify();

  for (QSet<int>::iterator it = affectedRowNumbers.begin(); it != affectedRowNumbers.end(); ++it)
    {
    QString attributeNameToDelete( d->NodeAttributesTable->item((*it), 0)->text() );
    d->InspectedNode->RemoveAttribute( attributeNameToDelete.toUtf8().constData() );
    }

  d->InspectedNode->Modified();
  d->InspectedNode->EndModify(wasModifying);
}

//-----------------------------------------------------------------------------
int qMRMLNodeAttributeTableView::attributeCount() const
{
  Q_D(const qMRMLNodeAttributeTableView);

  return d->NodeAttributesTable->rowCount();
}

//-----------------------------------------------------------------------------
QStringList qMRMLNodeAttributeTableView::attributes() const
{
  Q_D(const qMRMLNodeAttributeTableView);

  QStringList attributeList;
  for (int i=0; i<d->NodeAttributesTable->rowCount(); ++i)
    {
    attributeList << d->NodeAttributesTable->item(i,0)->text();
    }

  return attributeList;
}

//-----------------------------------------------------------------------------
QTableWidgetItem* qMRMLNodeAttributeTableView::findAttributeNameItem(const QString& attributeName) const
{
  Q_D(const qMRMLNodeAttributeTableView);

  QTableWidgetItem* item = nullptr;
  int numberOfAttributesFound = 0;
  QList<QTableWidgetItem*> itemList = d->NodeAttributesTable->findItems(attributeName, Qt::MatchFixedString);
  foreach(QTableWidgetItem* currentItem, itemList)
    {
    // Check if found item is in the name column (there may be values containing the same text)
    if (currentItem != nullptr && currentItem->column() == 0)
      {
        numberOfAttributesFound++;
        item = currentItem;
      }
    }

  return (numberOfAttributesFound == 1) ? item : nullptr;
}

//-----------------------------------------------------------------------------
QString qMRMLNodeAttributeTableView::attributeValue(const QString& attributeName) const
{
  Q_D(const qMRMLNodeAttributeTableView);

  QTableWidgetItem* item = this->findAttributeNameItem(attributeName);
  return item ? d->NodeAttributesTable->item(item->row(), item->column()+1)->text() : QString();
}

//-----------------------------------------------------------------------------
void qMRMLNodeAttributeTableView::setAttribute(const QString& attributeName, const QString& attributeValue)
{
  if (attributeName.isNull())
    {
    return;
    }

  Q_D(qMRMLNodeAttributeTableView);

  QTableWidgetItem* nameItem = this->findAttributeNameItem(attributeName);
  if (!nameItem)
    {
    if (attributeName.isEmpty() || attributeValue.isNull())
      {
      return;
      }
    int rowCountBefore = d->NodeAttributesTable->rowCount();
    d->NodeAttributesTable->insertRow( rowCountBefore );
    d->NodeAttributesTable->setItem( rowCountBefore, 0, new QTableWidgetItem(attributeName) );
    d->NodeAttributesTable->setItem( rowCountBefore, 1, new QTableWidgetItem(attributeValue) );
    return;
    }
  else if (attributeValue.isNull())
    {
    d->InspectedNode->RemoveAttribute(attributeName.toUtf8().constData());
    return;
    }

  QTableWidgetItem* valueItem = d->NodeAttributesTable->item(nameItem->row(), nameItem->column()+1);

  if (valueItem)
    {
    valueItem->setText(attributeValue);
    }
}

//-----------------------------------------------------------------------------
void qMRMLNodeAttributeTableView::renameAttribute(const QString& oldName, const QString& newName)
{
  QTableWidgetItem* nameItem = this->findAttributeNameItem(oldName);
  if (nameItem)
    {
    nameItem->setText(newName);
    }
}

//-----------------------------------------------------------------------------
void qMRMLNodeAttributeTableView::selectItemRange(const int topRow, const int leftColumn, const int bottomRow, const int rightColumn)
{
  Q_D(qMRMLNodeAttributeTableView);

  QTableWidgetSelectionRange range(topRow, leftColumn, bottomRow, rightColumn);
  d->NodeAttributesTable->setRangeSelected(range, true);
}

//-----------------------------------------------------------------------------
QItemSelectionModel* qMRMLNodeAttributeTableView::selectionModel()
{
  Q_D(qMRMLNodeAttributeTableView);

  return d->NodeAttributesTable->selectionModel();
}
