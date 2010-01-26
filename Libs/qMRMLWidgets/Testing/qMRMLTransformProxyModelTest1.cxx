/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// qMRML includes
#include "qMRMLNodeFactoryButton.h"
#include "qMRMLSceneGeneratorButton.h"
#include "qMRMLSceneModel.h"
#include "qMRMLTreeProxyModel.h"
#include "qMRMLTransformProxyModel.h"

#include "TestingMacros.h"
#include <vtkEventBroker.h>

// qCTK includes
#include <qCTKModelTester.h>

// QT includes
#include <QApplication>
#include <QString>
#include <QStringList>

// std includes
#include <stdlib.h>
#include <iostream>

int qMRMLTransformProxyModelTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);

  try
    {
    qMRMLSceneModel model(0);
    qCTKModelTester tester(&model);

    qMRMLSceneGeneratorButton sceneGenerator(0);
    sceneGenerator.generateScene();

    model.setMRMLScene(sceneGenerator.mrmlScene());
    if (model.mrmlScene() != sceneGenerator.mrmlScene())
      {
      //std::cerr << "Scene not set." << std::endl;
      return EXIT_FAILURE;
      }
    // check one more time, just to make sure a reset works correctly
    model.setMRMLScene(sceneGenerator.mrmlScene());

    qMRMLNodeFactoryButton nodeFactory(0);
    nodeFactory.setMRMLScene(sceneGenerator.mrmlScene());
  
    nodeFactory.generateRandomNode();
    nodeFactory.deleteRandomNode();
  
    nodeFactory.generateRandomNode();
    nodeFactory.deleteRandomNode();
  
    nodeFactory.generateRandomNode();
    nodeFactory.generateRandomNode();
  
    nodeFactory.deleteRandomNode();
    nodeFactory.deleteRandomNode();
  
    qMRMLTransformProxyModel transformModel(0);
    qCTKModelTester transformTester(&transformModel);
  
    transformModel.setSourceModel(&model);
  
    nodeFactory.generateRandomNode();
    nodeFactory.deleteRandomNode();
  
    nodeFactory.generateRandomNode();
    nodeFactory.deleteRandomNode();
  
    nodeFactory.generateRandomNode();
    nodeFactory.generateRandomNode();
  
    nodeFactory.deleteRandomNode();
    nodeFactory.deleteRandomNode();

    for( int i = 0; i < 100; ++i)
      {
      nodeFactory.deleteRandomNode();
      }
    for( int i = 0; i < 100; ++i)
      {
      nodeFactory.generateRandomNode();
      }
    for( int i = 0; i < 99; ++i)
      {
      nodeFactory.deleteRandomNode();
      }
    }
  catch (const char* error)
    {
    std::cerr << error << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
