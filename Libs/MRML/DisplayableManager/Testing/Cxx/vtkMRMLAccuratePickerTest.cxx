/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// This guards the interactive performance of accurate picking in 3D views.
// vtkMRMLAccuratePicker is the picker shared per view (by
// vtkMRMLThreeDViewInteractorStyle) and used on every mouse move for markup
// dragging and hover read-out. A plain vtkCellPicker tests every cell of every
// pickable surface, so over a large mesh (for example a segmentation closed
// surface with millions of cells) each pick costs tens to hundreds of
// milliseconds and interaction becomes janky. vtkMRMLAccuratePicker indexes
// large surfaces with cell locators; this test checks that repeated picks over
// a large mesh stay fast.

// MRMLDisplayableManager includes
#include "vtkMRMLAccuratePicker.h"

// VTK includes
#include <vtkActor.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkTimerLog.h>

// STD includes
#include <cstdlib>
#include <iostream>

int vtkMRMLAccuratePickerTest(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  // A large, pickable surface standing in for a segmentation closed surface.
  vtkNew<vtkSphereSource> sphere;
  sphere->SetThetaResolution(900);
  sphere->SetPhiResolution(900);
  sphere->Update();
  const vtkIdType numberOfCells = sphere->GetOutput()->GetNumberOfCells();
  std::cout << "Large surface: " << numberOfCells << " cells" << std::endl;

  vtkNew<vtkPolyDataMapper> mapper;
  // SetInputData (not a pipeline connection) so the mapper's input poly data is
  // available for locator building without a render/update.
  mapper->SetInputData(sphere->GetOutput());
  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);

  vtkNew<vtkRenderer> renderer;
  renderer->AddActor(actor);
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetOffScreenRendering(1);
  renderWindow->SetSize(300, 300);
  renderWindow->AddRenderer(renderer);
  renderer->ResetCamera();
  renderWindow->Render();

  vtkNew<vtkMRMLAccuratePicker> picker;
  picker->SetTolerance(0.005);

  const int x = 150;
  const int y = 150;

  // First pick: must hit the surface, and pays the one-time locator build.
  if (!picker->Pick(x, y, 0, renderer))
  {
    std::cerr << "Failed: pick did not hit the surface at the view center" << std::endl;
    return EXIT_FAILURE;
  }
  if (picker->GetActor() == nullptr)
  {
    std::cerr << "Failed: pick did not report the surface actor" << std::endl;
    return EXIT_FAILURE;
  }

  // Repeated picks (as during a drag or hover) must be fast because the picker
  // is locator-accelerated. Without the locator each pick is O(number of cells)
  // and takes tens to hundreds of milliseconds on this many cells.
  const int numberOfPicks = 50;
  vtkNew<vtkTimerLog> timer;
  timer->StartTimer();
  for (int i = 0; i < numberOfPicks; ++i)
  {
    picker->Pick(x + (i % 7) - 3, y + (i % 5) - 2, 0, renderer);
  }
  timer->StopTimer();
  const double millisecondsPerPick = timer->GetElapsedTime() / numberOfPicks * 1000.0;
  std::cout << "vtkMRMLAccuratePicker over " << numberOfCells << " cells: " << millisecondsPerPick << " ms/pick" << std::endl;

  // Generous threshold: the accelerated pick is well under a millisecond, while
  // an un-indexed brute-force pick over this mesh is far above 30 ms on any
  // hardware. The wide gap keeps the test insensitive to machine speed.
  const double thresholdMillisecondsPerPick = 30.0;
  if (millisecondsPerPick > thresholdMillisecondsPerPick)
  {
    std::cerr << "Failed: " << millisecondsPerPick << " ms/pick (threshold " << thresholdMillisecondsPerPick
              << " ms). Large-surface picks are not locator-accelerated, so control-point dragging "
              << "and hover read-out would be janky when a large surface is shown." << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
