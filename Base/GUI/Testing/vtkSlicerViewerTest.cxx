
/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// GUI includes
#include "vtkKWWindowBase.h"
#include "vtkKWFrame.h"

#include "vtkSlicerViewerWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerTheme.h"

// MRML includes
#include <vtkMRMLScene.h>

// STD includes
#include <stdlib.h>

#include "TestingMacros.h"

extern "C" int Slicerbasegui_Init(Tcl_Interp *interp);
extern "C" int Slicerbaselogic_Init(Tcl_Interp *interp);
extern "C" int Mrml_Init(Tcl_Interp *interp);

int vtkSlicerViewerTest( int argc, char *argv[] )
{

  Tcl_Interp *interp = NULL;
  interp = vtkKWApplication::InitializeTcl(argc, argv, &std::cout);
  if (!interp)
    {
    std::cerr << "Error: InitializeTcl failed" << std::endl ;
    return 1;
    }

  Slicerbasegui_Init(interp);
  Slicerbaselogic_Init(interp);
  Mrml_Init(interp);


  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  std::string testScene(vtksys::SystemTools::GetEnv("Slicer_HOME"));
  testScene += "/share/MRML/Testing/vol_and_cube_camera.mrml";
  mrmlScene->SetURL(testScene.c_str());
  mrmlScene->Connect();

  vtkSlicerApplication *app = vtkSlicerApplication::GetInstance();

  vtkKWWindowBase *win = vtkKWWindowBase::New();
  win->SupportHelpOn();
  app->AddWindow(win);
  win->Create();

  vtkSlicerViewerWidget* viewerWidget = vtkSlicerViewerWidget::New();
  viewerWidget->SetAndObserveMRMLScene(mrmlScene);
  viewerWidget->SetParent( win->GetViewFrame() );
  viewerWidget->Create();
  viewerWidget->PackWidget( win->GetViewFrame() );
  viewerWidget->UpdateFromMRML();

  EXERCISE_BASIC_OBJECT_METHODS( viewerWidget );

  app->PromptBeforeExitOff();
  app->Script( "after 5000 %s Exit", app->GetTclName() );
  
  int res = app->StartApplication();
  
  viewerWidget->SetMRMLScene( NULL );
  viewerWidget->SetParent( NULL );
  viewerWidget->Delete();
  mrmlScene->Delete();
  win->Delete();
  app->Delete();

  return res;
}

