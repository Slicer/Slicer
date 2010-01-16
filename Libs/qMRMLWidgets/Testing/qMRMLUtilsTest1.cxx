#include <qMRMLUtils.h>
#include <QApplication>

#include <vtkMRMLNode.h>

// std includes
#include <stdlib.h>
#include <iostream>

int qMRMLUtilsTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  vtkMRMLNode * mrmlNode = vtkMRMLNode::New();

  const int numberOfChildren = qMRMLUtils::childCount( mrmlNode );

  std::cout << "number of children " << numberOfChildren << std::endl;

  //  FIXME   Many other methods to be exercised here...


  mrmlNode->Delete();
  
  return EXIT_SUCCESS;
}
