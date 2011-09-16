/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// EMSegment includes
#include "vtkPlotGaussian.h"

// VTK includes
#include "vtkPlot.h"
#include "vtkLookupTableItem.h"
#include "vtkChartXY.h"
#include "vtkContextScene.h"
#include "vtkContextView.h"
#include "vtkRegressionTestImage.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"

// STD includes
#include <iostream>

int vtkPlotGaussianTest1( int argc, char * argv [] )
{
  // Set up a 2D scene, add an XY chart to it
  vtkSmartPointer<vtkContextView> view =
    vtkSmartPointer<vtkContextView>::New();
  view->GetRenderer()->SetBackground(1.0, 1.0, 1.0);
  view->GetRenderWindow()->SetSize(400, 300);
  vtkSmartPointer<vtkChartXY> chart = vtkSmartPointer<vtkChartXY>::New();
  chart->SetTitle("Chart");
  view->GetScene()->AddItem(chart);

  vtkSmartPointer<vtkPlotGaussian> item =
    vtkSmartPointer<vtkPlotGaussian>::New();
  item->SetMean(1.f);
  item->SetCovariance(1.f);
  item->SetLog(false);
  item->SetColor(185, 74, 74, 255);
  chart->AddPlot(item);

  vtkSmartPointer<vtkPlotGaussian> item2 =
    vtkSmartPointer<vtkPlotGaussian>::New();
  item2->SetMean(1.f);
  item2->SetCovariance(1.f);
  item2->SetLog(true);
  item2->SetColor(223, 135, 19, 255);
  chart->AddPlot(item2);

  vtkSmartPointer<vtkPlotGaussian> item3 =
    vtkSmartPointer<vtkPlotGaussian>::New();
  item3->SetMean(1.f);
  item3->SetCovariance(2.f);
  item3->SetLog(false);
  item3->SetColor(137, 223, 0, 255);
  chart->AddPlot(item3);

  vtkSmartPointer<vtkPlotGaussian> item4 =
    vtkSmartPointer<vtkPlotGaussian>::New();
  item4->SetMean(2.f);
  item4->SetCovariance(1.f);
  item4->SetColor(74, 74, 186, 255);
  chart->AddPlot(item4);

  vtkSmartPointer<vtkPlotGaussian> item5 =
    vtkSmartPointer<vtkPlotGaussian>::New();
  item5->SetMean(1.f);
  item5->SetCovariance(1.f);
  item5->SetProbability(2.f);
  item5->SetColor(74, 186, 149, 255);
  chart->AddPlot(item5);
  // next color: 43 138 59

  //Finally render the scene and compare the image to a reference image
  view->GetRenderWindow()->SetMultiSamples(0);
  int retVal = vtkRegressionTestImage(view->GetRenderWindow());
  if (retVal == vtkRegressionTester::DO_INTERACTOR)
    {
    view->GetInteractor()->Initialize();
    view->GetInteractor()->Start();
    }

  return !retVal;
}
