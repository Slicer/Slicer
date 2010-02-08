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
#include "qCTKAbstractQObjectFactory.h"
#include "qCTKModelTester.h"

// QT includes
#include <QApplication>

// STL includes
#include <stdlib.h>
#include <iostream>

int qCTKAbstractQObjectFactoryTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);

  qCTKAbstractQObjectFactory< qCTKModelTester > qctkObject;


  return EXIT_SUCCESS;
}

