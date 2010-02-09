/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/


#include "qCTKAddRemoveComboBox.h"

#include "ui_qCTKAddRemoveComboBox.h" 

#include <QDebug>

//-----------------------------------------------------------------------------
class qCTKAddRemoveComboBoxPrivate : public qCTKPrivate<qCTKAddRemoveComboBox>,
                                      public Ui_qCTKAddRemoveComboBox
{
public:
  qCTKAddRemoveComboBoxPrivate();

  // Description:
  // Insert 'Add', 'Remove' and 'Edit' item in the combobox
  // Note: Also make sure that no signals are emited while the items are inserted
  // That function doesn't prevent from inserting multiple time the action items
  void insertActionItems();

  // Description:
  // Insert 'None' item
  // Note: Also make sure that no signal is emited while the item is inserted
  // That function doesn't prevent from inserting multiple time the 'None' item
  void insertEmptyItem();

  void connectComboBox(QComboBox* combobox);

  // Empty item
  QString EmptyText;

  // Actions text
  QString AddText;
  QString RemoveText;
  QString EditText;

  // Set to true when inserting either action items or the 'None' item.
  // Will prevent the itemAdded signal from being sent
  bool    AddingEmptyItem; 
  bool    AddingActionItems;

  // Set to true when removing either action items or the 'None' item.
  // Will prevent the itemRemoved signal from being sent
  bool    RemovingEmptyItem;
  bool    RemovingActionItems;

  // Actions state
  bool    AddEnabled;
  bool    RemoveEnabled;
  bool    EditEnabled;

  // If true, it means there is no item beside of the 'None' one
  bool    HasEmptyItem;
  
  // true: PushButtons enabled and visible,
  // false: Actions available in the combobox
  bool    PushButtonEnabled;
};

// --------------------------------------------------------------------------
qCTKAddRemoveComboBox::qCTKAddRemoveComboBox(QWidget* _parent) : Superclass(_parent)
{
  QCTK_INIT_PRIVATE(qCTKAddRemoveComboBox);
  QCTK_D(qCTKAddRemoveComboBox);
  d->setupUi(this);
  
  // connect 
  d->connectComboBox(d->ComboBox);
    
  this->connect(d->AddPushButton, SIGNAL(pressed()), SLOT(onAdd()));
  this->connect(d->RemovePushButton, SIGNAL(pressed()), SLOT(onRemove()));
  this->connect(d->EditPushButton, SIGNAL(pressed()), SLOT(onEdit()));

  // Add default 'empty item'
  d->insertEmptyItem();

  // By default, add the combo box action
  //d->insertActionItems();
}

/// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::printAdditionalInfo()
{
  QCTK_D(qCTKAddRemoveComboBox);
  qDebug() << "qCTKAddRemoveComboBox:" << this << endl
           << " EmptyText:" << d->EmptyText << endl
           << " AddText:" << d->AddText << endl
           << " RemoveText:" << d->RemoveText << endl
           << " EditText:" << d->EditText << endl
           << " AddingEmptyItem:" << d->AddingEmptyItem << endl
           << " RemovingEmptyItem:" << d->RemovingEmptyItem << endl
           << " AddEnabled:" << d->AddEnabled << endl
           << " RemoveEnabled:" << d->RemoveEnabled << endl
           << " EditEnabled:" << d->EditEnabled << endl
           << " HasEmptyItem:" << d->HasEmptyItem << endl
           << " PushButtonEnabled:" << d->PushButtonEnabled;
}

/// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setComboBox(QComboBox* comboBox)
{
  QCTK_D(qCTKAddRemoveComboBox);
  if ((comboBox == d->ComboBox) ||
      comboBox->count())
    {
    return;
    }
  
  QLayoutItem* oldComboBoxItem = this->layout()->takeAt(0);
  QComboBox* oldComboBox = qobject_cast<QComboBox*>(oldComboBoxItem->widget());
  comboBox->setSizePolicy(oldComboBox->sizePolicy());
  comboBox->setEnabled(this->comboBoxEnabled());

  delete oldComboBoxItem;

  dynamic_cast<QBoxLayout*>(this->layout())->insertWidget(0, comboBox);
  d->connectComboBox(comboBox);
  d->ComboBox = comboBox;
  delete oldComboBox;

  // Add default 'empty item'
  d->insertEmptyItem();
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setEmptyText(const QString& text)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    {
    Q_ASSERT(d->ComboBox->count() == 1);
    this->setItemText(0, text);
    }
  d->EmptyText = text;
}

// --------------------------------------------------------------------------
QCTK_GET_CXX(qCTKAddRemoveComboBox, QString, emptyText, EmptyText);

// --------------------------------------------------------------------------
QCTK_SET_CXX(qCTKAddRemoveComboBox, const QString&, setAddText, AddText);
QCTK_GET_CXX(qCTKAddRemoveComboBox, QString, addText, AddText);

