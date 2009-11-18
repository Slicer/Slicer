
#include "qCTKAddRemoveComboBox.h"

#include "ui_qCTKAddRemoveComboBox.h" 

#include <QDebug>

//-----------------------------------------------------------------------------
struct qCTKAddRemoveComboBox::qInternal : public Ui::qCTKAddRemoveComboBox
{
  qInternal()
    {
    this->HasEmptyItem = false; 
    this->AddingEmptyItem = false;
    this->RemovingEmptyItem = false;
    this->EmptyText = "None";

    this->AddEnabled = true;
    this->RemoveEnabled = true;
    this->EditEnabled = true;
    }

  bool    HasEmptyItem; 
  bool    AddingEmptyItem; 
  bool    RemovingEmptyItem; 
  QString EmptyText;
  
  bool    AddEnabled;
  bool    RemoveEnabled;
  bool    EditEnabled; 
};

// --------------------------------------------------------------------------
qCTKAddRemoveComboBox::qCTKAddRemoveComboBox(QWidget* parent) : Superclass(parent)
{
  this->Internal = new qInternal; 
  this->Internal->setupUi(this);
  
  // connect 
  this->connectComboBox(this->Internal->ComboBox);
    
  this->connect(this->Internal->AddPushButton, SIGNAL(pressed()), SLOT(onAdd()));
  this->connect(this->Internal->RemovePushButton, SIGNAL(pressed()), SLOT(onRemove())); 
  this->connect(this->Internal->EditPushButton, SIGNAL(pressed()), SLOT(onEdit())); 

  // Add default 'empty item'
  this->Internal->AddingEmptyItem = true;
  this->Internal->ComboBox->addItem(this->Internal->EmptyText);
  this->Internal->AddingEmptyItem = false;
  this->Internal->HasEmptyItem = true;
}

// --------------------------------------------------------------------------
qCTKAddRemoveComboBox::~qCTKAddRemoveComboBox()
{
  delete this->Internal; 
}

