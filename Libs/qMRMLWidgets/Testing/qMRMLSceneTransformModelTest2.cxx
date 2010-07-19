/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// QT includes
#include <QApplication>
#include <QString>
#include <QStringList>

// qMRML includes
#include "qMRMLSceneTransformModel.h"

#include "TestingMacros.h"
#include <vtkEventBroker.h>

// STD includes
#include <cstdlib>
#include <iostream>

int qMRMLSceneTransformModelTest2(int argc, char * argv [] )
{
  QApplication app(argc, argv);

  try
    {
    qMRMLSceneTransformModel model;
    vtkMRMLScene* scene = vtkMRMLScene::New();
    model.setMRMLScene(scene);
    scene->SetURL("/home/julien/data/Slicer/SPL_PNL_Brain_Atlas2008/brain_atlas_2008.mrml");
    scene->Connect();
    std::cout << std::endl << "Loaded" << std::endl;
    scene->Delete();

    }
  catch (const char* error)
    {
    std::cerr << error << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