// --------------------------------------------------------------------------
QCTK_SET_CXX(qCTKAddRemoveComboBox, const QString&, setRemoveText, RemoveText);
QCTK_GET_CXX(qCTKAddRemoveComboBox, QString, removeText, RemoveText);

// --------------------------------------------------------------------------
QCTK_SET_CXX(qCTKAddRemoveComboBox, const QString&, setEditText, EditText);
QCTK_GET_CXX(qCTKAddRemoveComboBox, QString, editText, EditText);

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onRowsInserted(const QModelIndex & _parent, int start, int end)
{
  QCTK_D(qCTKAddRemoveComboBox);

  if (_parent != d->ComboBox->rootModelIndex())
    {//rows that are to be added in the model are not displayed by the combobox
    return;
    }
    
  if (d->HasEmptyItem && !d->AddingEmptyItem)
    {
    // Remove the Empty item as some real items have been added
    d->HasEmptyItem = false;
    d->RemovingEmptyItem = true;
    d->ComboBox->removeItem(start == 0 ? end + 1 : 0);
    d->RemovingEmptyItem = false;
    
    if (d->RemoveEnabled)
      {
      d->RemovePushButton->setEnabled(true);
      }
    if (d->EditEnabled)
      {
      d->EditPushButton->setEnabled(true);
      }
    // as we removed the empty item, we need to shift the start/end items if needed
    if (start > 0 )
      {
      --start;
      --end;
      }
    }

  // don't emit signal if we are adding the Empty item
  if (!d->AddingEmptyItem)
    {
    for (int i = start; i <= end; ++i)
      {
      emit this->itemAdded(i);
      }
    }
 }

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onRowsAboutToBeRemoved(const QModelIndex & _parent, int start, int end)
{
  QCTK_D(qCTKAddRemoveComboBox);

  if (_parent != d->ComboBox->rootModelIndex())
    {//rows that are to be added in the model are not displayed by the combobox
    return;
    }

  // if the user try to remove the Empty item, don't send event
  if (d->RemovingEmptyItem)
    {
    return;
    }
  for (int i = start; i <= end; ++i)
    {
    emit this->itemAboutToBeRemoved(i);
    }
}


// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onRowsRemoved(const QModelIndex & _parent, int start, int end)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (_parent != d->ComboBox->rootModelIndex())
    {//rows that are to be added in the model are not displayed by the combobox
    return;
    }

  // the combobox is now empty, add the EmptyItem if needed
  if (d->ComboBox->count() == 0)
    {
    // Add default 'empty item'
    d->insertEmptyItem();
   
    if (d->RemoveEnabled)
      {
      d->RemovePushButton->setEnabled(false);
      }
    if (d->EditEnabled)
      {
      d->EditPushButton->setEnabled(false);
      }
    }  

  if (!d->RemovingEmptyItem)
    {
    for (int i = start; i <= end; ++i)
      {
      emit this->itemRemoved(i);
      }
    }
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setComboBoxEnabled(bool enable)
{
  qctk_d()->ComboBox->setEnabled(enable);
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::comboBoxEnabled()const
{
  //const cast as I'm not sure why isEnabledTo doesn't take a const
  return qctk_d()->ComboBox->isEnabledTo(const_cast<qCTKAddRemoveComboBox*>(this));
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setAddEnabled(bool enable)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  d->AddPushButton->setEnabled(enable);
  d->AddEnabled = enable;
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::addEnabled()const
{
  return qctk_d()->AddEnabled;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setRemoveEnabled(bool enable)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (this->count() > 0)
    {
    d->RemovePushButton->setEnabled(enable);
    }
  d->RemoveEnabled = enable;
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::removeEnabled()const
{
  return qctk_d()->RemoveEnabled;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setEditEnabled(bool enable)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (this->count() > 0)
    { 
    d->EditPushButton->setEnabled(enable);
    }
  d->EditEnabled = enable;
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::editEnabled()const
{
  return qctk_d()->EditEnabled;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setPushButtonsEnabled(bool enabled)
{
  QCTK_D(qCTKAddRemoveComboBox);

  // Update buttons state
  QList<QWidget*> buttons;
  buttons << d->AddPushButton << d->RemovePushButton << d->EditPushButton;

  foreach(QWidget* w, buttons)
    {
    w->setDisabled(enabled);
    w->setVisible(enabled);
    }
}

// --------------------------------------------------------------------------
QCTK_GET_CXX(qCTKAddRemoveComboBox, bool, pushButtonsEnabled, PushButtonEnabled)

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onAdd()
{
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onRemove()
{
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onEdit()
{
}

// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::count()const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  
  return (d->HasEmptyItem ? 0 : d->ComboBox->count());
}

// --------------------------------------------------------------------------
QCTK_GET_CXX(qCTKAddRemoveComboBox, bool, empty, HasEmptyItem);

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setCurrentIndex(int index)
{
  return qctk_d()->ComboBox->setCurrentIndex(index);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::insertItem(int index, const QString &text, const QVariant &userDataVariable)
{
  //qDebug() << __FUNCTION__ << " " << index <<  " " << text << " " << userDataVariable ;
  qctk_d()->ComboBox->insertItem(index, text, userDataVariable);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::insertItem(int index, const QIcon &icon, const QString &text, const QVariant &userDataVariable)
{
  qctk_d()->ComboBox->insertItem(index, icon, text, userDataVariable);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::insertItems(int index, const QStringList &texts)
{
  qctk_d()->ComboBox->insertItems(index, texts);
}
  
// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::findText(const QString & text, Qt::MatchFlags flags)const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    { // if the scene is empty, don't even try to find the text (it could be the
      // one of the EmptyText prop.
    return -1;
    }
  return d->ComboBox->findText(text, flags);
}

// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::findData(const QVariant &dataVariable, int role, Qt::MatchFlags flags)const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    { // if the scene is empty, don't even try to find the dataVariable 
    return -1;
    }
  return d->ComboBox->findData(dataVariable, role, flags);
}

// --------------------------------------------------------------------------
QString qCTKAddRemoveComboBox::itemText(int index) const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    {
    return QString();
    }
  return d->ComboBox->itemText(index);
}
  
// --------------------------------------------------------------------------
QVariant qCTKAddRemoveComboBox::itemData(int index, int role) const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    {
    return QVariant();
    }
  return d->ComboBox->itemData(index,role);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setItemText(int index, const QString& text)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    {
    return;
    }
  return d->ComboBox->setItemText(index, text);
}
  
// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setItemData(int index, const QVariant& dataVariable, int role)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    {
    return;
    }
  d->ComboBox->setItemData(index, dataVariable, role);
}

// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::currentIndex() const
{
  QCTK_D(const qCTKAddRemoveComboBox);
  
  return d->HasEmptyItem ? -1 : d->ComboBox->currentIndex();
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::removeItem(int index)
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    {
    return; 
    }
  d->ComboBox->removeItem(index);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::clear()
{
  QCTK_D(qCTKAddRemoveComboBox);
  
  if (d->HasEmptyItem)
    {
    return;
    }
  d->ComboBox->clear();
}

// --------------------------------------------------------------------------
QModelIndex qCTKAddRemoveComboBox::rootModelIndex()const
{
  return qctk_d()->ComboBox->rootModelIndex();
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setRootModelIndex(const QModelIndex& root)
{
  qctk_d()->ComboBox->setRootModelIndex(root);
}

// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::modelColumn()const
{
  return qctk_d()->ComboBox->modelColumn();
}

// --------------------------------------------------------------------------
QAbstractItemModel* qCTKAddRemoveComboBox::model()const
{
  return qctk_d()->ComboBox->model();
}

// --------------------------------------------------------------------------
// qCTKAddRemoveComboBoxPrivate methods

// --------------------------------------------------------------------------
qCTKAddRemoveComboBoxPrivate::qCTKAddRemoveComboBoxPrivate()
{
  this->EmptyText = "None";

  this->AddText = "Add";
  this->RemoveText = "Remove";
  this->EditText = "Edit";

  this->AddingEmptyItem = false;
  this->AddingActionItems = false;

  this->RemovingEmptyItem = false;
  this->RemovingActionItems = false;

  this->AddEnabled = true;
  this->RemoveEnabled = true;
  this->EditEnabled = true;

  this->HasEmptyItem = false;
  this->PushButtonEnabled = true;
}
    
// --------------------------------------------------------------------------
void qCTKAddRemoveComboBoxPrivate::insertActionItems()
{
  this->ComboBox->addItem(this->AddText);
  this->ComboBox->addItem(this->RemoveText);
  this->ComboBox->addItem(this->EditText);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBoxPrivate::insertEmptyItem()
{
  this->AddingEmptyItem = true;
  this->ComboBox->insertItem(0, this->EmptyText);
  this->AddingEmptyItem = false;
  this->HasEmptyItem = true;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBoxPrivate::connectComboBox(QComboBox* comboBox)
{
  QCTK_P(qCTKAddRemoveComboBox);
  QObject::connect(comboBox, SIGNAL(activated(int)),
                p, SIGNAL(activated(int)));
  QObject::connect(comboBox, SIGNAL(currentIndexChanged(int)),
                p, SIGNAL(currentIndexChanged(int)));
  /*
  this->connect(qctk_d()->ComboBox->model(),
  SIGNAL(rowsAboutToBeInserted(const QModelIndex & _parent, int start, int end )),
  SLOT(onRowsAboutToBeInserted(const QModelIndex & _parent, int start, int end )));
  */
  QObject::connect(comboBox->model(),
                SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
                p, SLOT(onRowsAboutToBeRemoved(const QModelIndex & , int , int  )));

  QObject::connect(comboBox->model(),
                SIGNAL(rowsInserted(const QModelIndex &, int, int )),
                p, SLOT(onRowsInserted(const QModelIndex &, int, int)));
  QObject::connect(comboBox->model(),
                SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                p, SLOT(onRowsRemoved(const QModelIndex &, int, int )));
}
