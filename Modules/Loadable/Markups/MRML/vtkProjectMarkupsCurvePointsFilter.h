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

class vtkDoubleArray;
class vtkOBBTree;
class vtkPointLocator;
class vtkPoints;
class vtkPolyData;

class vtkMRMLMarkupsCurveNode;
class vtkMRMLModelNode;

class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkProjectMarkupsCurvePointsFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkProjectMarkupsCurvePointsFilter, vtkPolyDataAlgorithm);
  static vtkProjectMarkupsCurvePointsFilter* New();

  /// Sets the input curve node. It does not take ownership of the node and will not extend its lifetime
  /// If the inputCurveNode is deleted during this object's lifetime it will as if nullptr was passed
  /// into this function.
  void SetInputCurveNode(vtkMRMLMarkupsCurveNode* inputCurveNode);

  static bool ConstrainPointsToSurface(vtkPoints* originalPoints, vtkDoubleArray* normalVectors, vtkPolyData* surfacePolydata,
    vtkPoints* surfacePoints, double maximumSearchRadiusTolerance);

  void SetMaximumSearchRadiusTolerance(double maximumSearchRadiusTolerance);
  double GetMaximumSearchRadiusTolerance() const;

protected:
  int FillInputPortInformation(int port, vtkInformation* info) override;
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;

  vtkProjectMarkupsCurvePointsFilter();
private:
  vtkWeakPointer<vtkMRMLMarkupsCurveNode> InputCurveNode;
  double MaximumSearchRadiusTolerance;

  bool ProjectPointsToSurface(vtkMRMLModelNode* modelNode, double maximumSearchRadiusTolerance, vtkPoints* interpolatedPoints, vtkPoints* outputPoints);
  static bool ConstrainPointsToSurfaceImpl(vtkOBBTree* surfaceObbTree, vtkPointLocator* pointLocator,
      vtkPoints* originalPoints, vtkDoubleArray* normalVectors, vtkPolyData* surfacePolydata,
      vtkPoints* surfacePoints, double maximumSearchRadius=.25);

  class PointProjectionHelper
  {
  public:
    PointProjectionHelper();
    void SetModel(vtkMRMLModelNode* model);
    /// Gets the point normals on the model at the points with the given controlPoints.
    /// Both points and control points must have no outstanding transformations.
    vtkSmartPointer<vtkDoubleArray> GetPointNormals(vtkPoints* points, vtkPoints* controlPoints);
    vtkPointLocator* GetPointLocator();
    vtkOBBTree* GetObbTree();
    vtkPolyData* GetSurfacePolyData();

  private:
    vtkMRMLModelNode* Model;
    vtkMTimeType LastModelModifiedTime;
    vtkMTimeType LastTransformModifiedTime;
    vtkSmartPointer<vtkDataArray> ModelNormalVectorArray;
    vtkSmartPointer<vtkPointLocator> ModelPointLocator;
    vtkSmartPointer<vtkOBBTree> ModelObbTree;
    vtkSmartPointer<vtkPolyData> SurfacePolyData;

    bool UpdateAll();
    static vtkIdType GetClosestControlPointIndex(const double point[3], vtkPoints* controlPoints);
  };

  PointProjectionHelper PointProjection;
};

#endif
