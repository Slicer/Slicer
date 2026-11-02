/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// Qt includes
#include <QApplication>

// CTK includes
#include <ctkVTKDataSetArrayComboBox.h>

// qMRML includes
#include "qMRMLScalarsDisplayWidget.h"
#include "qMRMLWidget.h"

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkDoubleArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

namespace
{

//----------------------------------------------------------------------------
void AddPointArray(vtkPolyData* mesh, const char* name)
{
  vtkNew<vtkDoubleArray> values;
  values->SetName(name);
  values->SetNumberOfTuples(mesh->GetNumberOfPoints());
  values->Fill(1.0);
  mesh->GetPointData()->AddArray(values);
}

//----------------------------------------------------------------------------
std::string ActiveScalarName(vtkMRMLDisplayNode* displayNode)
{
  return (displayNode->GetActiveScalarName() ? displayNode->GetActiveScalarName() : "");
}

} // namespace

//----------------------------------------------------------------------------
/// The "(Default)" entry (the array the host chose) heads the array list. When the mesh is modified the list is rebuilt
/// from it without that entry, and choosing an array from the rebuilt list chooses that array.
int qMRMLScalarsDisplayWidgetTest1(int argc, char* argv[])
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkPolyData> mesh;
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(0.0, 0.0, 0.0);
  points->InsertNextPoint(1.0, 0.0, 0.0);
  points->InsertNextPoint(0.0, 1.0, 0.0);
  mesh->SetPoints(points);
  AddPointArray(mesh, "Alpha");
  AddPointArray(mesh, "Beta");
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  modelNode->SetAndObserveMesh(mesh);
  modelNode->CreateDefaultDisplayNodes();
  vtkMRMLModelDisplayNode* displayNode = modelNode->GetModelDisplayNode();
  CHECK_NOT_NULL(displayNode);

  qMRMLScalarsDisplayWidget widget;
  widget.setMRMLScene(scene);
  widget.setDefaultScalarArrayName("Beta");
  widget.setMRMLDisplayNode(displayNode);
  ctkVTKDataSetArrayComboBox* arrayComboBox = widget.findChild<ctkVTKDataSetArrayComboBox*>("ActiveScalarComboBox");
  CHECK_NOT_NULL(arrayComboBox);
  CHECK_STD_STRING(arrayComboBox->itemText(0).toStdString(), "(Default)");
  CHECK_BOOL(arrayComboBox->findText("Alpha") > 0, true);

  // Choosing the entry chooses the default array
  arrayComboBox->setCurrentIndex(0);
  QMetaObject::invokeMethod(&widget, "onCurrentArrayActivated");
  CHECK_STD_STRING(ActiveScalarName(displayNode), "Beta");

  // The mesh gets another array: the list is rebuilt. Choosing an array by name chooses that
  // array, wherever the entry for the default array is
  AddPointArray(mesh, "Gamma");
  mesh->Modified();
  int alphaIndex = arrayComboBox->findText("Alpha");
  std::cout << "Alpha is at row " << alphaIndex << " after the mesh was modified" << std::endl;
  CHECK_BOOL(alphaIndex >= 0, true);
  arrayComboBox->setCurrentIndex(alphaIndex);
  QMetaObject::invokeMethod(&widget, "onCurrentArrayActivated");
  CHECK_STD_STRING(ActiveScalarName(displayNode), "Alpha");

  // Updated from the node again, the entry is back at the top
  displayNode->Modified();
  CHECK_STD_STRING(arrayComboBox->itemText(0).toStdString(), "(Default)");
  arrayComboBox->setCurrentIndex(0);
  QMetaObject::invokeMethod(&widget, "onCurrentArrayActivated");
  CHECK_STD_STRING(ActiveScalarName(displayNode), "Beta");

  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
