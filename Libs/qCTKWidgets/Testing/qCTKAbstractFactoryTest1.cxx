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
#include "qCTKAbstractFactory.h"
#include "qCTKDoubleSlider.h"

// QT includes
#include <QApplication>

// std includes
#include <stdlib.h>
#include <iostream>

int qCTKAbstractFactoryTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);

  qCTKAbstractFactory<qCTKDoubleSlider>  qctkObject;


  return EXIT_SUCCESS;
}

