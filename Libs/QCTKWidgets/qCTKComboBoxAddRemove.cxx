
#include "qCTKComboBoxAddRemove.h"

#include "ui_qCTKComboBoxAddRemove.h" 

#include <QDebug>

//-----------------------------------------------------------------------------
class qCTKComboBoxAddRemove::qInternal : public Ui::qCTKComboBoxAddRemove
{
public:
  qInternal()
    {
    this->Empty = true; 
    this->EmptyText = "None";
    this->EditButtonEnabled = false; 
    }
  bool    Empty; 
  QString EmptyText;
  bool    EditButtonEnabled; 
};

// --------------------------------------------------------------------------
qCTKComboBoxAddRemove::qCTKComboBoxAddRemove(QWidget* parent) : Superclass(parent)
{
  this->Internal = new qInternal; 
  this->Internal->setupUi(this);
  
  // connect 
  this->connect(this->Internal->ComboBox, SIGNAL(activated(const QString &)), SIGNAL(itemSelected(const QString &)));
  this->connect(this->Internal->AddPushButton, SIGNAL(pressed()), SIGNAL(addPushButtonPressed()));
  this->connect(this->Internal->RemovePushButton, SIGNAL(pressed()), SLOT(onRemovePushButtonPressed())); 
  //this->connect(this->Internal->RemovePushButton, SIGNAL(pressed()), SLOT(removeSelectedItem())); 
  
  this->setEditButtonEnabled(true);
  
  // By default, description label is not visible
  this->Internal->DescriptionLabel->setVisible(false); 
  
  // Add default 'empty item'
  this->Internal->ComboBox->addItem(this->Internal->EmptyText);
  
  // Disable combo box
  this->Internal->ComboBox->setEnabled(false); 
}

// --------------------------------------------------------------------------
qCTKComboBoxAddRemove::~qCTKComboBoxAddRemove()
{
  delete this->Internal; 
}

// --------------------------------------------------------------------------
void qCTKComboBoxAddRemove::setDescription(const QString& text)
{
  this->Internal->DescriptionLabel->setVisible(!text.isEmpty()); 
  this->Internal->DescriptionLabel->setText(text); 
}

// --------------------------------------------------------------------------
void qCTKComboBoxAddRemove::setEmptyText(const QString& text)
{
  this->Internal->EmptyText = text; 
  if (this->Internal->Empty)
    {
    this->updateSelectedItemName(this->Internal->EmptyText);
    }
}

// --------------------------------------------------------------------------
void qCTKComboBoxAddRemove::setEditButtonEnabled(bool enable)
{
  if (this->Internal->EditButtonEnabled == enable)
    {
    return; 
    }
    
  if (enable)
    {
    // connect edit button
    this->connect(this->Internal->EditPushButton, SIGNAL(pressed()), 
      SLOT(onEditPushButtonPressed())); 
    
    // enable/disable edit button according to widget state
    this->Internal->EditPushButton->setEnabled(!this->Internal->Empty); 
    }
  else
    {
    // disconnect edit button
    QObject::disconnect(this->Internal->EditPushButton, SIGNAL(pressed()), 
      this, SLOT(onEditPushButtonPressed()));
    }
  this->Internal->EditPushButton->setVisible(enable); 
  this->Internal->EditButtonEnabled = enable; 
}

// --------------------------------------------------------------------------
void qCTKComboBoxAddRemove::onEditPushButtonPressed()
{
  emit this->itemEditRequested(this->Internal->ComboBox->currentText());
  emit this->itemEditRequested(
    this->Internal->ComboBox->itemData(this->Internal->ComboBox->currentIndex()));
}

void qCTKComboBoxAddRemove::onRemovePushButtonPressed()
{
  emit this->removePushButtonPressed(this->Internal->ComboBox->currentText());
  emit this->removePushButtonPressed(
    this->Internal->ComboBox->itemData(this->Internal->ComboBox->currentIndex()));
}

// --------------------------------------------------------------------------
void qCTKComboBoxAddRemove::addItemNoNotify(const QString & text, const QVariant & userData)
{
  this->addItem(text, userData, false);
}

