// QT includes
#include <QApplication>

// qVTK includes
#include "qVTKObjectTest.h"

// 

// std includes
#include <stdlib.h>
#include <iostream>

int qVTKObjectTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  qVTKObjectTest objectTest;
  if (!objectTest.test())
    {
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
