/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// qCTK includes
#include "qCTKModelTester.h"

// QT includes
#include <QApplication>
#include <QList>
#include <QTreeWidget>
#include <QString>
#include <QStringList>
#include <QModelIndex>

// std includes
#include <stdlib.h>
#include <iostream>

class QAbstractItemModelHelper : public QAbstractItemModel
{
public:
  virtual QModelIndex index(int, int, const QModelIndex&) const { return QModelIndex(); }
  virtual QModelIndex parent(const QModelIndex&) const { return QModelIndex(); }
  virtual int rowCount(const QModelIndex&) const { return 0; }
  virtual int columnCount(const QModelIndex&) const { return 0; }
  virtual QVariant data(const QModelIndex&, int) const { return QVariant(); }
};

int qCTKModelTesterTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);

  QAbstractItemModelHelper * item = new QAbstractItemModelHelper;
  QObject * object = new QObject; 

  qCTKModelTester qctkObject( item, object );

  delete item;

  return EXIT_SUCCESS;
}

