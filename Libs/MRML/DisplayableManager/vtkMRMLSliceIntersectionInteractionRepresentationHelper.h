/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Ebatinca S.L., Las Palmas de Gran Canaria, Spain

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

/**
 * @class   vtkMRMLSliceIntersectionInteractionRepresentationHelper
 * @brief   utility class to compute slice intersection interactions
 */

#ifndef vtkMRMLSliceIntersectionInteractionRepresentationHelper_h
#define vtkMRMLSliceIntersectionInteractionRepresentationHelper_h

#include "vtkMRMLDisplayableManagerExport.h" // For export macro

// VTK includes
#include <vtkObject.h>
class vtkPoints;
class vtkMatrix4x4;
class vtkMRMLSliceNode;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLSliceIntersectionInteractionRepresentationHelper : public vtkObject
{
public:
  /**
   * Instantiate this class.
   */
  static vtkMRMLSliceIntersectionInteractionRepresentationHelper* New();

  //@{
  /**
   * Standard methods for instances of this class.
   */
  vtkTypeMacro(vtkMRMLSliceIntersectionInteractionRepresentationHelper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  int IntersectWithFinitePlane(double n[3],
                               double o[3],
                               double pOrigin[3],
                               double px[3],
                               double py[3],
                               double x0[3],
                               double x1[3]);

  /// Compute intersection between a 2D line and the slice view boundaries
  void GetIntersectionWithSliceViewBoundaries(double* pointA,
                                              double* pointB,
                                              double* sliceViewBounds,
                                              double* intersectionPoint);

  /// Get boundaries of the slice view associated with a given vtkMRMLSliceNode
  void GetSliceViewBoundariesXY(vtkMRMLSliceNode* sliceNode, double* sliceViewBounds);

  int GetLineTipsFromIntersectingSliceNode(vtkMRMLSliceNode* intersectingSliceNode,
                                           vtkMatrix4x4* intersectingXYToXY,
                                           double intersectionLineTip1[3],
                                           double intersectionLineTip2[3]);

  void ComputeHandleToWorldTransformMatrix(double handlePosition[2],
                                           double handleOrientation[2],
                                           vtkMatrix4x4* handleToWorldTransformMatrix);
  void RotationMatrixFromVectors(double vector1[2], double vector2[2], vtkMatrix4x4* rotationMatrixHom);

protected:
  vtkMRMLSliceIntersectionInteractionRepresentationHelper();
  ~vtkMRMLSliceIntersectionInteractionRepresentationHelper() override;

private:
  vtkMRMLSliceIntersectionInteractionRepresentationHelper(
    const vtkMRMLSliceIntersectionInteractionRepresentationHelper&) = delete;
  void operator=(const vtkMRMLSliceIntersectionInteractionRepresentationHelper&) = delete;
};

#endif
