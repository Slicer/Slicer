/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKTreeComboBox_h
#define __qCTKTreeComboBox_h

/// qCTK includes
#include "qCTKPimpl.h"

/// QT includes
#include <QComboBox>

#include "qCTKWidgetsExport.h"

/// Description:
/// ComboBox that displays the items as a tree view.
/// See below for a use case:
///    qCTKTreeComboBox combo;
///    QStandardItemModel model;
///    model.appendRow(new QStandardItem("Test1"));
///    model.item(0)->appendRow(new QStandardItem("Test1.1"));
///    model.item(0)->appendRow(new QStandardItem("Test1.2"));
///    model.item(0)->appendRow(new QStandardItem("Test1.3"));
///    model.appendRow(new QStandardItem("Test2"));
///    model.appendRow(new QStandardItem("Test3"));
///    combo.setModel(&model);
///    combo.show();
//
class qCTKTreeComboBoxPrivate;

class QCTK_WIDGETS_EXPORT qCTKTreeComboBox : public QComboBox
{
  Q_OBJECT
public:
  typedef QComboBox Superclass;
  explicit qCTKTreeComboBox(QWidget* parent = 0);
  virtual ~qCTKTreeComboBox(){}

  virtual bool eventFilter(QObject* object, QEvent* event);
  virtual void showPopup();
  virtual void hidePopup();

protected:
  virtual void paintEvent(QPaintEvent*);
  
protected slots:
  void onExpanded(const QModelIndex&);
  void onCollapsed(const QModelIndex&);
  
private:
  QCTK_DECLARE_PRIVATE(qCTKTreeComboBox);
};

#endif
