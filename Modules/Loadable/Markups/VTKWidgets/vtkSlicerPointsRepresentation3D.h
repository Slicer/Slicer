/*=========================================================================

 Copyright (c) ProxSim ltd., Kwun Tong, Hong Kong. All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
 and development was supported by ProxSim ltd.

=========================================================================*/

/**
 * @class   vtkSlicerPointsRepresentation3D
 * @brief   Default representation for the points widget
 *
 * This class provides the default concrete representation for the
 * vtkSlicerAbstractWidget. See vtkSlicerAbstractWidget
 * for details.
 * @sa
 * vtkSlicerAbstractWidgetRepresentation3D vtkSlicerAbstractWidget
*/

#ifndef vtkSlicerPointsRepresentation3D_h
#define vtkSlicerPointsRepresentation3D_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkSlicerAbstractWidgetRepresentation3D.h"

class vtkAppendPolyData;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerPointsRepresentation3D : public vtkSlicerAbstractWidgetRepresentation3D
{
public:
  /// Instantiate this class.
  static vtkSlicerPointsRepresentation3D *New();

  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkSlicerPointsRepresentation3D,vtkSlicerAbstractWidgetRepresentation3D);

protected:
  vtkSlicerPointsRepresentation3D();
  ~vtkSlicerPointsRepresentation3D() VTK_OVERRIDE;

private:
  vtkSlicerPointsRepresentation3D(const vtkSlicerPointsRepresentation3D&) = delete;
  void operator=(const vtkSlicerPointsRepresentation3D&) = delete;
};

#endif
