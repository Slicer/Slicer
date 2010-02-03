// Qt includes
#include <QApplication>

// qMRML includes
#include <qMRMLSceneFactoryWidget.h>

// MRML includes
#include <vtkMRMLScene.h>

// std includes
#include <stdlib.h>
#include <iostream>

int qMRMLSceneFactoryWidgetTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  qMRMLSceneFactoryWidget   sceneFactory;
  sceneFactory.generateScene();
  if (sceneFactory.mrmlScene() == 0)
    {
    std::cerr << "qMRMLSceneFactoryWidget::generateScene() failed" << std::endl;
    return EXIT_FAILURE;
    }
  if (sceneFactory.mrmlScene()->GetNumberOfNodes() != 0)
    {
    std::cerr << "qMRMLSceneFactoryWidget::generateScene() failed" << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.generateScene();
  if (sceneFactory.mrmlScene() == 0)
    {
    std::cerr << "qMRMLSceneFactoryWidget::generateScene() failed" << std::endl;
    return EXIT_FAILURE;
    }
  if (sceneFactory.mrmlScene()->GetNumberOfNodes() != 0)
    {
    std::cerr << "qMRMLSceneFactoryWidget::generateScene() failed" << std::endl;
    return EXIT_FAILURE;
    }
  // delete a node that doesn't exist: should not crash
  sceneFactory.deleteNode();
  sceneFactory.generateNode();
  if (sceneFactory.mrmlScene()->GetNumberOfNodes() != 1)
    {
    std::cerr << "qMRMLSceneFactoryWidget::generateScene() failed" << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
