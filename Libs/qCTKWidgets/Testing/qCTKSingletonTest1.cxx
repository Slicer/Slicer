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
#include "qCTKSingleton.h"

// QT includes
#include <QApplication>

// STL includes
#include <stdlib.h>
#include <iostream>

class qCTKSingletonHelper
{
public:
  QCTK_SINGLETON_DECLARE(qCTKSingletonHelper);
};

void qCTKSingletonHelper::classInitialize()
{
}

int qCTKSingletonTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);

  qCTKSingletonHelper qctkObject;

  qctkObject.classInitialize();


  return EXIT_SUCCESS;
}

