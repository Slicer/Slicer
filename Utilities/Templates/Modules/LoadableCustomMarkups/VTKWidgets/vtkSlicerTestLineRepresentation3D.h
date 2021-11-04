/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

==============================================================================*/

#ifndef __vtkslicertestlinerepresentation3d_h_
#define __vtkslicertestlinerepresentation3d_h_

#include "vtkSlicerTemplateKeyModuleVTKWidgetsExport.h"

// Markups VTKWidgets includes
#include "vtkSlicerLineRepresentation3D.h"

// VTK includes
#include <vtkWeakPointer.h>

//------------------------------------------------------------------------------
class vtkCutter;
class vtkPlane;

/**
 * @class   vtkSlicerTestLineRepresentation3D
 * @brief   Default representation for the line widget
 *
 * This class provides the default concrete representation for the
 * vtkMRMLAbstractWidget. See vtkMRMLAbstractWidget
 * for details.
 * @sa
 * vtkSlicerMarkupsWidgetRepresentation2D vtkMRMLAbstractWidget
*/

class VTK_SLICER_TEMPLATEKEY_MODULE_VTKWIDGETS_EXPORT vtkSlicerTestLineRepresentation3D
: public vtkSlicerLineRepresentation3D
{
public:
  static vtkSlicerTestLineRepresentation3D* New();
  vtkTypeMacro(vtkSlicerTestLineRepresentation3D, vtkSlicerLineRepresentation3D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void* callData=nullptr) override;

  /// Methods to make this class behave as a vtkProp.
  void GetActors(vtkPropCollection*) override;
  void ReleaseGraphicsResources(vtkWindow*) override;
  int RenderOverlay(vtkViewport* viewport) override;
  int RenderOpaqueGeometry(vtkViewport* viewport) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport* viewport) override;
  vtkTypeBool HasTranslucentPolygonalGeometry() override;

protected:
  vtkSlicerTestLineRepresentation3D();
  ~vtkSlicerTestLineRepresentation3D() override;

  vtkSmartPointer<vtkCutter> Cutter;
  vtkSmartPointer<vtkPolyDataMapper> ContourMapper;
  vtkSmartPointer<vtkActor> ContourActor;
  vtkSmartPointer<vtkPolyData> MiddlePoint;
  vtkSmartPointer<vtkPolyDataMapper> MiddlePointMapper;
  vtkSmartPointer<vtkActor> MiddlePointActor;
  vtkWeakPointer<vtkPolyData> TargetOrgan;
  vtkSmartPointer<vtkSphereSource> MiddlePointSource;
  vtkSmartPointer<vtkPlane> SlicingPlane;

  void BuildMiddlePoint();
  void BuildSlicingPlane();

private:
  vtkSlicerTestLineRepresentation3D(const vtkSlicerTestLineRepresentation3D&) = delete;
  void operator=(const vtkSlicerTestLineRepresentation3D&) = delete;
};

#endif // __vtkslicertestlinerepresentation3d_h_
