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

/**
 * @class   vtkMRMLRubberBandWidgetRepresentation
 * @brief   represent intersections of other slice views in the current slice view
 *
 * @sa
 * vtkSliceIntersectionWidget vtkWidgetRepresentation vtkAbstractWidget
*/

#ifndef vtkMRMLRubberBandWidgetRepresentation_h
#define vtkMRMLRubberBandWidgetRepresentation_h

#include "vtkMRMLDisplayableManagerExport.h" // For export macro
#include "vtkMRMLAbstractWidgetRepresentation.h"

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLRubberBandWidgetRepresentation : public vtkMRMLAbstractWidgetRepresentation
{
public:
  /**
   * Instantiate this class.
   */
  static vtkMRMLRubberBandWidgetRepresentation *New();

  //@{
  /**
   * Standard methods for instances of this class.
   */
  vtkTypeMacro(vtkMRMLRubberBandWidgetRepresentation, vtkMRMLAbstractWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  /// Top-left point position in display coordinates
  vtkGetVector2Macro(CornerPoint1, int);
  vtkSetVector2Macro(CornerPoint1, int);
  vtkGetVector2Macro(CornerPoint2, int);
  vtkSetVector2Macro(CornerPoint2, int);

  //@{
  /**
   * Methods to make this class behave as a vtkProp.
   */
  void GetActors2D(vtkPropCollection *) override;
  void ReleaseGraphicsResources(vtkWindow *) override;
  int RenderOverlay(vtkViewport *viewport) override;
  //@}

protected:
  vtkMRMLRubberBandWidgetRepresentation();
  ~vtkMRMLRubberBandWidgetRepresentation() override;

  class vtkInternal;
  vtkInternal * Internal;

  int CornerPoint1[2];
  int CornerPoint2[2];

private:
  vtkMRMLRubberBandWidgetRepresentation(const vtkMRMLRubberBandWidgetRepresentation&) = delete;
  void operator=(const vtkMRMLRubberBandWidgetRepresentation&) = delete;
};

#endif
