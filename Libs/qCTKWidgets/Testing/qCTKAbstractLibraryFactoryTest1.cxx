/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

// qCTK includes
#include "qCTKAbstractLibraryFactory.h"

// QT includes
#include <QApplication>

// STL includes
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

