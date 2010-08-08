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
#include <vtkMRMLLayoutNode.h>

// VTK includes
#include <vtkSmartPointer.h>

// std includes
#include <stdlib.h>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

int qSlicerLayoutManagerTest2(int argc, char * argv[] )
{
  QApplication app(argc, argv);
  QWidget w;
  w.show();
  qSlicerLayoutManager* layoutManager = new qSlicerLayoutManager(&w);

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

  vtkMRMLLayoutNode* layoutNode = vtkMRMLLayoutNode::New();
  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
  scene->AddNode(layoutNode);
  layoutNode->Delete();
  layoutManager->setMRMLScene(scene);

  if (layoutManager->layout() != vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView ||
      layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView)
    {
    std::cerr << __LINE__ << " Add scene failed:" << std::endl
              << " Layout wanted: " << vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView
              << ", layout set: " << layoutManager->layout()
              << ", node layout: " << layoutNode->GetViewArrangement() << std::endl;
    return EXIT_FAILURE;
    }

  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
  if (layoutManager->layout() != vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView ||
      layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView)
    {
    std::cerr << __LINE__ << " Set View Arrangement on LayoutNode failed." << std::endl
              << " Layout wanted: " << vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView
              << ", layout set: " << layoutManager->layout()
              << ", node layout: " << layoutNode->GetViewArrangement() << std::endl;
    return EXIT_FAILURE;
    }

  layoutManager->setLayout(vtkMRMLLayoutNode::SlicerLayoutCompareView);
  if (layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutCompareView ||
      layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutCompareView)
    {
    std::cerr << __LINE__ << " Set View Arrangement on layout manager failed." << std::endl
              << " Layout wanted: " << vtkMRMLLayoutNode::SlicerLayoutCompareView
              << ", layout set: " << layoutManager->layout()
              << ", node layout: " << layoutNode->GetViewArrangement() << std::endl;
    return EXIT_FAILURE;
    }
  scene->SetIsImporting(true);
  scene->SetIsImporting(false);

  if (layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutCompareView ||
      layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutCompareView)
    {
    std::cerr << __LINE__ <<  " Set View Arrangement on layout manager failed." << std::endl
              << " Layout wanted: " << vtkMRMLLayoutNode::SlicerLayoutCompareView
              << ", layout set: " << layoutManager->layout()
              << ", node layout: " << layoutNode->GetViewArrangement() << std::endl;
    return EXIT_FAILURE;
    }
  scene->SetIsImporting(true);
  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
  scene->SetIsImporting(false);

  if (layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView ||
      layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView)
    {
    std::cerr << __LINE__ << "Set View Arrangement on layout manager failed." << std::endl
              << " Layout wanted: " << vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView
              << ", layout set: " << layoutManager->layout()
              << ", node layout: " << layoutNode->GetViewArrangement() << std::endl;
    return EXIT_FAILURE;
    }

  scene->InvokeEvent(vtkMRMLScene::SceneAboutToBeClosedEvent, NULL);
  scene->InvokeEvent(vtkMRMLScene::SceneClosedEvent, NULL);

  if (layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView ||
      layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView)
    {
    std::cerr << __LINE__ << "Set View Arrangement on layout manager failed." << std::endl
              << " Layout wanted: " << vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView
              << ", layout set: " << layoutManager->layout()
              << ", node layout: " << layoutNode->GetViewArrangement() << std::endl;
    return EXIT_FAILURE;
    }

  scene->InvokeEvent(vtkMRMLScene::SceneAboutToBeClosedEvent, NULL);
  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
  scene->InvokeEvent(vtkMRMLScene::SceneClosedEvent, NULL);

  if (layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView ||
      layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView)
    {
    std::cerr << __LINE__ << "Set View Arrangement on layout manager failed." << std::endl
              << " Layout wanted: " << vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView
              << ", layout set: " << layoutManager->layout()
              << ", node layout: " << layoutNode->GetViewArrangement() << std::endl;
    return EXIT_FAILURE;
    }

  QTimer autoExit;
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QObject::connect(&autoExit, SIGNAL(timeout()), &app, SLOT(quit()));
    autoExit.start(1000);
    }
  int res = app.exec();

  scene->Delete();
  delete layoutManager;
  return res;
}

