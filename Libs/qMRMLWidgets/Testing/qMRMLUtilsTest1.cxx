#include <qMRMLUtils.h>
#include <QApplication>

#include <vtkMRMLSliceNode.h>

// STD includes
#include <stdlib.h>
#include <iostream>

int qMRMLUtilsTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  vtkMRMLSliceNode * mrmlNode = vtkMRMLSliceNode::New();

  const int numberOfChildren = qMRMLUtils::childCount( mrmlNode );

  std::cout << "number of children " << numberOfChildren << std::endl;

  //  FIXME
  //  FIXME
  //  FIXME
  //  FIXME   Many other methods to be exercised here...
  //  FIXME
  //  FIXME
  //  FIXME
  //  FIXME


  mrmlNode->Delete();
  
  return EXIT_SUCCESS;
}
