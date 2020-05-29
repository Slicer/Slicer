/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// this is a test of the slicer slice logic resampling pipeline

// Slicer includes
#include "vtkSlicerConfigure.h" // For Slicer_BUILD_WEBENGINE_SUPPORT

// Qt includes
#include <QApplication>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
#include <QWebEngineView>
#endif

// Slicer includes
#include "qSlicerWidget.h"
#include "qMRMLWidget.h"

// Slicer includes
#include <vtkMRMLSliceLogic.h>

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLVolumeArchetypeStorageNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLColorTableNode.h>

// VTK includes
#include <QVTKOpenGLNativeWidget.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkImageMapper.h>
#include <vtkProperty2D.h>
#include <vtkActor2D.h>
#include <vtkVersion.h>

// STD includes

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 900

//
// Build the slicer rendering infrastructure to render data from volume file
// into the given render window.  Populate the scene and return the slice logic.
//
vtkMRMLSliceLogic *setupSliceDisplay(vtkMRMLScene *scene, vtkRenderWindow *rw, const char *archetype)
{
  // Add default slice orientation presets
  vtkMRMLSliceNode::AddDefaultSliceOrientationPresets(scene);

  //
  // allocate needed nodes, add them to the scene, and connect them together
  //
  vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::New();
  vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::New();
  vtkMRMLVolumeArchetypeStorageNode *storageNode = vtkMRMLVolumeArchetypeStorageNode::New();
  vtkMRMLColorTableNode *colorNode = vtkMRMLColorTableNode::New();

  volumeNode->SetScene( scene );
  displayNode->SetScene( scene );
  colorNode->SetScene( scene );
  colorNode->SetTypeToGrey();
  storageNode->SetScene( scene );

  scene->AddNode( volumeNode );
  scene->AddNode( colorNode );
  scene->AddNode( displayNode );
  scene->AddNode( storageNode );

  volumeNode->SetName( "sample volume" );
  volumeNode->SetAndObserveStorageNodeID( storageNode->GetID() );
  volumeNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
  displayNode->SetAndObserveColorNodeID( colorNode->GetID() );

  // read the data
  storageNode->SetFileName( archetype );
  storageNode->ReadData( volumeNode );


  //
  // Create the slice logic to create the slice image
  //
  vtkMRMLSliceLogic *sliceLogic = vtkMRMLSliceLogic::New();
  sliceLogic->SetName( "Image Viewer" );
  sliceLogic->SetMRMLScene(scene);

  vtkMRMLSliceNode *sliceNode = sliceLogic->GetSliceNode();
  sliceNode->SetOrientationToCoronal();

  vtkMRMLSliceCompositeNode *compositeNode = sliceLogic->GetSliceCompositeNode();
  compositeNode->SetBackgroundVolumeID( volumeNode->GetID() );

  //
  // get the output slice and put it into the render window
  //
  // vtkImageData *slice = 0;
  vtkAlgorithmOutput *slicePort = sliceLogic->GetImageDataConnection();

  vtkImageMapper *mapper = vtkImageMapper::New();
  mapper->SetColorWindow( 255. );
  mapper->SetColorLevel ( 127.5 );
  mapper->SetInputConnection( slicePort );
  vtkActor2D *actor = vtkActor2D::New();
  actor->SetMapper( mapper );
  actor->GetProperty()->SetDisplayLocationToBackground();
  vtkRenderer *renderer = vtkRenderer::New();
  renderer->AddActor2D( actor );
  rw->AddRenderer( renderer );

  // clean up
  mapper->Delete();
  actor->Delete();
  renderer->Delete();
  volumeNode->Delete();
  colorNode->Delete();
  displayNode->Delete();
  storageNode->Delete();

  return( sliceLogic );
}

int qSlicerWidgetTest2(int argc, char * argv[] )
{
  if (argc != 2 && argc != 3)
    {
    std::cerr << "Line " << __LINE__ << " - Missing parameters !" << std::endl
      << "Usage:" << std::endl
      << "  Default: " << argv[0] << " /path/to/temp" << std::endl
      << "  For interactive testing: " << argv[0] << " /path/to/temp -I" << std::endl
      << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Create a simple gui with a quit button and render window
  //
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  QWidget parentWidget;
  parentWidget.setWindowTitle("qSlicerWidgetTest2");
  QVBoxLayout vbox;
  parentWidget.setLayout(&vbox);

  QPushButton quitButton;
  quitButton.setParent(&parentWidget);
  quitButton.setText("Quit");
  vbox.addWidget(&quitButton);
  app.connect(&quitButton, SIGNAL(clicked()), &parentWidget, SLOT(close()));

  qSlicerWidget* widget = new qSlicerWidget();
  widget->setParent(&parentWidget);
  vbox.addWidget(widget);

  QVTKOpenGLNativeWidget * vtkWidget = new QVTKOpenGLNativeWidget;
  vtkWidget->setEnableHiDPI(true);

  vtkWidget->setParent(&parentWidget);
  vbox.addWidget(vtkWidget);
  vtkWidget->GetRenderWindow()->Render();

#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  QWebEngineView webView;
  webView.setParent(&parentWidget);
  webView.setUrl(QUrl("http://pyjs.org/examples"));
  vbox.addWidget(&webView);
#endif

  vtkMRMLScene* scene = vtkMRMLScene::New();
  widget->setMRMLScene(scene);
  parentWidget.show();
  parentWidget.raise();

  vtkMRMLSliceLogic *sliceLogic = setupSliceDisplay(
          scene, vtkWidget->GetRenderWindow(), argv[1] );


  if (argc < 3 || QString(argv[2]) != "-I")
  {
    // quit after 5 seconds if the Quit button hasn't been clicked
    QTimer::singleShot(5000, &parentWidget, SLOT(close()));
  }

  // run the app
  app.exec();

  sliceLogic->Delete();
  scene->Delete();

  return EXIT_SUCCESS;
}

