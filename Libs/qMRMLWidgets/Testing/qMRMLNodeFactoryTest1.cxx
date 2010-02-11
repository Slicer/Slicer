#include <qMRMLNodeFactory.h>
#include <QApplication>

// STD includes
#include <stdlib.h>
#include <iostream>

int qMRMLNodeFactoryTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  qMRMLNodeFactory   mrmlItem;
  
  return EXIT_SUCCESS;
}
