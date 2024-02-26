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
/// \class   vtkSlicerMarkupsInteractionWidget
/// \brief   Process interaction events to update state of interaction widgets
///
/// \sa vtkMRMLAbstractWidget vtkSlicerWidgetRepresentation vtkSlicerWidgetEventTranslator
///

#ifndef vtkSlicerMarkupsInteractionWidget_h
#define vtkSlicerMarkupsInteractionWidget_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"

#include "vtkMRMLInteractionWidget.h"

class vtkMRMLAbstractViewNode;
class vtkMRMLApplicationLogic;
class vtkMRMLDisplayableNode;
class vtkMRMLInteractionEventData;
class vtkMRMLInteractionNode;
class vtkIdList;
class vtkPolyData;
class vtkTransform;
class vtkMRMLMarkupsNode;
class vtkMRMLMarkupsDisplayNode;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerMarkupsInteractionWidget : public vtkMRMLInteractionWidget
{
public:
  /// Instantiate this class.
  static vtkSlicerMarkupsInteractionWidget* New();

  ///@{
  /// Standard VTK class macros.
  vtkTypeMacro(vtkSlicerMarkupsInteractionWidget, vtkMRMLInteractionWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  ///@}

  /// Create the default widget representation and initializes the widget and representation.
  virtual void CreateDefaultRepresentation(vtkMRMLMarkupsDisplayNode* displayNode,
                                           vtkMRMLAbstractViewNode* viewNode,
                                           vtkRenderer* renderer);

  virtual vtkMRMLMarkupsDisplayNode* GetDisplayNode();
  virtual vtkMRMLMarkupsNode* GetMarkupsNode();

  int GetActiveComponentType() override;
  void SetActiveComponentType(int type) override;

  int GetActiveComponentIndex() override;
  void SetActiveComponentIndex(int type) override;

  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) override;

  void ScaleWidget(double eventPos[2], bool uniformScale = false) override;
  virtual void ScaleWidgetPlane(double eventPos[2], bool symmetricScale);
  virtual void FlipPlaneHandles(bool flipLRHandle, bool flipPAHandle);
  virtual void ScaleWidgetROI(double eventPos[2], bool symmetricScale);
  virtual void FlipROIHandles(bool flipLRHandle, bool flipPAHandle, bool flipISHandle);

  virtual vtkSlicerMarkupsInteractionWidget* CreateInstance() const;

  bool ProcessWidgetJumpCursor(vtkMRMLInteractionEventData* eventData);

  bool ProcessWidgetMenu(vtkMRMLInteractionEventData* eventData) override;

protected:
  vtkSlicerMarkupsInteractionWidget();
  ~vtkSlicerMarkupsInteractionWidget() override;

  void ApplyTransform(vtkTransform* transform) override;

private:
  vtkSlicerMarkupsInteractionWidget(const vtkSlicerMarkupsInteractionWidget&) = delete;
  void operator=(const vtkSlicerMarkupsInteractionWidget&) = delete;
};

//----------------------------------------------------------------------
// CREATE INSTANCE MACRO

#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
# define vtkSlicerMarkupsInteractionWidgetCreateInstanceMacroBody(type) \
   vtkObject* ret = vtkObjectFactory::CreateInstance(#type);            \
   if (ret)                                                             \
   {                                                                    \
     return static_cast<type*>(ret);                                    \
   }                                                                    \
   type* result = new type;                                             \
   result->InitializeObjectBase();                                      \
   return result;
#else
# define vtkSlicerMarkupsInteractionWidgetCreateInstanceMacroBody(type) \
   vtkObject* ret = vtkObjectFactory::CreateInstance(#type);            \
   if (ret)                                                             \
   {                                                                    \
     return static_cast<type*>(ret);                                    \
   }                                                                    \
   return new type;
#endif

#define vtkSlicerMarkupsInteractionWidgetCreateInstanceMacro(type)   \
  vtkSlicerMarkupsInteractionWidget* CreateInstance() const override \
  {                                                                  \
    vtkSlicerMarkupsInteractionWidgetCreateInstanceMacroBody(type)   \
  }

#endif
