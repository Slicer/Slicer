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
  vtkNew<vtkMRMLPlotDataNode> node;
  EXERCISE_ALL_BASIC_MRML_METHODS(node.GetPointer());
  scene->AddNode(node);

  vtkPlot* plot1 = node->GetPlot();
  CHECK_NOT_NULL(plot1);

  // Create a vtkTable
  vtkNew<vtkTable> table;

  vtkNew<vtkFloatArray> arrX;
  arrX->SetName("X Axis");
  table->AddColumn(arrX);

  vtkNew<vtkFloatArray> arrC;
  arrC->SetName("Cosine");
  table->AddColumn(arrC);

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
  vtkNew<vtkMRMLTableNode> TableNode;
  scene->AddNode(TableNode);
  TableNode->SetAndObserveTable(table.GetPointer());

  // Set and Observe the MRMLTableNode
  node->SetAndObserveTableNodeID(TableNode->GetID());

  node->SetType(vtkMRMLPlotDataNode::BAR);
  vtkPlot* plot2 = node->GetPlot();
  CHECK_NOT_NULL(plot2);

  CHECK_STD_STRING(node->GetYColumnName(), arrC->GetName())

  // Verify that Copy method creates a true independent copy
  vtkSmartPointer<vtkMRMLPlotDataNode> nodeCopy = vtkSmartPointer<vtkMRMLPlotDataNode>::New();
  nodeCopy->CopyWithScene(node.GetPointer());

  CHECK_STD_STRING(node->GetName(), nodeCopy->GetName());

  std::cout << "vtkMRMLPlotDataNodeTest1 completed successfully" << std::endl;
  return EXIT_SUCCESS;
}
