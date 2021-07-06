/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMRMLSliceIntersectionInteractionRepresentation.cxx

  Copyright (c) Ebatinca S.L., Las Palmas de Gran Canaria, Spain
  All rights reserved.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/**
 * @class   vtkMRMLSliceIntersectionInteractionRepresentation
 * @brief   represent intersections of other slice views in the current slice view
 *
 * @sa
 * vtkSliceIntersectionWidget vtkWidgetRepresentation vtkAbstractWidget
*/

#ifndef vtkMRMLSliceIntersectionInteractionRepresentation_h
#define vtkMRMLSliceIntersectionInteractionRepresentation_h

#include "vtkMRMLDisplayableManagerExport.h" // For export macro
#include "vtkMRMLAbstractWidgetRepresentation.h"
#include "vtkMRMLSliceIntersectionInteractionRepresentationHelper.h"

#include "vtkMRMLSliceDisplayNode.h"
#include "vtkMRMLSliceNode.h"

class vtkMRMLApplicationLogic;
class vtkMRMLModelDisplayNode;
class vtkMRMLSliceLogic;

class vtkProperty2D;
class vtkActor2D;
class vtkPolyDataMapper2D;
class vtkPolyData;
class vtkPoints;
class vtkCellArray;
class vtkTextProperty;
class vtkLeaderActor2D;
class vtkTextMapper;
class vtkTransform;
class vtkActor2D;

class SliceIntersectionInteractionDisplayPipeline;
class vtkMRMLInteractionEventData;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLSliceIntersectionInteractionRepresentation : public vtkMRMLAbstractWidgetRepresentation
{
  public:
    /**
     * Instantiate this class.
     */
    static vtkMRMLSliceIntersectionInteractionRepresentation* New();

    //@{
    /**
     * Standard methods for instances of this class.
     */
    vtkTypeMacro(vtkMRMLSliceIntersectionInteractionRepresentation, vtkMRMLAbstractWidgetRepresentation);
    void PrintSelf(ostream& os, vtkIndent indent) override;
    //@}

    void SetSliceNode(vtkMRMLSliceNode* sliceNode);
    vtkMRMLSliceNode* GetSliceNode();

    vtkMRMLSliceDisplayNode* GetSliceDisplayNode();

    bool IsDisplayable();
    void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void* callData = nullptr) override;

    void AddIntersectingSliceLogic(vtkMRMLSliceLogic* sliceLogic);
    void RemoveIntersectingSliceNode(vtkMRMLSliceNode* sliceNode);
    void UpdateIntersectingSliceNodes();
    void RemoveAllIntersectingSliceNodes();

    //@{
    /**
     * Methods to make this class behave as a vtkProp.
     */
    void GetActors2D(vtkPropCollection*) override;
    void ReleaseGraphicsResources(vtkWindow*) override;
    int RenderOverlay(vtkViewport* viewport) override;
    //@}

    void SetMRMLApplicationLogic(vtkMRMLApplicationLogic*);
    vtkGetObjectMacro(MRMLApplicationLogic, vtkMRMLApplicationLogic);

    /// Get slice intersection point between red, green and yellow slice nodes
    double* GetSliceIntersectionPoint();

    /// Check whether the mouse cursor is within the slice view or not
    bool IsMouseCursorInSliceView(double cursorPosition[2]);

    void SetPipelinesHandlesVisibility(bool visible);
    void SetPipelinesHandlesOpacity(double opacity);

    void TransformIntersectingSlices(vtkMatrix4x4* rotatedSliceToSliceTransformMatrix);

    /// Return found component type (as vtkMRMLInteractionDisplayNode::ComponentType).
    /// closestDistance2 is the squared distance in display coordinates from the closest position where interaction is possible.
    /// componentIndex returns index of the found component (e.g., if control point is found then control point index is returned).
    virtual std::string CanInteract(vtkMRMLInteractionEventData* interactionEventData,
        int& foundComponentType, int& foundComponentIndex, double& closestDistance2, double& handleOpacity);

    virtual double GetMaximumHandlePickingDistance2();

    class HandleInfo
      {
      public:
        HandleInfo(int index, int componentType, const std::string& intersectingSliceNodeID, double positionWorld[3], double positionLocal[3])
          : Index(index)
          , ComponentType(componentType)
          , IntersectingSliceNodeID(intersectingSliceNodeID)
          {
          for (int i = 0; i < 3; ++i)
            {
            this->PositionWorld[i] = positionWorld[i];
            }
          this->PositionWorld[3] = 1.0;
          for (int i = 0; i < 3; ++i)
            {
            this->PositionLocal[i] = positionLocal[i];
            }
          this->PositionLocal[3] = 1.0;
          }
        int Index;
        int ComponentType;
        std::string IntersectingSliceNodeID;
        double PositionLocal[4];
        double PositionWorld[4];
      };

    /// Get the list of info for all interaction handles
    typedef std::vector<HandleInfo> HandleInfoList;
    virtual HandleInfoList GetHandleInfoList(SliceIntersectionInteractionDisplayPipeline* pipeline);

  protected:
    vtkMRMLSliceIntersectionInteractionRepresentation();
    ~vtkMRMLSliceIntersectionInteractionRepresentation() override;

    SliceIntersectionInteractionDisplayPipeline* GetDisplayPipelineFromSliceLogic(vtkMRMLSliceLogic* sliceLogic);

    static void SliceNodeModifiedCallback(vtkObject* caller, unsigned long eid, void* clientData, void* callData);
    void SliceNodeModified(vtkMRMLSliceNode* sliceNode);
    void SliceModelDisplayNodeModified(vtkMRMLModelDisplayNode* sliceNode);

    void UpdateSliceIntersectionDisplay(SliceIntersectionInteractionDisplayPipeline* pipeline);

    vtkMRMLSliceDisplayNode* GetSliceDisplayNode(vtkMRMLSliceNode* sliceNode);

    void SetSliceDisplayNode(vtkMRMLSliceDisplayNode* sliceDisplayNode);

    double GetSliceRotationAngleRad(int eventPos[2]);

    /// Support picking
    double LastEventPosition[2];

    /// Slice intersection point in XY
    double SliceIntersectionPoint[4];

    /// Handle size, specified in renderer world coordinate system.
    /// For slice views, renderer world coordinate system is the display coordinate system, so it is measured in pixels.
    /// For 3D views, renderer world coordinate system is the Slicer world coordinate system, so it is measured in the
    /// scene length unit (typically millimeters).
    double InteractionSize{ 3.0 };

    double GetViewScaleFactorAtPosition(double positionWorld[3]);

    vtkMRMLApplicationLogic* MRMLApplicationLogic;

    class vtkInternal;
    vtkInternal* Internal;

    vtkMRMLSliceIntersectionInteractionRepresentationHelper* Helper;

  private:
    vtkMRMLSliceIntersectionInteractionRepresentation(const vtkMRMLSliceIntersectionInteractionRepresentation&) = delete;
    void operator=(const vtkMRMLSliceIntersectionInteractionRepresentation&) = delete;
};

#endif
