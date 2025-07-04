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
 * @class   vtkSlicerMarkupsInteractionWidgetRepresentation
 * @brief   Abstract representation for the transform handle widgets
 *
 * @sa
 * vtkMRMLInteractionWidgetRepresentation
 */

#ifndef vtkSlicerMarkupsInteractionWidgetRepresentation_h
#define vtkSlicerMarkupsInteractionWidgetRepresentation_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"

// MRMLDM includes
#include "vtkMRMLInteractionWidgetRepresentation.h"

// MRML includes
#include <vtkMRMLMarkupsDisplayNode.h>

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerMarkupsInteractionWidgetRepresentation
  : public vtkMRMLInteractionWidgetRepresentation
{
public:
  /**
   * Instantiate this class.
   */
  static vtkSlicerMarkupsInteractionWidgetRepresentation* New();

  //@{
  /**
   * Standard VTK class macros.
   */
  vtkTypeMacro(vtkSlicerMarkupsInteractionWidgetRepresentation, vtkMRMLInteractionWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  int GetActiveComponentType() override;
  void SetActiveComponentType(int type) override;
  int GetActiveComponentIndex() override;
  void SetActiveComponentIndex(int type) override;
  bool GetHandleVisibility(int type, int index) override;
  bool GetApplyScaleToPosition(int type, int index) override;

  virtual int InteractionComponentToMarkupsComponent(int);
  virtual int MarkupsComponentToInteractionComponent(int);

  virtual vtkMRMLMarkupsDisplayNode* GetDisplayNode();
  virtual void SetDisplayNode(vtkMRMLMarkupsDisplayNode* displayNode);

  virtual vtkMRMLMarkupsNode* GetMarkupsNode();

  void UpdateInteractionPipeline() override;

  virtual void UpdatePlaneScaleHandles();
  virtual void UpdateROIScaleHandles();
  virtual void UpdateROIScaleHandles3D();
  virtual void UpdateROIScaleHandles2D();

  using Superclass::UpdateHandleToWorldTransform;
  void UpdateHandleToWorldTransform(vtkTransform* handleToWorldTransform) override;

  double GetInteractionScalePercent() override; // Size relative to screen
  double GetInteractionSizeMm() override;       // Size in mm
  bool GetInteractionSizeAbsolute() override;   // True -> size in mm; false -> relative to screen

  void GetInteractionHandlePositionWorld(int type, int index, double positionWorld[3]) override;

  bool IsDisplayable() override;

  void CreateScaleHandles() override;

  void GetInteractionHandleAxisLocal(int type, int index, double axis_Local[3]) override;
  void GetHandleColor(int type, int index, double color[4]) override;

  bool AddScaleEdgeIntersection(int pointIndex,
                                vtkIdTypeArray* visibilityArray,
                                vtkPoints* scaleHandleArray,
                                double sliceNormal[3],
                                double sliceOrigin[3],
                                double edgePoint[3],
                                double edgeVector[3]);

protected:
  vtkSlicerMarkupsInteractionWidgetRepresentation();
  ~vtkSlicerMarkupsInteractionWidgetRepresentation() override;

  vtkSmartPointer<vtkMRMLMarkupsDisplayNode> DisplayNode{ nullptr };

private:
  vtkSlicerMarkupsInteractionWidgetRepresentation(const vtkSlicerMarkupsInteractionWidgetRepresentation&) = delete;
  void operator=(const vtkSlicerMarkupsInteractionWidgetRepresentation&) = delete;
};

#endif
