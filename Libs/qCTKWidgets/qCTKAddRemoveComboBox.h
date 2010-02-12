/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKAddRemoveComboBox_h
#define __qCTKAddRemoveComboBox_h

/// qCTK includes
#include "qCTKPimpl.h"

/// QT includes
#include <QWidget>
#include <QVariant>
#include <QModelIndex>

#include "qCTKWidgetsExport.h"

class QComboBox;
class qCTKAddRemoveComboBoxPrivate;

class QCTK_WIDGETS_EXPORT qCTKAddRemoveComboBox : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString emptyText READ emptyText WRITE setEmptyText)
  Q_PROPERTY(bool addEnabled READ addEnabled WRITE setAddEnabled)
  Q_PROPERTY(bool removeEnabled READ removeEnabled WRITE setRemoveEnabled)
  Q_PROPERTY(bool editEnabled READ editEnabled WRITE setEditEnabled)
  
public:
  /// Superclass typedef
  typedef QWidget Superclass;
  
  /// Constructors
  explicit qCTKAddRemoveComboBox(QWidget* parent = 0);
  virtual ~qCTKAddRemoveComboBox(){}
  virtual void printAdditionalInfo();
  
  /// 
  /// Set text that should be displayed in the comboBox when it is empty
  void setEmptyText(const QString& text); 
  QString emptyText()const;
  
  /// 
  /// Enable/Disable the add button. 
  void setComboBoxEnabled(bool enable);
  bool comboBoxEnabled()const; 
  
  /// 
  /// Enable/Disable the add button. 
  void setAddEnabled(bool enable);
  bool addEnabled()const; 
  
  /// 
  /// Enable/Disable the add button. 
  void setRemoveEnabled(bool enable);
  bool removeEnabled()const;
  
  /// 
  /// Enable/Disable the edit button. 
  void setEditEnabled(bool enable); 
  bool editEnabled()const;
  
  inline void addItem(const QString &text, const QVariant &userDataVariable = QVariant() )
    {this->insertItem(this->count(), text, userDataVariable);}
  inline void addItem(const QIcon &icon, const QString &text, const QVariant &userDataVariable = QVariant() )
    {this->insertItem(this->count(), icon, text, userDataVariable);}
  inline void addItems(const QStringList &texts )
    {this->insertItems(this->count(), texts);}
    
  void insertItem(int index, const QString &text, const QVariant &userDataVariable = QVariant() );
  void insertItem(int index, const QIcon &icon, const QString &text, const QVariant &userDataVariable = QVariant() );
  void insertItems(int index, const QStringList &texts);  
  
  /// 
  /// Return the number of item
  int count()const;
  bool empty()const;
    
  /// 
  /// Returns the index of the item containing the given text; otherwise returns -1.
  /// The flags specify how the items in the combobox are searched.
  int findText(const QString& text, Qt::MatchFlags flags = Qt::MatchExactly | Qt::MatchCaseSensitive ) const;
  int findData(const QVariant & data, int role = Qt::UserRole, Qt::MatchFlags flags = Qt::MatchExactly | Qt::MatchCaseSensitive ) const;

  /// 
  QString   itemText(int index) const;
  QVariant  itemData(int index, int role = Qt::UserRole) const;

  void setItemText(int index, const QString& text);
  void setItemData(int index, const QVariant& data, int role = Qt::UserRole);

  /// 
  /// Return the current item
  int       currentIndex() const;
  inline QString  currentText() const
    {return this->itemText(this->currentIndex());}
  inline QVariant currentData(int role = Qt::UserRole) const
    {return this->itemData(this->currentIndex(), role);}

  /// 
  /// Remove the item currently selected. See signal 'itemRemoved'
  void removeItem(int index);
  inline void removeCurrentItem()
    {this->removeItem(this->currentIndex());}

  /// 
  /// Remove all the items
  void clear();

signals:
  void currentIndexChanged(int index);
  void activated(int index);

  /// 
  /// This signal is sent after the method 'addItem' has been called programmatically
  void itemAdded(int index);
  
  /// 
  void itemAboutToBeRemoved(int index);
  void itemRemoved(int index);
    
public slots:
  /// 
  /// Select the current index
  void setCurrentIndex(int index);

protected slots:
  /// 
  virtual void onAdd();
  virtual void onRemove();
  virtual void onEdit();

protected:
  void setComboBox(QComboBox* comboBox);
  QModelIndex rootModelIndex()const;
  void setRootModelIndex(const QModelIndex& root);
  int modelColumn()const;
  QAbstractItemModel* model()const;

private slots:
  //void onRowsAboutToBeInserted(const QModelIndex & parent, int start, int end );
  void onRowsAboutToBeRemoved(const QModelIndex & parent, int start, int end);
  void onRowsInserted(const QModelIndex & parent, int start, int end);
  void onRowsRemoved(const QModelIndex & parent, int start, int end);

private:
  QCTK_DECLARE_PRIVATE(qCTKAddRemoveComboBox);
};

#endif
