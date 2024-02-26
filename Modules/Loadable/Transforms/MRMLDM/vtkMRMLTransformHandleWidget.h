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
/// \class vtkMRMLTransformHandleWidget
/// \brief Process interaction events to update state of interaction widgets
///
/// \sa vtkMRMLAbstractWidget vtkSlicerWidgetRepresentation vtkSlicerWidgetEventTranslator
///

#ifndef vtkMRMLTransformHandleWidget_h
#define vtkMRMLTransformHandleWidget_h

#include "vtkSlicerTransformsModuleMRMLDisplayableManagerExport.h"

#include "vtkMRMLInteractionWidget.h"

class vtkMRMLAbstractViewNode;
class vtkMRMLApplicationLogic;
class vtkMRMLDisplayableNode;
class vtkMRMLInteractionEventData;
class vtkMRMLInteractionNode;
class vtkIdList;
class vtkPolyData;
class vtkTransform;
class vtkMRMLTransformDisplayNode;
class vtkMRMLTransformNode;

class VTK_SLICER_TRANSFORMS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLTransformHandleWidget
  : public vtkMRMLInteractionWidget
{
public:
  /// Instantiate this class.
  static vtkMRMLTransformHandleWidget* New();

  ///@{
  /// Standard VTK class macros.
  vtkTypeMacro(vtkMRMLTransformHandleWidget, vtkMRMLInteractionWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  ///@}

  /// The state of the widget
  enum
  {
    WidgetStateTranslateTransformCenter =
      WidgetStateInteraction_Last ///< mouse move transforms the center of transformation
  };

  enum
  {
    WidgetEventTranslateTransformCenterStart = WidgetEventInteraction_Last,
    WidgetEventTranslateTransformCenterEnd,
  };

  /// Create the default widget representation and initializes the widget and representation.
  virtual void CreateDefaultRepresentation(vtkMRMLTransformDisplayNode* displayNode,
                                           vtkMRMLAbstractViewNode* viewNode,
                                           vtkRenderer* renderer);

  virtual vtkMRMLTransformDisplayNode* GetDisplayNode();
  virtual vtkMRMLTransformNode* GetTransformNode();

  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2) override;
  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) override;

protected:
  vtkMRMLTransformHandleWidget();
  ~vtkMRMLTransformHandleWidget() override;

  virtual bool ProcessWidgetTranslateTransformCenterStart(vtkMRMLInteractionEventData* eventData);
  bool ProcessMouseMove(vtkMRMLInteractionEventData* eventData) override;
  bool ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData) override;
  bool ProcessWidgetMenu(vtkMRMLInteractionEventData* eventData) override;
  bool ProcessJumpCursor(vtkMRMLInteractionEventData* eventData) override;

  virtual void TranslateTransformCenter(double eventPos[2]);

  void ApplyTransform(vtkTransform* transform) override;

private:
  vtkMRMLTransformHandleWidget(const vtkMRMLTransformHandleWidget&) = delete;
  void operator=(const vtkMRMLTransformHandleWidget&) = delete;
};

#endif
