/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women’s Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __vtkSlicerDijkstraGraphGeodesicPath_h
#define __vtkSlicerDijkstraGraphGeodesicPath_h

// VTK includes
#include <vtkDijkstraGraphGeodesicPath.h>

// export
#include "vtkSlicerMarkupsModuleMRMLExport.h"

/// Filter that generates curves between points of an input polydata
class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkSlicerDijkstraGraphGeodesicPath : public vtkDijkstraGraphGeodesicPath
{
public:
  vtkTypeMacro(vtkSlicerDijkstraGraphGeodesicPath, vtkDijkstraGraphGeodesicPath);
  static vtkSlicerDijkstraGraphGeodesicPath* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Cost function type is the method that is used to calculate the cost of each edge using the distance and scalar.
  /// Ex.
  ///     COST_FUNCTION_TYPE_DISTANCE        = distance
  ///     COST_FUNCTION_TYPE_ADDITIVE        = distance + scalar
  ///     COST_FUNCTION_TYPE_MULTIPLICATIVE  = distance * scalar
  ///     COST_FUNCTION_TYPE_INVERSE_SQUARED = distance / scalar^2
  enum
    {
    COST_FUNCTION_TYPE_DISTANCE,
    COST_FUNCTION_TYPE_ADDITIVE,
    COST_FUNCTION_TYPE_MULTIPLICATIVE,
    COST_FUNCTION_TYPE_INVERSE_SQUARED,
    COST_FUNCTION_TYPE_LAST,
    };
  static const char* GetCostFunctionTypeAsString(int costFunctionType);
  static int GetCostFunctionTypeFromString(const char* costFunctionType);
  vtkSetMacro(CostFunctionType, int);
  vtkGetMacro(CostFunctionType, int);

protected:
  /// Reimplemented to rebuild the adjacency info if either CostFunctionType or UseScalarWeights are changed.
  int RequestData(vtkInformation*, vtkInformationVector**,
    vtkInformationVector*) override;

  /// The fixed cost going from vertex u to v.
  /// Reimplemented to provide additonal cost function types.
  /// \sa SetCostFunctionType()
  double CalculateStaticEdgeCost(vtkDataSet* inData, vtkIdType u, vtkIdType v) override;

  int CostFunctionType;
  int PreviousCostFunctionType;
  bool PreviousUseScalarWeights;

protected:
  vtkSlicerDijkstraGraphGeodesicPath();
  ~vtkSlicerDijkstraGraphGeodesicPath() override;
  vtkSlicerDijkstraGraphGeodesicPath(const vtkSlicerDijkstraGraphGeodesicPath&) = delete;
  void operator=(const vtkSlicerDijkstraGraphGeodesicPath&) = delete;
};

#endif
