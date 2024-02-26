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
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

/**
 * @class   vtkSlicerROIRepresentation2D
 * @brief   Default representation for the plane widget
 *
 * This class provides the default concrete representation for the
 * vtkMRMLAbstractWidget. See vtkMRMLAbstractWidget
 * for details.
 * @sa
 * vtkSlicerMarkupsWidgetRepresentation2D vtkMRMLAbstractWidget
 */

#ifndef vtkSlicerROIRepresentation2D_h
#define vtkSlicerROIRepresentation2D_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkSlicerMarkupsWidgetRepresentation2D.h"
#include "vtkSlicerROIRepresentation3D.h"

class vtkAppendPolyData;
class vtkClipPolyData;
class vtkContourTriangulator;
class vtkCutter;
class vtkDiscretizableColorTransferFunction;
class vtkMRMLMarkupsROINode;
class vtkOutlineFilter;
class vtkSampleImplicitFunctionFilter;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerROIRepresentation2D
  : public vtkSlicerMarkupsWidgetRepresentation2D
{
public:
  /// Instantiate this class.
  static vtkSlicerROIRepresentation2D* New();

  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkSlicerROIRepresentation2D, vtkSlicerMarkupsWidgetRepresentation2D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Subclasses of vtkContourCurveRepresentation must implement these methods. These
  /// are the methods that the widget and its representation use to
  /// communicate with each other.
  void UpdateFromMRMLInternal(vtkMRMLNode* caller, unsigned long event, void* callData = nullptr) override;

  /// Methods to make this class behave as a vtkProp.
  void GetActors(vtkPropCollection*) override;
  void ReleaseGraphicsResources(vtkWindow*) override;
  int RenderOverlay(vtkViewport* viewport) override;
  int RenderOpaqueGeometry(vtkViewport* viewport) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport* viewport) override;
  vtkTypeBool HasTranslucentPolygonalGeometry() override;

  /// Return the bounds of the representation
  double* GetBounds() VTK_SIZEHINT(6) override;

  void CanInteract(vtkMRMLInteractionEventData* interactionEventData,
                   int& foundComponentType,
                   int& foundComponentIndex,
                   double& closestDistance2) override;

  void CanInteractWithROI(vtkMRMLInteractionEventData* interactionEventData,
                          int& foundComponentType,
                          int& foundComponentIndex,
                          double& closestDistance2);

protected:
  /// Update the cube source filter from the ROI node.
  virtual void UpdateCubeSourceFromMRML(vtkMRMLMarkupsROINode* roiNode);

protected:
  vtkSlicerROIRepresentation2D();
  ~vtkSlicerROIRepresentation2D() override;

  void SetROISource(vtkPolyDataAlgorithm* roiSource);

  vtkSmartPointer<vtkPolyDataAlgorithm> ROISource;

  vtkSmartPointer<vtkPassThrough> ROIPipelineInputFilter;
  vtkSmartPointer<vtkTransform> ROIToWorldTransform;
  vtkSmartPointer<vtkTransformPolyDataFilter> ROIToWorldTransformFilter;
  vtkSmartPointer<vtkCutter> ROIOutlineCutter;
  vtkSmartPointer<vtkTransformPolyDataFilter> ROIOutlineWorldToSliceTransformFilter;
  vtkSmartPointer<vtkContourTriangulator> ROIIntersectionTriangulator;

  vtkSmartPointer<vtkPolyDataMapper2D> ROIMapper;
  vtkSmartPointer<vtkProperty2D> ROIProperty;
  vtkSmartPointer<vtkActor2D> ROIActor;

  vtkSmartPointer<vtkPolyDataMapper2D> ROIOutlineMapper;
  vtkSmartPointer<vtkProperty2D> ROIOutlineProperty;
  vtkSmartPointer<vtkActor2D> ROIOutlineActor;

private:
  vtkSlicerROIRepresentation2D(const vtkSlicerROIRepresentation2D&) = delete;
  void operator=(const vtkSlicerROIRepresentation2D&) = delete;
};

#endif
