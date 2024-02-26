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

///
/// \class vtkMRMLInteractionWidget
/// \brief Process interaction events to update state of interaction widgets
///
/// \sa vtkMRMLAbstractWidget vtkSlicerWidgetRepresentation vtkSlicerWidgetEventTranslator
///

#ifndef vtkMRMLInteractionWidget_h
#define vtkMRMLInteractionWidget_h

#include "vtkMRMLDisplayableManagerExport.h"

#include "vtkMRMLAbstractWidget.h"

class vtkMRMLAbstractViewNode;
class vtkMRMLApplicationLogic;
class vtkMRMLDisplayNode;
class vtkMRMLDisplayableNode;
class vtkMRMLInteractionEventData;
class vtkMRMLInteractionNode;
class vtkIdList;
class vtkPolyData;
class vtkMRMLInteractionWidgetRepresentation;
class vtkTransform;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLInteractionWidget : public vtkMRMLAbstractWidget
{
public:
  ///@{
  /// Standard VTK class macros.
  vtkTypeMacro(vtkMRMLInteractionWidget, vtkMRMLAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  ///@}

  enum
  {
    InteractionNone,
    InteractionTranslationHandle,
    InteractionRotationHandle,
    InteractionScaleHandle,
  };

  /// Widget states
  enum
  {
    WidgetStateInteraction_First = WidgetStateUser,
    WidgetStateOnTranslationHandle = WidgetStateInteraction_First, // hovering over a translation interaction handle
    WidgetStateOnRotationHandle,                                   // hovering over a rotation interaction handle
    WidgetStateOnScaleHandle,                                      // hovering over a scale interaction handle
    WidgetStateInteraction_Last
  };

  /// Widget events
  enum
  {
    WidgetEventReserved = WidgetEventUser, // this events is only to prevent other widgets from processing an event
    WidgetStateUniformScale,
    WidgetEventUniformScaleStart,
    WidgetEventUniformScaleEnd,
    WidgetEventInteraction_Last
  };

  virtual int GetActiveComponentType();
  virtual void SetActiveComponentType(int type);

  virtual int GetActiveComponentIndex();
  virtual void SetActiveComponentIndex(int index);

  /// Return true if the widget can process the event.
  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2) override;

  /// Process interaction event.
  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) override;

  /// Called when the the widget loses the focus.
  void Leave(vtkMRMLInteractionEventData* eventData) override;

  /// Allows the widget to request interactive mode (faster updates)
  bool GetInteractive() override;
  /// Allows the widget to request a cursor shape
  int GetMouseCursor() override;

protected:
  vtkMRMLInteractionWidget();
  ~vtkMRMLInteractionWidget() override;

  void StartWidgetInteraction(vtkMRMLInteractionEventData* eventData);
  void EndWidgetInteraction();

  virtual void TranslateWidget(double eventPos[2]);
  virtual void ScaleWidget(double eventPos[2], bool uniform = false);
  virtual void RotateWidget(double eventPos[2]);
  virtual void ApplyTransform(vtkTransform* transform) = 0;

  /// Index of the control point that is currently being previewed (follows the mouse pointer).
  /// If <0 it means that there is currently no point being previewed.
  int PreviewPointIndex;

  /// Callback interface to capture events when
  /// placing the widget.
  /// Return true if the event is processed.
  virtual bool ProcessMouseMove(vtkMRMLInteractionEventData* eventData);
  virtual bool ProcessWidgetMenu(vtkMRMLInteractionEventData* eventData) = 0;
  virtual bool ProcessWidgetMenuDisplayNodeTypeAndIndex(vtkMRMLInteractionEventData* eventData,
                                                        vtkMRMLDisplayNode* displayNode,
                                                        int type,
                                                        int index);
  virtual bool ProcessWidgetTranslateStart(vtkMRMLInteractionEventData* eventData);
  virtual bool ProcessWidgetRotateStart(vtkMRMLInteractionEventData* eventData);
  virtual bool ProcessWidgetScaleStart(vtkMRMLInteractionEventData* eventData);
  virtual bool ProcessWidgetUniformScaleStart(vtkMRMLInteractionEventData* eventData);
  virtual bool ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData);
  virtual bool ProcessJumpCursor(vtkMRMLInteractionEventData* eventData);

  // Jump to the handle position for the given type and index. Returns true if successful.
  virtual bool JumpToHandlePosition(int type, int index);

  /// Get the closest point on the line defined by the interaction handle axis.
  /// Input coordinates are in display coordinates, while output are in world coordinates.
  virtual bool GetClosestPointOnInteractionAxis(int type,
                                                int index,
                                                const double inputDisplay[2],
                                                double outputIntersectionWorld[3]);

  /// Get the closest point on the plane defined using the interaction handle axis as the plane normal.
  /// Input coordinates are in display coordinates, while output are in world coordinates
  virtual bool GetIntersectionOnAxisPlane(int type,
                                          int index,
                                          const double inputDisplay[2],
                                          double outputIntersectionWorld[3]);

  /// Variables for translate/rotate/scale
  double LastEventPosition[2];
  double StartEventOffsetPosition[2];

  bool ConvertDisplayPositionToWorld(const int displayPos[2],
                                     double worldPos[3],
                                     double worldOrientationMatrix[9],
                                     double* refWorldPos = nullptr);

private:
  vtkMRMLInteractionWidget(const vtkMRMLInteractionWidget&) = delete;
  void operator=(const vtkMRMLInteractionWidget&) = delete;
};

#endif