// --------------------------------------------------------------------------
void qCTKComboBoxAddRemove::addItem(const QString & text, const QVariant & userData, bool notify)
{
  if (this->Internal->Empty)
    {
    // Remove empty item
    this->Internal->ComboBox->removeItem(0); 
    this->Internal->Empty = false;
    
    // Enable combobox
    this->Internal->ComboBox->setEnabled(true); 
    
    // Enable remove/edit button
    this->Internal->RemovePushButton->setEnabled(true);
    this->Internal->EditPushButton->setEnabled(true);
    }
    
  this->Internal->ComboBox->insertItem(0, text, userData);
   
  // Set selected item to be the added one
  this->Internal->ComboBox->setCurrentIndex(0);
  
  if (notify)
    {
    emit this->itemAdded(this->Internal->ComboBox->currentText());
    emit this->itemAdded(
      this->Internal->ComboBox->itemData(this->Internal->ComboBox->currentIndex()));
    }
}

// --------------------------------------------------------------------------
void qCTKComboBoxAddRemove::removeItemNoNotify(const QString & text)
{
  this->removeItem(text, false); 
}

// --------------------------------------------------------------------------
void qCTKComboBoxAddRemove::removeItem(const QString & text, bool notify)
{
  if (this->Internal->ComboBox->findText(text)  >= 0)
    {
    this->removeItem(this->Internal->ComboBox->findText(text), notify);
    }
}

// --------------------------------------------------------------------------
void qCTKComboBoxAddRemove::removeItemNoNotify(const QVariant & data)
{
  this->removeItem(data, false); 
}

// --------------------------------------------------------------------------
void qCTKComboBoxAddRemove::removeItem(const QVariant & data, bool notify)
{
  this->removeItem(this->Internal->ComboBox->findData(data), notify);
}

// --------------------------------------------------------------------------
int qCTKComboBoxAddRemove::count()
{
  return (this->Internal->Empty ? 0 : this->Internal->ComboBox->count()); 
}

// --------------------------------------------------------------------------
QString qCTKComboBoxAddRemove::getSelectedItemName()
{
  return this->Internal->ComboBox->currentText(); 
}

// --------------------------------------------------------------------------
QVariant qCTKComboBoxAddRemove::getSelectedItemData()
{
  return this->Internal->ComboBox->itemData(this->Internal->ComboBox->currentIndex());
}

// --------------------------------------------------------------------------
void qCTKComboBoxAddRemove::removeSelectedItem()
{
  int index = this->Internal->ComboBox->currentIndex(); 
    
  this->removeItem(index, true);
}

// --------------------------------------------------------------------------
void qCTKComboBoxAddRemove::updateSelectedItemName(const QString& newItemName)
{
  this->Internal->ComboBox->setItemText(this->Internal->ComboBox->currentIndex(), newItemName);
}

// --------------------------------------------------------------------------
void qCTKComboBoxAddRemove::updateItemName(const QVariant& itemData, const QString& newItemName)
{
  this->Internal->ComboBox->setItemText(this->Internal->ComboBox->findData(itemData), newItemName);
}

// --------------------------------------------------------------------------
void qCTKComboBoxAddRemove::updateItemName(const QString& itemName, const QString& newItemName)
{
  this->Internal->ComboBox->setItemText(this->Internal->ComboBox->findText(itemName), newItemName);
}

// --------------------------------------------------------------------------
void qCTKComboBoxAddRemove::removeItem(int index, bool notify)
{
//  Q_ASSERT(index >= 0);
    // asserts cause Slicer to crash, just return
  if (index  < 0)
    {
    return;
    }
  if (this->Internal->Empty)
    {
    return; 
    }
  
  // Save the property of the item to be removed
  QString text = this->Internal->ComboBox->itemText(index); 
  QVariant data = this->Internal->ComboBox->itemData(index); 
  
  
  this->Internal->ComboBox->removeItem(index); 
  
  bool isLastItem = (this->Internal->ComboBox->count() == 0); 
  if (isLastItem)
    {
    // Disable remove/edit button
    this->Internal->RemovePushButton->setDisabled(true);
    this->Internal->EditPushButton->setDisabled(true);
    
    // Set widget to empty state
    this->Internal->Empty = true; 
    this->Internal->ComboBox->addItem(this->Internal->EmptyText);
    
    // Disable combobox
    this->Internal->ComboBox->setEnabled(false); 
    
    if (notify)
      {
      emit this->lastItemRemoved(); 
      }
  }
  
  if (notify)
    {
    emit this->itemRemoved(text, isLastItem);
    emit this->itemRemoved(data, isLastItem); 
    }
}
