/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageErode.h,v $
  Date:      $Date: 2006/01/06 17:56:40 $
  Version:   $Revision: 1.17 $

=========================================================================auto=*/

// Qt includes
#include <QApplication>
#include <QTimer>
#include <QWidget>

// SlicerQt includes
#include "qSlicerLayoutManager.h"

// MRML includes
#include <vtkMRMLScene.h>

// std includes
#include <stdlib.h>

int qSlicerLayoutManagerTest1(int argc, char * argv[] )
{
  QApplication app(argc, argv);
  qSlicerLayoutManager* layoutManager = new qSlicerLayoutManager(0);

  vtkMRMLScene* scene = vtkMRMLScene::New();
  layoutManager->setMRMLScene(scene);
  if (layoutManager->mrmlScene() != scene)
    {
    std::cerr << "scene incorrectly set." << std::endl;
    return EXIT_FAILURE;
    }
  layoutManager->setMRMLScene(0);
  scene->Delete();
  scene = vtkMRMLScene::New();
  layoutManager->setMRMLScene(scene);
  QWidget* viewport = new QWidget(0);
  viewport->setWindowTitle("Old widget");
  layoutManager->setViewport(viewport);
  viewport->show();

  layoutManager->setViewport(0);
  layoutManager->setViewport(viewport);

  QWidget* viewport2 = new QWidget(0);
  viewport2->setWindowTitle("New widget");
  layoutManager->setViewport(viewport2);
  viewport2->show();

  QTimer autoExit;
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QObject::connect(&autoExit, SIGNAL(timeout()), &app, SLOT(quit()));
    autoExit.start(1000);
    }
  int res = app.exec();

  scene->Delete();
  delete layoutManager;
  delete viewport;
  delete viewport2;
  return res;
}

