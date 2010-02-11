/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// qMRML includes
#include "qMRMLSceneFactoryWidget.h"
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

// STD includes
#include <stdlib.h>
#include <iostream>

int qMRMLModelTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);

  try
    {
    qMRMLSceneModel model(0);
    qCTKModelTester tester(&model);

    qMRMLSceneFactoryWidget sceneFactory(0);
    sceneFactory.generateScene();

    model.setMRMLScene(sceneFactory.mrmlScene());
    model.setMRMLScene(sceneFactory.mrmlScene());

    sceneFactory.generateNode();
    sceneFactory.deleteNode();
  
    sceneFactory.generateNode();
    sceneFactory.deleteNode();
  
    sceneFactory.generateNode();
    sceneFactory.generateNode();
  
    sceneFactory.deleteNode();
    sceneFactory.deleteNode();
  
    qMRMLTransformProxyModel TreeModel(0);
    qCTKModelTester TreeTester(&TreeModel);
  
    TreeModel.setSourceModel(&model);
  
    sceneFactory.generateNode();
    sceneFactory.deleteNode();
  
    sceneFactory.generateNode();
    sceneFactory.deleteNode();
  
    sceneFactory.generateNode();
    sceneFactory.generateNode();
  
    sceneFactory.deleteNode();
    sceneFactory.deleteNode();

    for( int i = 0; i < 100; ++i)
      {
      sceneFactory.deleteNode();
      }
    for( int i = 0; i < 100; ++i)
      {
      sceneFactory.generateNode();
      }
    for( int i = 0; i < 99; ++i)
      {
      sceneFactory.deleteNode();
      }
    }
  catch (const char* error)
    {
    std::cerr << error << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

