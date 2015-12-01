
// VTK includes
#include <vtkActor.h>
#include <vtkBiDimensionalWidget.h>
#include <vtkCommand.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSphereSource.h>

// STD includes
#include <sstream>
#include <string>

#include "vtkMRMLCoreTestingMacros.h"

// The actual test function
int TestAnnotationWidgetsStandalone( int argc, char *argv[] )
{
  bool retVal = false;

  vtkSphereSource *ss = vtkSphereSource::New();

  vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
  mapper->SetInputConnection(ss->GetOutputPort());
  vtkActor *actor = vtkActor::New();
  actor->SetMapper(mapper);


  // Create the RenderWindow, Renderer
  //
  vtkRenderer *renderer = vtkRenderer::New();
  vtkRenderWindow *renderWindow = vtkRenderWindow::New();
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(600,600);

  vtkRenderWindowInteractor *renderInteractor = vtkRenderWindowInteractor::New();
  renderInteractor->SetRenderWindow(renderWindow);
  vtkInteractorStyleSwitch* iStyle = vtkInteractorStyleSwitch::New();
  iStyle->SetCurrentStyleToTrackballCamera();

  renderInteractor->SetInteractorStyle(iStyle);

  renderInteractor->Initialize();

  renderer->AddActor(actor);
  renderer->SetBackground(0.1, 0.2, 0.4);

  // Create a test pipeline
  //
  vtkBiDimensionalWidget * bidimensionalWidget = vtkBiDimensionalWidget::New();

  bidimensionalWidget->SetInteractor(renderInteractor);
  bidimensionalWidget->SetCurrentRenderer(renderer);

  bidimensionalWidget->CreateDefaultRepresentation();

  bidimensionalWidget->On();
  renderWindow->Render();


  vtkNew<vtkInteractorEventRecorder> recorder;
  recorder->SetInteractor(renderInteractor);
  recorder->ReadFromInputStringOn();

  std::ostringstream o;

  double position1[2];
  double position2[2];
  double position3[2];
  double position4[2];

  position1[0] = 30;
  position1[1] = 30;
  position2[0] = 10;
  position2[1] = 40;
  position3[0] = 50;
  position3[1] = 70;
  position4[0] = 100;
  position4[1] = 100;

  o << "EnterEvent 2 184 0 0 0 0 0\n";
  o << "MouseMoveEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o << "LeftButtonPressEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o << "RenderEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o << "LeftButtonReleaseEvent " << position1[0] << " " << position1[1] << " 0 0 0 0 t\n";
  o << "MouseMoveEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o << "LeftButtonPressEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o << "RenderEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o << "LeftButtonReleaseEvent " << position2[0] << " " << position2[1] << " 0 0 0 0 t\n";
  o << "LeftButtonPressEvent " << position4[0] << " " << position4[1] << " 0 0 0 0\n";
  o << "RenderEvent " << position4[0] << " " << position4[1] << " 0 0 0 0\n";
  o << "LeftButtonReleaseEvent " << position4[0] << " " << position4[1] << " 0 0 0 0 t\n";
  o << "ExitEvent " << position4[0] << " " << position4[1] << " 0 0 113 1 q\n";

  recorder->SetInputString(o.str().c_str());
  recorder->Play();




  vtkBiDimensionalWidget * bidimensionalWidget2 = vtkBiDimensionalWidget::New();

  bidimensionalWidget2->SetInteractor(renderInteractor);
  bidimensionalWidget2->SetCurrentRenderer(renderer);

  bidimensionalWidget2->CreateDefaultRepresentation();

  bidimensionalWidget2->On();
  renderWindow->Render();


  std::ostringstream o2;

  position1[0] = 300;
  position1[1] = 300;
  position2[0] = 100;
  position2[1] = 400;
  position3[0] = 500;
  position3[1] = 700;
  position4[0] = 1000;
  position4[1] = 1000;

  o2 << "EnterEvent 2 184 0 0 0 0 0\n";
  o2 << "MouseMoveEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o2 << "LeftButtonPressEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o2 << "RenderEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o2 << "LeftButtonReleaseEvent " << position1[0] << " " << position1[1] << " 0 0 0 0 t\n";
  o2 << "MouseMoveEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o2 << "LeftButtonPressEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o2 << "RenderEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o2 << "LeftButtonReleaseEvent " << position2[0] << " " << position2[1] << " 0 0 0 0 t\n";
  o2 << "LeftButtonPressEvent " << position4[0] << " " << position4[1] << " 0 0 0 0\n";
  o2 << "RenderEvent " << position4[0] << " " << position4[1] << " 0 0 0 0\n";
  o2 << "LeftButtonReleaseEvent " << position4[0] << " " << position4[1] << " 0 0 0 0 t\n";
  o2 << "ExitEvent " << position4[0] << " " << position4[1] << " 0 0 113 1 q\n";

  recorder->SetInputString(o2.str().c_str());
  recorder->Play();




  vtkBiDimensionalWidget * bidimensionalWidget3 = vtkBiDimensionalWidget::New();

  bidimensionalWidget3->SetInteractor(renderInteractor);
  bidimensionalWidget3->SetCurrentRenderer(renderer);

  bidimensionalWidget3->CreateDefaultRepresentation();

  bidimensionalWidget3->On();
  renderWindow->Render();


  std::ostringstream o3;

  position1[0] = 200;
  position1[1] = 200;
  position2[0] = 200;
  position2[1] = 300;
  position3[0] = 300;
  position3[1] = 500;
  position4[0] = 600;
  position4[1] = 600;

  o3 << "EnterEvent 2 184 0 0 0 0 0\n";
  o3 << "MouseMoveEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o3 << "LeftButtonPressEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o3 << "RenderEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o3 << "LeftButtonReleaseEvent " << position1[0] << " " << position1[1] << " 0 0 0 0 t\n";
  o3 << "MouseMoveEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o3 << "LeftButtonPressEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o3 << "RenderEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o3 << "LeftButtonReleaseEvent " << position2[0] << " " << position2[1] << " 0 0 0 0 t\n";
  o3 << "LeftButtonPressEvent " << position4[0] << " " << position4[1] << " 0 0 0 0\n";
  o3 << "RenderEvent " << position4[0] << " " << position4[1] << " 0 0 0 0\n";
  o3 << "LeftButtonReleaseEvent " << position4[0] << " " << position4[1] << " 0 0 0 0 t\n";
  o3 << "ExitEvent " << position4[0] << " " << position4[1] << " 0 0 113 1 q\n";

  recorder->SetInputString(o3.str().c_str());
  recorder->Play();



  vtkBiDimensionalWidget * bidimensionalWidget4 = vtkBiDimensionalWidget::New();

  bidimensionalWidget4->SetInteractor(renderInteractor);
  bidimensionalWidget4->SetCurrentRenderer(renderer);

  bidimensionalWidget4->CreateDefaultRepresentation();

  bidimensionalWidget4->On();
  renderWindow->Render();


  std::ostringstream o4;

  position1[0] = 200;
  position1[1] = 200;
  position2[0] = 200;
  position2[1] = 300;
  position3[0] = 300;
  position3[1] = 500;
  position4[0] = 600;
  position4[1] = 600;

  o4 << "EnterEvent 2 184 0 0 0 0 0\n";
  o4 << "MouseMoveEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o4 << "LeftButtonPressEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o4 << "RenderEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o4 << "LeftButtonReleaseEvent " << position1[0] << " " << position1[1] << " 0 0 0 0 t\n";
  o4 << "MouseMoveEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o4 << "LeftButtonPressEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o4 << "RenderEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o4 << "LeftButtonReleaseEvent " << position2[0] << " " << position2[1] << " 0 0 0 0 t\n";
  o4 << "LeftButtonPressEvent " << position4[0] << " " << position4[1] << " 0 0 0 0\n";
  o4 << "RenderEvent " << position4[0] << " " << position4[1] << " 0 0 0 0\n";
  o4 << "LeftButtonReleaseEvent " << position4[0] << " " << position4[1] << " 0 0 0 0 t\n";
  o4 << "ExitEvent " << position4[0] << " " << position4[1] << " 0 0 113 1 q\n";

  recorder->SetInputString(o4.str().c_str());
  recorder->Play();




  vtkBiDimensionalWidget * bidimensionalWidget5 = vtkBiDimensionalWidget::New();

  bidimensionalWidget5->SetInteractor(renderInteractor);
  bidimensionalWidget5->SetCurrentRenderer(renderer);

  bidimensionalWidget5->CreateDefaultRepresentation();

  bidimensionalWidget5->On();
  renderWindow->Render();


  std::ostringstream o5;

  position1[0] = 250;
  position1[1] = 250;
  position2[0] = 250;
  position2[1] = 350;
  position3[0] = 350;
  position3[1] = 550;
  position4[0] = 650;
  position4[1] = 650;

  o5 << "EnterEvent 2 184 0 0 0 0 0\n";
  o5 << "MouseMoveEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o5 << "LeftButtonPressEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o5 << "RenderEvent " << position1[0] << " " << position1[1] << " 0 0 0 0\n";
  o5 << "LeftButtonReleaseEvent " << position1[0] << " " << position1[1] << " 0 0 0 0 t\n";
  o5 << "MouseMoveEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o5 << "LeftButtonPressEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o5 << "RenderEvent " << position2[0] << " " << position2[1] << " 0 0 0 0\n";
  o5 << "LeftButtonReleaseEvent " << position2[0] << " " << position2[1] << " 0 0 0 0 t\n";
  o5 << "LeftButtonPressEvent " << position4[0] << " " << position4[1] << " 0 0 0 0\n";
  o5 << "RenderEvent " << position4[0] << " " << position4[1] << " 0 0 0 0\n";
  o5 << "LeftButtonReleaseEvent " << position4[0] << " " << position4[1] << " 0 0 0 0 t\n";
  o5 << "ExitEvent " << position4[0] << " " << position4[1] << " 0 0 113 1 q\n";

  recorder->SetInputString(o5.str().c_str());
  recorder->Play();




  //renderInteractor->Start();

  ss->Delete();
  mapper->Delete();
  actor->Delete();
  bidimensionalWidget->Delete();
  bidimensionalWidget2->Delete();
  bidimensionalWidget3->Delete();
  bidimensionalWidget4->Delete();
  bidimensionalWidget5->Delete();
  iStyle->Delete();
  renderInteractor->Delete();
  renderWindow->Delete();
  renderer->Delete();

  return !retVal;
}

