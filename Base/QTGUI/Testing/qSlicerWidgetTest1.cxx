/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageErode.h,v $
  Date:      $Date: 2006/01/06 17:56:40 $
  Version:   $Revision: 1.17 $

=========================================================================auto=*/

#include "qSlicerAbstractCoreModule.h"
#include "qSlicerWidget.h"

#include <QApplication>
#include <QWidget>

#include <vtkMRMLScene.h>
#include <stdlib.h>

int qSlicerWidgetTest1(int argc, char * argv[] )
{
  QApplication app(argc, argv);
  QWidget parentWidget;
  qSlicerWidget* widget = new qSlicerWidget();
  if (widget->mrmlScene() != 0)
    {
    std::cerr << "scene incorrectly initialized." << std::endl;
    return EXIT_FAILURE;
    }
  if (widget->parentWidget() != 0)
    {
    std::cerr << "parent widget incorrectly initialized." << std::endl;
    return EXIT_FAILURE;
    }
  widget->setParent(&parentWidget);
  if (widget->parentWidget() != &parentWidget)
    {
    std::cerr << "parent widget incorrectly set." << std::endl;
    return EXIT_FAILURE;
    }
  if (widget->isParentContainerScrollArea())
    {
    std::cerr << "parent widget is not a scroll area." << std::endl;
    return EXIT_FAILURE;
    }
  QRect parentGeometry(10,10,20,20);
  widget->setParentGeometry( parentGeometry.x(), parentGeometry.y(), 
    parentGeometry.width(), parentGeometry.height());
  if (parentWidget.geometry() != parentGeometry)
    {
    std::cerr << "parent geometry incorrectly set." << std::endl;
    return EXIT_FAILURE;
    }
  // check for infinite loop
  QObject::connect(widget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), 
                   widget, SLOT(setMRMLScene(vtkMRMLScene*)));
  vtkMRMLScene* scene = vtkMRMLScene::New();
  widget->setMRMLScene(scene);
  if (widget->mrmlScene() != scene)
    {
    std::cerr << "scene incorrectly set." << std::endl;
    return EXIT_FAILURE;
    }
  scene->Delete();
  if (widget->mrmlScene() != scene)
    {
    std::cerr << "scene has been deleted, qSlicerWidget is supposed to keep a ref on it." << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

