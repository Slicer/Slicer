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
#include "qMRMLSortFilterProxyModel.h"

#include "TestingMacros.h"
#include <vtkEventBroker.h>

// STD includes
#include <cstdlib>
#include <iostream>

int qMRMLSceneTransformModelTest2(int argc, char * argv [] )
{
  QApplication app(argc, argv);
  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputURL_scene.mrml " << std::endl;
    return EXIT_FAILURE;
    }

  try
    {
    qMRMLSceneTransformModel model;
    qMRMLSortFilterProxyModel sort;
    sort.setSourceModel(&model);
    vtkMRMLScene* scene = vtkMRMLScene::New();
    model.setMRMLScene(scene);
    scene->SetURL(argv[1]);
    scene->Connect();
    std::cout << std::endl << "Loaded" << std::endl;

    scene->SetURL(argv[1]);
    scene->Connect();
    std::cout << std::endl << "Loaded twice" << std::endl;

    scene->Delete();

    }
  catch (const char* error)
    {
    std::cerr << error << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

