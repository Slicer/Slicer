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

#include "vtkMRMLPlotDataNode.h"
#include "vtkMRMLPlotChartNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTableNode.h"

#include "vtkFloatArray.h"
#include "vtkPlot.h"
#include "vtkTable.h"
#include "vtkTestErrorObserver.h"

#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLPlotChartNodeTest1(int , char * [] )
{
  // Create a PlotChart node
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLPlotChartNode> node;
  EXERCISE_ALL_BASIC_MRML_METHODS(node.GetPointer());
  scene->AddNode(node);

  // Create two PlotDataNodes
  vtkNew<vtkMRMLPlotDataNode> plotDataNode1;
  vtkNew<vtkMRMLPlotDataNode> plotDataNode2;
  scene->AddNode(plotDataNode1);
  scene->AddNode(plotDataNode2);

  // Create a vtkTable
  vtkNew<vtkTable> table;

  vtkNew<vtkFloatArray> arrX;
  arrX->SetName("X Axis");
  table->AddColumn(arrX);

  vtkNew<vtkFloatArray> arrC;
  arrC->SetName("Cosine");
  table->AddColumn(arrC);

  vtkNew<vtkFloatArray> arrS;
  arrS->SetName("Sine");
  table->AddColumn(arrS);

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
  vtkNew<vtkMRMLTableNode> TableNode;
  scene->AddNode(TableNode);
  TableNode->SetAndObserveTable(table.GetPointer());

  // Set and Observe the MRMLTableNode
  plotDataNode1->SetAndObserveTableNodeID(TableNode->GetID());
  plotDataNode2->SetAndObserveTableNodeID(TableNode->GetID());
  plotDataNode2->SetYColumnIndex(2);

  // Add and Observe plots IDs in PlotChart
  node->AddAndObservePlotDataNodeID(plotDataNode1->GetID());
  node->AddAndObservePlotDataNodeID(plotDataNode2->GetID());

  // Test The references
  CHECK_POINTER(node->GetPlotDataNode(), plotDataNode1);
  CHECK_POINTER(node->GetNthPlotDataNode(1), plotDataNode2);

  node->RemovePlotDataNodeID(plotDataNode1->GetID());
  CHECK_POINTER(node->GetPlotDataNode(), plotDataNode2);

  // Verify that Copy method creates a true independent copy
  vtkSmartPointer< vtkMRMLPlotChartNode > nodeCopy = vtkSmartPointer< vtkMRMLPlotChartNode >::New();
  nodeCopy->Copy(node.GetPointer());

  CHECK_STD_STRING(node->GetName(), nodeCopy->GetName());

  std::cout << "vtkMRMLPlotChartNodeTest1 completed successfully" << std::endl;
  return EXIT_SUCCESS;
}
