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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QTimer>
#include <QGridLayout>

// qMRML includes
#include "qMRMLPlotView.h"
#include "qSlicerCoreApplication.h"
#include "qMRMLPlotChartPropertiesWidget.h"
#include "qMRMLPlotSeriesPropertiesWidget.h"

// MRML includes
#include "vtkFloatArray.h"
#include "vtkMRMLPlotChartNode.h"
#include "vtkMRMLPlotSeriesNode.h"
#include "vtkMRMLPlotViewNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLTableNode.h"

// MRMLLogic includes
#include <vtkMRMLColorLogic.h>

// Slicer includes
#include "vtkSlicerConfigure.h"

// VTK includes
#include <vtkNew.h>
#include <vtkTable.h>
#include "qMRMLWidget.h"

int qMRMLPlotPropertiesWidgetTest1( int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLColorLogic> colorLogic;
  colorLogic->SetMRMLScene(scene.GetPointer());

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
  float inc = 7.5 / (numPoints - 1);
  table->SetNumberOfRows(numPoints);
  for (int i = 0; i < numPoints; ++i)
  {
    table->SetValue(i, 0, i * inc);
    table->SetValue(i, 1, cos(i * inc));
    table->SetValue(i, 2, sin(i * inc));
  }

  // Create a MRMLTableNode
  vtkMRMLTableNode* tableNode = vtkMRMLTableNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTableNode"));
  tableNode->SetAndObserveTable(table.GetPointer());

  // Create two plotSeriesNodes

  vtkMRMLPlotSeriesNode* plotSeriesNode1 = vtkMRMLPlotSeriesNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLPlotSeriesNode", "My cosine"));
  plotSeriesNode1->SetAndObserveTableNodeID(tableNode->GetID());
  plotSeriesNode1->SetXColumnName(tableNode->GetColumnName(0));
  plotSeriesNode1->SetYColumnName(tableNode->GetColumnName(1));

  vtkMRMLPlotSeriesNode* plotSeriesNode2 = vtkMRMLPlotSeriesNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLPlotSeriesNode", "My sine"));
  plotSeriesNode2->SetAndObserveTableNodeID(tableNode->GetID());
  plotSeriesNode2->SetXColumnName(tableNode->GetColumnName(0));
  plotSeriesNode2->SetYColumnName(tableNode->GetColumnName(2));
  plotSeriesNode2->SetUniqueColor();
  plotSeriesNode2->SetLineStyle(vtkMRMLPlotSeriesNode::LineStyleSolid);
  plotSeriesNode2->SetMarkerStyle(vtkMRMLPlotSeriesNode::MarkerStyleNone);

  // Create a PlotChart node
  vtkNew<vtkMRMLPlotChartNode> plotChartNode;
  scene->AddNode(plotChartNode.GetPointer());
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
  // Create a simple GUI
  //
  QWidget parentWidget;
  parentWidget.setWindowTitle("qMRMLPlotSeriesPropertiesWidgetTest1");
  QGridLayout gridLayout;
  parentWidget.setLayout(&gridLayout);

  qMRMLPlotView* plotView = new qMRMLPlotView();
  plotView->setMinimumSize(100, 100);
  plotView->setMRMLScene(scene.GetPointer());
  plotView->setMRMLPlotViewNode(plotViewNode.GetPointer());
  gridLayout.addWidget(plotView, 0, 0, 1, 2);

  qMRMLPlotChartPropertiesWidget* chartPropertiesWidget = new qMRMLPlotChartPropertiesWidget();
  chartPropertiesWidget->setMRMLScene(scene.GetPointer());
  chartPropertiesWidget->setMRMLPlotChartNode(plotChartNode.GetPointer());
  gridLayout.addWidget(chartPropertiesWidget, 1, 0);

  qMRMLPlotSeriesPropertiesWidget* seriesPropertiesWidget = new qMRMLPlotSeriesPropertiesWidget();
  seriesPropertiesWidget->setMRMLScene(scene.GetPointer());
  seriesPropertiesWidget->setMRMLPlotSeriesNode(plotSeriesNode1);
  gridLayout.addWidget(seriesPropertiesWidget, 1, 1);

  parentWidget.show();
  parentWidget.raise();

  if (argc < 2 || QString(argv[1]) != "-I")
  {
    QTimer::singleShot(200, &app, SLOT(quit()));
  }

  return app.exec();
}
