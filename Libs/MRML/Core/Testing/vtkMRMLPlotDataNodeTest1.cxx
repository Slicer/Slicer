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
#include "vtkMRMLScene.h"
#include "vtkMRMLTableNode.h"

#include "vtkFloatArray.h"
#include "vtkPlot.h"
#include "vtkTable.h"
#include "vtkTestErrorObserver.h"

#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLPlotDataNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLPlotDataNode> plotDataNode;
  EXERCISE_ALL_BASIC_MRML_METHODS(plotDataNode.GetPointer());
  scene->AddNode(plotDataNode.GetPointer());

  vtkPlot* plot1 = plotDataNode->GetPlot();
  CHECK_NOT_NULL(plot1);

  // Create a vtkTable
  vtkNew<vtkTable> table;

  vtkNew<vtkFloatArray> arrX;
  arrX->SetName("X Axis");
  table->AddColumn(arrX.GetPointer());

  vtkNew<vtkFloatArray> arrC;
  arrC->SetName("Cosine");
  table->AddColumn(arrC.GetPointer());

  // Fill in the table with some example values
  int numPoints = 69;
  float inc = 7.5 / (numPoints-1);
  table->SetNumberOfRows(numPoints);
  for (int i = 0; i < numPoints; ++i)
    {
    table->SetValue(i, 0, i * inc);
    table->SetValue(i, 1, cos(i * inc));
    }

  // Create a MRMLTableNode
  vtkNew<vtkMRMLTableNode> tableNode;
  scene->AddNode(tableNode.GetPointer());
  tableNode->SetAndObserveTable(table.GetPointer());

  // Set and Observe the MRMLTableNode
  plotDataNode->SetAndObserveTableNodeID(tableNode->GetID());

  plotDataNode->SetType(vtkMRMLPlotDataNode::BAR);
  vtkPlot* plot2 = plotDataNode->GetPlot();
  CHECK_NOT_NULL(plot2);

  plotDataNode->SetXColumnName(arrX->GetName());
  plotDataNode->SetYColumnName(arrC->GetName());


  // Verify that Copy method creates a true independent copy
  vtkSmartPointer<vtkMRMLPlotDataNode> nodeCopy = vtkSmartPointer<vtkMRMLPlotDataNode>::New();
  nodeCopy->CopyWithScene(plotDataNode.GetPointer());

  CHECK_STD_STRING(plotDataNode->GetName(), nodeCopy->GetName());
  CHECK_STD_STRING(plotDataNode->GetXColumnName(), arrX->GetName());
  CHECK_STD_STRING(plotDataNode->GetYColumnName(), arrC->GetName());

  std::cout << "vtkMRMLPlotDataNodeTest1 completed successfully" << std::endl;
  return EXIT_SUCCESS;
}
