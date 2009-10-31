
#include "qCTKAddRemoveComboBox.h"

#include "ui_qCTKAddRemoveComboBox.h" 

#include <QDebug>

//-----------------------------------------------------------------------------
struct qCTKAddRemoveComboBox::qInternal : public Ui::qCTKAddRemoveComboBox
{
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
qCTKAddRemoveComboBox::qCTKAddRemoveComboBox(QWidget* parent) : Superclass(parent)
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
qCTKAddRemoveComboBox::~qCTKAddRemoveComboBox()
{
  delete this->Internal; 
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setDescription(const QString& text)
{
  this->Internal->DescriptionLabel->setVisible(!text.isEmpty()); 
  this->Internal->DescriptionLabel->setText(text); 
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setEmptyText(const QString& text)
{
  this->Internal->EmptyText = text; 
  if (this->Internal->Empty)
    {
    this->updateSelectedItemName(this->Internal->EmptyText);
    }
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setEditButtonEnabled(bool enable)
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
void qCTKAddRemoveComboBox::setAddButtonEnabled(bool enable)
{
  this->Internal->AddPushButton->setEnabled(enable); 
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onEditPushButtonPressed()
{
  emit this->itemEditRequested(this->Internal->ComboBox->currentText());
  emit this->itemEditRequested(
    this->Internal->ComboBox->itemData(this->Internal->ComboBox->currentIndex()));
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onRemovePushButtonPressed()
{
  emit this->removePushButtonPressed(this->Internal->ComboBox->currentText());
  emit this->removePushButtonPressed(
    this->Internal->ComboBox->itemData(this->Internal->ComboBox->currentIndex()));
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::addItemNoNotify(const QString & text, const QVariant & userData)
{
  this->addItem(text, userData, false);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::addItem(const QString & text, const QVariant & userData, bool notify)
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
void qCTKAddRemoveComboBox::removeItemNoNotify(const QString & text)
{
  this->removeItem(text, false); 
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::removeItem(const QString & text, bool notify)
{
  if (this->Internal->ComboBox->findText(text)  >= 0)
    {
    this->removeItem(this->Internal->ComboBox->findText(text), notify);
    }
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::removeItemNoNotify(const QVariant & data)
{
  this->removeItem(data, false); 
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::removeItem(const QVariant & data, bool notify)
{
  this->removeItem(this->Internal->ComboBox->findData(data), notify);
}

// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::count()const
{
  return (this->Internal->Empty ? 0 : this->Internal->ComboBox->count()); 
}

// --------------------------------------------------------------------------
QString qCTKAddRemoveComboBox::selectedItemName()const
{
  return this->Internal->ComboBox->currentText(); 
}

// --------------------------------------------------------------------------
QVariant qCTKAddRemoveComboBox::selectedItemData()const
{
  return this->Internal->ComboBox->itemData(this->Internal->ComboBox->currentIndex());
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::removeSelectedItem()
{
  int index = this->Internal->ComboBox->currentIndex(); 
    
  this->removeItem(index, true);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::updateSelectedItemName(const QString& newItemName)
{
  this->Internal->ComboBox->setItemText(this->Internal->ComboBox->currentIndex(), newItemName);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::updateItemName(const QVariant& itemData, const QString& newItemName)
{
  this->Internal->ComboBox->setItemText(this->Internal->ComboBox->findData(itemData), newItemName);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::updateItemName(const QString& itemName, const QString& newItemName)
{
  this->Internal->ComboBox->setItemText(this->Internal->ComboBox->findText(itemName), newItemName);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::removeItem(int index, bool notify)
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
