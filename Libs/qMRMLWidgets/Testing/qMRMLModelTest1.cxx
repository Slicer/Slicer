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

int qMRMLModelTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);

  try
    {
    qMRMLSceneModel model(0);
    qCTKModelTester tester(&model);

    qMRMLSceneGeneratorButton sceneGenerator(0);
    sceneGenerator.generateScene();

    model.setMRMLScene(sceneGenerator.mrmlScene());
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
  
    qMRMLTransformProxyModel TreeModel(0);
    qCTKModelTester TreeTester(&TreeModel);
  
    TreeModel.setSourceModel(&model);
  
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

