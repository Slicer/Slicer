/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// qCTK includes
#include "qCTKAbstractLibraryFactory.h"

// QT includes
#include <QApplication>
#include <QList>
#include <QTreeWidget>
#include <QString>
#include <QStringList>

// std includes
#include <stdlib.h>
#include <iostream>

class BaseClassHelperType
{
public:
};

class FactoryItemHelper
{
public:
  FactoryItemHelper( QString &, QString )
    {
    }
  void setSymbols(const QStringList& )
    {
    }
};

int qCTKAbstractLibraryFactoryTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);

//  typedef qCTKAbstractLibraryFactory < BaseClassHelperType, FactoryItemHelper >  FactoryType;
//  FactoryType  qctkObject;


  return EXIT_SUCCESS;
}

