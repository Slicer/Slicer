/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

// Slicer includes
#include "vtkSlicerConfigure.h"

// qMRML includes
#include "qMRMLPlotView.h"
#include "qMRMLPlotWidget.h"
#include "qMRMLPlotViewControllerWidget.h"

// MRML includes
#include "vtkMRMLPlotSeriesNode.h"
#include "vtkMRMLPlotChartNode.h"
#include "vtkMRMLPlotViewNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLTableNode.h"

// VTK includes
#include <vtkFloatArray.h>
#include <vtkNew.h>
#include <vtkTable.h>
#include "qMRMLWidget.h"

int qMRMLPlotViewTest1( int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkNew<vtkMRMLScene> scene;

  // qMRMLPlotViewControllerWidget requires selection node
  vtkNew<vtkMRMLSelectionNode> selectionNode;
  scene->AddNode(selectionNode.GetPointer());

  // Create a vtkTable
  vtkNew<vtkTable> table;

  vtkNew<vtkFloatArray> arrX;
  arrX->SetName("X Axis");
  table->AddColumn(arrX.GetPointer());

  vtkNew<vtkFloatArray> arrC;
  arrC->SetName("Cosine");
  table->AddColumn(arrC.GetPointer());

  vtkNew<vtkFloatArray> arrS;
  arrS->SetName("Sine");
  table->AddColumn(arrS.GetPointer());

  // Fill in the table with some example values
  int numPoints = 69;
  float inc = 7.5 / (numPoints-1);
  table->SetNumberOfRows(numPoints);
  for (int i = 0; i < numPoints; ++i)
    {
    table->SetValue(i, 0, i * inc);
    table->SetValue(i, 1, cos(i * inc));
    table->SetValue(i, 2, sin(i * inc));
    }

  // Create a MRMLTableNode
  vtkNew<vtkMRMLTableNode> tableNode;
  scene->AddNode(tableNode.GetPointer());
  tableNode->SetAndObserveTable(table.GetPointer());

  // Create two plotSeriesNodes
  vtkNew<vtkMRMLPlotSeriesNode> plotSeriesNode1;
  vtkNew<vtkMRMLPlotSeriesNode> plotSeriesNode2;
  scene->AddNode(plotSeriesNode1.GetPointer());
  scene->AddNode(plotSeriesNode2.GetPointer());

  // Set and Observe the MRMLTableNode
  plotSeriesNode1->SetAndObserveTableNodeID(tableNode->GetID());
  plotSeriesNode1->SetXColumnName(tableNode->GetColumnName(0));
  plotSeriesNode1->SetYColumnName(tableNode->GetColumnName(1));
  plotSeriesNode2->SetAndObserveTableNodeID(tableNode->GetID());
  plotSeriesNode2->SetXColumnName(tableNode->GetColumnName(0));
  plotSeriesNode2->SetYColumnName(tableNode->GetColumnName(2));

  // Create a PlotChart node
  vtkNew<vtkMRMLPlotChartNode> plotChartNode;
  scene->AddNode(plotChartNode.GetPointer());
  // Add and Observe plots IDs in PlotChart
  plotSeriesNode1->SetName(arrC->GetName());
  plotChartNode->AddAndObservePlotSeriesNodeID(plotSeriesNode1->GetID());
  plotSeriesNode2->SetName(arrS->GetName());
  plotChartNode->AddAndObservePlotSeriesNodeID(plotSeriesNode2->GetID());

  // Create PlotView node
  vtkNew<vtkMRMLPlotViewNode> plotViewNode;
  scene->AddNode(plotViewNode.GetPointer());
  // Set PlotChart ID in PlotView
  plotViewNode->SetPlotChartNodeID(plotChartNode->GetID());

  //
  // Create a simple gui with non-tranposed and transposed table view
  //
  QWidget parentWidget;
  parentWidget.setWindowTitle("qMRMLPlotViewTest1");
  QVBoxLayout vbox;
  parentWidget.setLayout(&vbox);

  qMRMLPlotWidget* plotWidget = new qMRMLPlotWidget();
  plotWidget->setParent(&parentWidget);
  plotWidget->setMRMLScene(scene.GetPointer());
  plotWidget->setMRMLPlotViewNode(plotViewNode.GetPointer());
  vbox.addWidget(plotWidget);
  parentWidget.show();
  parentWidget.raise();

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
