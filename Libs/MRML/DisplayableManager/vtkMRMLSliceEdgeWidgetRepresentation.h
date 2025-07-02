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

  This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
  and development was supported in part by CI3.

==============================================================================*/

#ifndef vtkMRMLSliceEdgeWidgetRepresentation_h
#define vtkMRMLSliceEdgeWidgetRepresentation_h

// MRMLDM includes
#include "vtkMRMLDisplayableManagerExport.h"
#include "vtkMRMLAbstractWidgetRepresentation.h"

class vtkMRMLInteractionEventData;
class vtkMRMLModelNode;
class vtkMRMLSliceNode;

class vtkActor;
class vtkPolyDataMapper;
class vtkFeatureEdges;
class vtkPolyData;
class vtkStripper;
class vtkTubeFilter;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLSliceEdgeWidgetRepresentation
  : public vtkMRMLAbstractWidgetRepresentation
{
public:
  /// Instantiate this class.
  static vtkMRMLSliceEdgeWidgetRepresentation* New();

  ///@{
  /// Standard VTK class macros.
  vtkTypeMacro(vtkMRMLSliceEdgeWidgetRepresentation, vtkMRMLAbstractWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  ///@}

  /// Update the representation from display node
  void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void* callData = nullptr) override;

  /// Methods to make this class behave as a vtkProp.
  void GetActors(vtkPropCollection*) override;
  void ReleaseGraphicsResources(vtkWindow*) override;
  int RenderOverlay(vtkViewport* viewport) override;
  int RenderOpaqueGeometry(vtkViewport* viewport) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport* viewport) override;
  vtkTypeBool HasTranslucentPolygonalGeometry() override;

  /// Returns the actor for the interaction widget.
  vtkProp* GetSliceEdgeActor();

  virtual void setSliceNode(vtkMRMLSliceNode* sliceNode);
  virtual vtkMRMLSliceNode* GetSliceNode();

  virtual void setSliceModelNode(vtkMRMLModelNode* sliceNode);
  virtual vtkMRMLModelNode* GetSliceModelNode();

  virtual void UpdateSliceEdgeFromSliceNode();
  virtual void UpdateViewScaleFactor();
  virtual void UpdateSliceEdgePipeline();

  /// Get size of slice edge in world coordinate system
  vtkGetMacro(SliceEdgeSize, double);

  virtual vtkPolyData* GetSliceEdgePolydata();
  virtual void GetSliceEdgeColor(double color[4]);

  vtkGetMacro(Interacting, bool);

protected:
  vtkMRMLSliceEdgeWidgetRepresentation();
  ~vtkMRMLSliceEdgeWidgetRepresentation() override;

  class SliceEdgePipeline
  {
  public:
    SliceEdgePipeline();
    ~SliceEdgePipeline() = default;

    vtkSmartPointer<vtkActor> Actor;
    vtkSmartPointer<vtkPolyDataMapper> Mapper;
    vtkSmartPointer<vtkFeatureEdges> FeatureEdges;
    vtkSmartPointer<vtkStripper> Stripper;
    vtkSmartPointer<vtkTubeFilter> TubeFilter;
  };

  double ViewScaleFactorMmPerPixel;
  double ScreenSizePixel; // diagonal size of the screen

  /// Handle size, specified in renderer world coordinate system.
  /// For 3D views, renderer world coordinate system is the Slicer world coordinate system, so it is measured in the
  /// scene length unit (typically millimeters).
  double SliceEdgeSize{ 2.0 };
  bool Interacting{ false };

  virtual void SetupSliceEdgePipeline();
  SliceEdgePipeline* Pipeline;
  vtkWeakPointer<vtkMRMLSliceNode> SliceNode;
  vtkWeakPointer<vtkMRMLModelNode> SliceModelNode;

private:
  vtkMRMLSliceEdgeWidgetRepresentation(const vtkMRMLSliceEdgeWidgetRepresentation&) = delete;
  void operator=(const vtkMRMLSliceEdgeWidgetRepresentation&) = delete;
};

#endif
