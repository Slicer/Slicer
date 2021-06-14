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

  This file was centerally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/
/**
 * @class   vtkFastSelectVisiblePoints
 * @brief   extract points that are visible (based on z-buffer calculation)
 *
 * vtkFastSelectVisiblePoints is a filter that selects points based on
 * whether they are visible or not. Visibility is determined by
 * accessing the z-buffer of a rendering window. (The position of each
 * input point is converted into display coordinates, and then the
 * z-value at that point is obtained. If within the user-specified
 * tolerance, the point is considered visible.)
 *
 * Points that are visible (or if the ivar SelectInvisible is on,
 * invisible points) are passed to the output. Associated data
 * attributes are passed to the output as well.
 *
 * This filter also allows you to specify a rectangular window in display
 * (pixel) coordinates in which the visible points must lie. This can be
 * used as a sort of local "brushing" operation to select just data within
 * a window.
 *
 *
 * @warning
 * You must carefully synchronize the execution of this filter. The
 * filter refers to a renderer, which is modified every time a render
 * occurs. Therefore, the filter is always out of date, and always
 * executes. You may have to perform two rendering passes, or if you
 * are using this filter in conjunction with vtkLabeledDataMapper,
 * things work out because 2D rendering occurs after the 3D rendering.
 */

#ifndef vtkFastSelectVisiblePoints_h
#define vtkFastSelectVisiblePoints_h

#include "vtkSelectVisiblePoints.h"
#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"

#include <vtkFloatArray.h>

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkFastSelectVisiblePoints : public vtkSelectVisiblePoints
{
public:
  vtkTypeMacro(vtkFastSelectVisiblePoints, vtkSelectVisiblePoints);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Instantiate object with no renderer; window selection turned off;
   * tolerance set to 0.01; and select invisible off.
   */
  static vtkFastSelectVisiblePoints* New();

  void UpdateZBuffer();
  void ResetZBuffer();

  vtkFloatArray* GetZBuffer() { return this->ZBuffer; };
  void SetZBuffer(vtkFloatArray* zBuffer) { this->ZBuffer = zBuffer; };

protected:
  vtkFastSelectVisiblePoints();
  ~vtkFastSelectVisiblePoints() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  vtkSmartPointer<vtkFloatArray> ZBuffer;

private:
  vtkFastSelectVisiblePoints(const vtkFastSelectVisiblePoints&) = delete;
  void operator=(const vtkFastSelectVisiblePoints&) = delete;
};

#endif
