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
#include <QVBoxLayout>

// qMRML includes
#include "qMRMLTableView.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerTableColumnPropertiesWidget.h"

// MRML includes
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTableNode.h"

// Slicer includes
#include "vtkSlicerConfigure.h"

// VTK includes
#include <vtkNew.h>
#include <vtkTable.h>
#include "qMRMLWidget.h"

int qSlicerTableColumnPropertiesWidgetTest1(int argc, char* argv[])
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  // Create a table with some points in it...
  vtkNew<vtkTable> table;
  vtkNew<vtkFloatArray> arrayX;
  arrayX->SetName("X Axis");
  table->AddColumn(arrayX.GetPointer());
  vtkNew<vtkDoubleArray> arrayY;
  arrayY->SetName("Y Axis");
  table->AddColumn(arrayY.GetPointer());
  vtkNew<vtkDoubleArray> arrSum;
  arrSum->SetName("Sum");
  table->AddColumn(arrSum.GetPointer());
  int numPoints = 15;
  table->SetNumberOfRows(numPoints);
  for (int i = 0; i < numPoints; ++i)
  {
    table->SetValue(i, 0, i * 0.5 - 10);
    table->SetValue(i, 1, i * 1.2 + 12);
    table->SetValue(i, 2, table->GetValue(i, 0).ToDouble() + table->GetValue(i, 1).ToDouble());
  }

  vtkNew<vtkMRMLTableNode> tableNode;
  tableNode->SetAndObserveTable(table.GetPointer());

  tableNode->SetColumnNullValue("xaxis", "-5000");
  tableNode->SetColumnDescription("xaxis", "this is the x column");
  tableNode->SetColumnTitle("xaxis", "X axis");
  tableNode->SetColumnUnitLabel("xaxis", "HU");
  tableNode->SetColumnProperty("xaxis", "unitCodeValue", "[hnsf'U]");
  tableNode->SetColumnProperty("xaxis", "unitCodingSchemeDesignator", "UCUM");
  tableNode->SetColumnProperty("xaxis", "unitCodeMeaning", "Hounsfield unit");

  tableNode->SetColumnNullValue("yaxis", "0.0");
  tableNode->SetColumnDescription("yaxis", "this is the y column");
  tableNode->SetColumnTitle("yaxis", "Y column");
  tableNode->SetColumnUnitLabel("yaxis", "mm");
  tableNode->SetColumnProperty("yaxis", "unitCodeValue", "1");
  tableNode->SetColumnProperty("yaxis", "unitCodingSchemeDesignator", "UCUM");
  tableNode->SetColumnProperty("yaxis", "unitCodeMeaning", "no units");

  //
  // Create a simple gui with non-tranposed and transposed table view
  //
  QWidget parentWidget;
  parentWidget.setWindowTitle("qMRMLTableViewTest1");
  QVBoxLayout vbox;
  parentWidget.setLayout(&vbox);

  qMRMLTableView* tableView = new qMRMLTableView();
  tableView->setParent(&parentWidget);
  tableView->setMRMLTableNode(tableNode.GetPointer());
  vbox.addWidget(tableView);

  qSlicerTableColumnPropertiesWidget* columnPropertiesWidget = new qSlicerTableColumnPropertiesWidget();
  columnPropertiesWidget->setParent(&parentWidget);
  columnPropertiesWidget->setSelectionFromMRMLTableView(tableView);
  vbox.addWidget(columnPropertiesWidget);

  parentWidget.show();
  parentWidget.raise();

  if (argc < 2 || QString(argv[1]) != "-I")
  {
    QTimer::singleShot(200, &app, SLOT(quit()));
  }

  return app.exec();
}
