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

#ifndef __vtkProjectMarkupsCurvePointsFilter_h
#define __vtkProjectMarkupsCurvePointsFilter_h

#include "vtkSlicerMarkupsModuleMRMLExport.h"

#include <vtkPolyDataAlgorithm.h>
#include <vtkInformation.h>
#include <vtkWeakPointer.h>

class vtkMRMLMarkupsCurveNode;

class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkProjectMarkupsCurvePointsFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkProjectMarkupsCurvePointsFilter, vtkPolyDataAlgorithm);
  static vtkProjectMarkupsCurvePointsFilter* New();

  /// Sets the input curve node. It does not take ownership of the node and will not extend its lifetime
  /// If the inputCurveNode is deleted during this object's lifetime it will as if nullptr was passed
  /// into this function.
  void SetInputCurveNode(vtkMRMLMarkupsCurveNode* inputCurveNode);

protected:
  int FillInputPortInformation(int port, vtkInformation* info) override;
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;
private:
  vtkWeakPointer<vtkMRMLMarkupsCurveNode> InputCurveNode;
};

#endif