void qCTKAddRemoveComboBox::connectComboBox(QComboBox* comboBox)
{
  this->connect(comboBox, 
                SIGNAL(activated(int)), 
                SIGNAL(activated(int)));
  this->connect(comboBox, 
                SIGNAL(currentIndexChanged(int)), 
                SIGNAL(currentIndexChanged(int)));
  /*
  this->connect(this->Internal->ComboBox->model(), 
  SIGNAL(rowsAboutToBeInserted(const QModelIndex & parent, int start, int end )),
  SLOT(onRowsAboutToBeInserted(const QModelIndex & parent, int start, int end )));
  */
  this->connect(comboBox->model(), 
                SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
                SLOT(onRowsAboutToBeRemoved(const QModelIndex & , int , int  )));  
  
  this->connect(comboBox->model(), 
                SIGNAL(rowsInserted(const QModelIndex &, int, int )),
                SLOT(onRowsInserted(const QModelIndex &, int, int)));
  this->connect(comboBox->model(), 
                SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                SLOT(onRowsRemoved(const QModelIndex &, int, int )));
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setComboBox(QComboBox* comboBox)
{
  if ((comboBox == this->Internal->ComboBox) ||
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
  this->connectComboBox(comboBox);
  this->Internal->ComboBox = comboBox;
  delete oldComboBox;

  // Add default 'empty item'
  this->Internal->AddingEmptyItem = true;
  this->Internal->ComboBox->addItem(this->Internal->EmptyText);
  this->Internal->AddingEmptyItem = false;
  this->Internal->HasEmptyItem = true;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setEmptyText(const QString& text)
{
  if (this->Internal->HasEmptyItem)
    {
    Q_ASSERT(this->Internal->ComboBox->count() == 1);
    this->setItemText(0, text);
    }
  this->Internal->EmptyText = text; 
}

// --------------------------------------------------------------------------
QString qCTKAddRemoveComboBox::emptyText()const
{
  return this->Internal->EmptyText;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onRowsInserted(const QModelIndex & parent, int start, int end)
{
  if (parent != this->Internal->ComboBox->rootModelIndex())
    {//rows that are to be added in the model are not displayed by the combobox
    return;
    }
    
  if (this->Internal->HasEmptyItem && !this->Internal->AddingEmptyItem)
    {
    // Remove the Empty item as some real items have been added
    this->Internal->HasEmptyItem = false;
    this->Internal->RemovingEmptyItem = true;
    this->Internal->ComboBox->removeItem(start == 0 ? end + 1 : 0);
    this->Internal->RemovingEmptyItem = false;
    
    if (this->Internal->RemoveEnabled)
      {
      this->Internal->RemovePushButton->setEnabled(true);
      }
    if (this->Internal->EditEnabled)
      {
      this->Internal->EditPushButton->setEnabled(true);
      }
    // as we removed the empty item, we need to shift the start/end items if needed
    if (start > 0 )
      {
      --start;
      --end;
      }
    }

  // don't emit signal if we are adding the Empty item
  if (!this->Internal->AddingEmptyItem)
    {
    for (int i = start; i <= end; ++i)
      {
      emit this->itemAdded(i);
      }
    }
 }

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onRowsAboutToBeRemoved(const QModelIndex & parent, int start, int end)
{
  if (parent != this->Internal->ComboBox->rootModelIndex())
    {//rows that are to be added in the model are not displayed by the combobox
    return;
    }

  // if the user try to remove the Empty item, don't send event
  if (this->Internal->RemovingEmptyItem)
    {
    return;
    }
  for (int i = start; i <= end; ++i)
    {
    emit this->itemAboutToBeRemoved(i);
    }
}


// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::onRowsRemoved(const QModelIndex & parent, int start, int end)
{
  if (parent != this->Internal->ComboBox->rootModelIndex())
    {//rows that are to be added in the model are not displayed by the combobox
    return;
    }

  if (this->Internal->ComboBox->count() == 0)
    {
    this->Internal->HasEmptyItem = true;
    this->Internal->AddingEmptyItem = true;
    this->Internal->ComboBox->addItem(this->Internal->EmptyText);
    this->Internal->AddingEmptyItem = false;
    if (this->Internal->RemoveEnabled)
      {
      this->Internal->RemovePushButton->setEnabled(false);
      }
    if (this->Internal->EditEnabled)
      {
      this->Internal->EditPushButton->setEnabled(false);
      }
    }  

  if (!this->Internal->RemovingEmptyItem)
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
  this->Internal->ComboBox->setEnabled(enable);
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::comboBoxEnabled()const
{
  //const cast as I'm not sure why isEnabledTo doesn't take a const
  return this->Internal->ComboBox->isEnabledTo(
    const_cast<qCTKAddRemoveComboBox*>(this));
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setAddEnabled(bool enable)
{
  this->Internal->AddPushButton->setEnabled(enable);
  this->Internal->AddEnabled = enable;
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::addEnabled()const
{
  return this->Internal->AddEnabled;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setRemoveEnabled(bool enable)
{
  if (this->count() > 0)
    {
    this->Internal->RemovePushButton->setEnabled(enable);
    }
  this->Internal->RemoveEnabled = enable;
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::removeEnabled()const
{
  return this->Internal->RemoveEnabled;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setEditEnabled(bool enable)
{
  if (this->count() > 0)
    { 
    this->Internal->EditPushButton->setEnabled(enable);
    }
  this->Internal->EditEnabled = enable;
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::editEnabled()const
{
  return this->Internal->EditEnabled;
}

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
  return (this->Internal->HasEmptyItem ? 0 : this->Internal->ComboBox->count()); 
}

// --------------------------------------------------------------------------
bool qCTKAddRemoveComboBox::empty()const
{
  return this->Internal->HasEmptyItem;
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setCurrentIndex(int index)
{
  return this->Internal->ComboBox->setCurrentIndex(index); 
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::insertItem(int index, const QString &text, const QVariant &userData)
{
  //qDebug() << __FUNCTION__ << " " << index <<  " " << text << " " << userData ;
  this->Internal->ComboBox->insertItem(index, text, userData);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::insertItem(int index, const QIcon &icon, const QString &text, const QVariant &userData)
{
  this->Internal->ComboBox->insertItem(index, icon, text, userData);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::insertItems(int index, const QStringList &texts)
{
  this->Internal->ComboBox->insertItems(index, texts);
}
  
// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::findText(const QString & text, Qt::MatchFlags flags)const
{
  if (this->Internal->HasEmptyItem)
    { // if the scene is empty, don't even try to find the text (it could be the
      // one of the EmptyText prop.
    return -1;
    }
  return this->Internal->ComboBox->findText(text, flags); 
}

// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::findData(const QVariant &data, int role, Qt::MatchFlags flags)const
{
  if (this->Internal->HasEmptyItem)
    { // if the scene is empty, don't even try to find the data 
    return -1;
    }
  return this->Internal->ComboBox->findData(data, role, flags);
}

// --------------------------------------------------------------------------
QString qCTKAddRemoveComboBox::itemText(int index) const
{
  if (this->Internal->HasEmptyItem)
    {
    return QString();
    }
  return this->Internal->ComboBox->itemText(index);
}
  
// --------------------------------------------------------------------------
QVariant qCTKAddRemoveComboBox::itemData(int index, int role) const
{
  if (this->Internal->HasEmptyItem)
    {
    return QVariant();
    }
  return this->Internal->ComboBox->itemData(index,role);
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setItemText(int index, const QString& text)
{
  if (this->Internal->HasEmptyItem)
    {
    return;
    }
  return this->Internal->ComboBox->setItemText(index, text);
}
  
// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setItemData(int index, const QVariant& data, int role)
{
  if (this->Internal->HasEmptyItem)
    {
    return;
    }
  this->Internal->ComboBox->setItemData(index, data, role);
}

// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::currentIndex() const
{
  return this->Internal->HasEmptyItem ? -1 : this->Internal->ComboBox->currentIndex();
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::removeItem(int index)
{
  if (this->Internal->HasEmptyItem)
    {
    return; 
    }
  this->Internal->ComboBox->removeItem(index);
}

// --------------------------------------------------------------------------
QModelIndex qCTKAddRemoveComboBox::rootModelIndex()const
{
  return this->Internal->ComboBox->rootModelIndex();
}

// --------------------------------------------------------------------------
void qCTKAddRemoveComboBox::setRootModelIndex(const QModelIndex& root)
{
  this->Internal->ComboBox->setRootModelIndex(root);
}

// --------------------------------------------------------------------------
int qCTKAddRemoveComboBox::modelColumn()const
{
  return this->Internal->ComboBox->modelColumn();
}

// --------------------------------------------------------------------------
QAbstractItemModel* qCTKAddRemoveComboBox::model()const
{
  return this->Internal->ComboBox->model();
}
