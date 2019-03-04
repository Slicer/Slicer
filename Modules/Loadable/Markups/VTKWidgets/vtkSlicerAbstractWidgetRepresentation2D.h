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
 * @class   vtkSlicerAbstractWidgetRepresentation2D
 * @brief   Default representation for the slicer markups widget
 *
 * This class provides the default concrete representation for the
 * vtkSlicerAbstractWidget. See vtkSlicerAbstractWidget
 * for details.
 * @sa
 * vtkSlicerAbstractWidgetRepresentation2D vtkSlicerAbstractWidget
*/

#ifndef vtkSlicerAbstractWidgetRepresentation2D_h
#define vtkSlicerAbstractWidgetRepresentation2D_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkSlicerAbstractWidgetRepresentation.h"

#include "vtkMRMLSliceNode.h"

class vtkActor2D;
class vtkGlyph2D;
class vtkLabelPlacementMapper;
class vtkMarkupsGlyphSource2D;
class vtkOpenGLPolyDataMapper2D;
class vtkProperty2D;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerAbstractWidgetRepresentation2D : public vtkSlicerAbstractWidgetRepresentation
{
public:
  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkSlicerAbstractWidgetRepresentation2D,vtkSlicerAbstractWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  /// Position is displayed (slice) position
  void CanInteract(const int displayPosition[2], const double worldPosition[3],
    int &foundComponentType, int &foundComponentIndex, double &closestDistance2) VTK_OVERRIDE;

  /// Checks if interaction with straight line between visible points is possible.
  /// Can be used on the output of CanInteract, as if no better component is found then the input is returned.
  void CanInteractWithLine(const int displayPosition[2], const double worldPosition[3],
    int &foundComponentType, int &foundComponentIndex, double &closestDistance2);

  /// Subclasses of vtkSlicerAbstractWidgetRepresentation2D must implement these methods. These
  /// are the methods that the widget and its representation use to
  /// communicate with each other.
  virtual void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData=NULL);

  /// Methods to make this class behave as a vtkProp.
  void GetActors(vtkPropCollection *) VTK_OVERRIDE;
  void ReleaseGraphicsResources(vtkWindow *) VTK_OVERRIDE;
  int RenderOverlay(vtkViewport *viewport) VTK_OVERRIDE;
  int RenderOpaqueGeometry(vtkViewport *viewport) VTK_OVERRIDE;
  int RenderTranslucentPolygonalGeometry(vtkViewport *viewport) VTK_OVERRIDE;
  vtkTypeBool HasTranslucentPolygonalGeometry() VTK_OVERRIDE;

  /// Get the nth node's position on the slice. Will return
  /// 1 on success, or 0 if there are not at least
  /// (n+1) nodes (0 based counting).
  int GetNthNodeDisplayPosition(int n, double pos[2]) VTK_OVERRIDE;

  /// Set the Nth node slice visibility (i.e. if it is on the slice).
  virtual void SetNthPointSliceVisibility(int n, bool visibility);

  /// Set the center slice visibility (i.e. if it is on the slice).
  virtual void SetCenterSliceVisibility(bool visibility);

  void GetSliceToWorldCoordinates(const double[2], double[3]);
  void GetWorldToSliceCoordinates(const double worldPos[3], double slicePos[2]);

  /// Set/Get the 2d scale factor to divide 3D scale by to show 2D elements appropriately (usually set to 300)
  vtkSetMacro(ScaleFactor2D, double);
  vtkGetMacro(ScaleFactor2D, double);

protected:
  vtkSlicerAbstractWidgetRepresentation2D();
  ~vtkSlicerAbstractWidgetRepresentation2D() VTK_OVERRIDE;

    /// Get MRML view node as slice view node
  vtkMRMLSliceNode *GetSliceNode();

  bool GetAllControlPointsVisible() VTK_OVERRIDE;

  /// Check, if the point is displayable in the current slice geometry
  virtual bool IsPointDisplayableOnSlice(vtkMRMLMarkupsNode* node, int pointIndex = 0);

  /// Check, if the point is displayable in the current slice geometry
  virtual bool IsCenterDisplayableOnSlice(vtkMRMLMarkupsNode* node);

  /// Convert display to world coordinates
  void GetWorldToDisplayCoordinates(double r, double a, double s, double * displayCoordinates);
  void GetWorldToDisplayCoordinates(double * worldCoordinates, double * displayCoordinates);

  class ControlPointsPipeline2D : public ControlPointsPipeline
  {
  public:
    ControlPointsPipeline2D();
    virtual ~ControlPointsPipeline2D();

    vtkSmartPointer<vtkActor2D> Actor;
    vtkSmartPointer<vtkOpenGLPolyDataMapper2D> Mapper;
    vtkSmartPointer<vtkGlyph2D> Glypher;
    vtkSmartPointer<vtkActor2D> LabelsActor;
    vtkSmartPointer<vtkLabelPlacementMapper> LabelsMapper;
    // Properties used to control the appearance of selected objects and
    // the manipulator in general.
    vtkSmartPointer<vtkProperty2D> Property;
  };

  ControlPointsPipeline2D* GetControlPointsPipeline(int controlPointType);

  vtkSmartPointer<vtkIntArray> PointsVisibilityOnSlice;
  bool                        CenterVisibilityOnSlice;

  /// Scale factor for 2d windows
  double ScaleFactor2D;

  virtual void UpdateAllPointsAndLabelsFromMRML(double labelsOffset);

private:
  vtkSlicerAbstractWidgetRepresentation2D(const vtkSlicerAbstractWidgetRepresentation2D&) = delete;
  void operator=(const vtkSlicerAbstractWidgetRepresentation2D&) = delete;
};

#endif
