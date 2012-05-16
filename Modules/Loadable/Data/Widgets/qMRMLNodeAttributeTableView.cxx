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

// STL incudes
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

  /// Text of the attribute table item that is being edited
  QString SelectedAttributeTableItemText;
};

// --------------------------------------------------------------------------
qMRMLNodeAttributeTableViewPrivate::qMRMLNodeAttributeTableViewPrivate(qMRMLNodeAttributeTableView& object)
  : q_ptr(&object)
{
  this->InspectedNode = 0;
}

// --------------------------------------------------------------------------
void qMRMLNodeAttributeTableViewPrivate::init()
{
  Q_Q(qMRMLNodeAttributeTableView);
  this->setupUi(q);

  QObject::connect(this->NodeAttributesTable, SIGNAL(itemChanged(QTableWidgetItem*)),
          q, SLOT(onAttributeChanged(QTableWidgetItem*)));
  QObject::connect(this->NodeAttributesTable, SIGNAL(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)),
          q, SLOT(storeSelectedAttributeTableItem(QTableWidgetItem*,QTableWidgetItem*)));

  this->setMessage(QString());

  // Set up initial look of node attributes table
  this->NodeAttributesTable->horizontalHeader()->setResizeMode(
    QHeaderView::ResizeToContents);
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
qMRMLNodeAttributeTableView::~qMRMLNodeAttributeTableView()
{
}

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
  d->NodeAttributesTable->blockSignals(true);

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
    d->NodeAttributesTable->blockSignals(false);
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
      d->NodeAttributesTable->setItem( row, 0, new QTableWidgetItem(
        QString(iter->c_str())) );

      d->NodeAttributesTable->setItem( row, 1, new QTableWidgetItem(
        QString(d->InspectedNode->GetAttribute(iter->c_str())) ) );
      }
    }

  // Unblock signals
  d->NodeAttributesTable->blockSignals(false);
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
  // If attribute name has been changed
  else if (changedItem->column() == 0)
    {
    QTableWidgetItem* valueItem = d->NodeAttributesTable->item( changedItem->row(), 1 );
    std::string valueText("");
    if (valueItem)
      {
      valueText = valueItem->text().toLatin1();
      }

    // Don't set if there is another attribute with the same name (would overwrite it)
    if ( d->InspectedNode->GetAttribute(changedItem->text().toLatin1()) )
      {
      d->setMessage(tr("There is already an attribute with the same name"));
      d->NodeAttributesTable->blockSignals(true);
      changedItem->setText( d->SelectedAttributeTableItemText.toLatin1() );
      d->NodeAttributesTable->blockSignals(false);
      }
    else
      {
      int wasModifying = d->InspectedNode->StartModify();

      d->InspectedNode->SetAttribute(
        d->SelectedAttributeTableItemText.toLatin1(), 0 );
      d->InspectedNode->SetAttribute(
        changedItem->text().toLatin1(), valueText.c_str() );

      d->InspectedNode->EndModify(wasModifying);
      }
    }
  // If attribute value has been changed
  else if (changedItem->column() == 1)
    {
    QTableWidgetItem* nameItem = d->NodeAttributesTable->item( changedItem->row(), 0 );
    std::string nameText;
    if (nameItem)
      {
      nameText = nameItem->text().toLatin1();
      }
    else
      {
      nameText = "";
      }
    d->InspectedNode->SetAttribute( nameText.c_str(), changedItem->text().toLatin1() );
    }
}

//-----------------------------------------------------------------------------
void qMRMLNodeAttributeTableView::storeSelectedAttributeTableItem(QTableWidgetItem* selectedItem, QTableWidgetItem* previousItem)
{
  Q_D(qMRMLNodeAttributeTableView);

  d->SelectedAttributeTableItemText = selectedItem ? selectedItem->text() : QString();
}

//-----------------------------------------------------------------------------
QString qMRMLNodeAttributeTableView::generateNewAttributeName() const
{
  Q_D(const qMRMLNodeAttributeTableView);

  QString newAttributeNameBase("NewAttributeName");
  QString newAttributeName(newAttributeNameBase);
  int i=0;
  while (d->InspectedNode->GetAttribute(newAttributeName.toLatin1()))
    {
    newAttributeName = QString("%1%2").arg(newAttributeNameBase).arg(++i);
    }
  return newAttributeName;
}

//-----------------------------------------------------------------------------
void qMRMLNodeAttributeTableView::addAttribute()
{
  Q_D(qMRMLNodeAttributeTableView);

  d->setMessage(QString());

  int rowCountBefore = d->NodeAttributesTable->rowCount();
  d->NodeAttributesTable->insertRow( rowCountBefore );
  d->NodeAttributesTable->setItem( rowCountBefore, 0,
    new QTableWidgetItem(this->generateNewAttributeName()) );
  d->NodeAttributesTable->setItem( rowCountBefore, 1, new QTableWidgetItem(QString()) );
}

//-----------------------------------------------------------------------------
void qMRMLNodeAttributeTableView::removeSelectedAttributes()
{
  Q_D(qMRMLNodeAttributeTableView);

  d->setMessage(QString());

  if (!d->InspectedNode)
    {
    return;
    }

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
    d->InspectedNode->RemoveAttribute( attributeNameToDelete.toLatin1() );
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

  QTableWidgetItem* item = NULL;
  int numberOfAttributesFound = 0;
  QList<QTableWidgetItem*> itemList = d->NodeAttributesTable->findItems(attributeName, Qt::MatchFixedString);
  foreach(QTableWidgetItem* currentItem, itemList)
    {
    // Check if found item is in the name column (there may be values containing the same text)
    if (currentItem != NULL && currentItem->column() == 0)
      {
        numberOfAttributesFound++;
        item = currentItem;
      }
    }

  Q_ASSERT(numberOfAttributesFound == 1);
  return item;
}

//-----------------------------------------------------------------------------
QString qMRMLNodeAttributeTableView::attributeValue(const QString& attributeName) const
{
  QTableWidgetItem* item = findAttributeNameItem(attributeName);
  return item ? item->text() : QString();
}

//-----------------------------------------------------------------------------
void qMRMLNodeAttributeTableView::setAttribute(const QString& attributeName, const QString& attributeValue)
{
  Q_D(qMRMLNodeAttributeTableView);

  QTableWidgetItem* nameItem = findAttributeNameItem(attributeName);
  if (!nameItem)
    {
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
  QTableWidgetItem* nameItem = findAttributeNameItem(oldName);
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
