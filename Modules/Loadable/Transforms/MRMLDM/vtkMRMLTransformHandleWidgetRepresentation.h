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
/// \class vtkMRMLTransformHandleWidgetRepresentation
/// \brief Abstract representation for the transform handle widgets
///
/// \sa vtkMRMLInteractionWidgetRepresentation
///

#ifndef vtkMRMLTransformHandleWidgetRepresentation_h
#define vtkMRMLTransformHandleWidgetRepresentation_h

#include "vtkSlicerTransformsModuleMRMLDisplayableManagerExport.h"

// MRMLDM includes
#include "vtkMRMLInteractionWidgetRepresentation.h"

// MRML includes
#include <vtkMRMLTransformDisplayNode.h>

// VTK includes
#include <vtkArrayCalculator.h>
#include <vtkOutlineSource.h>

class VTK_SLICER_TRANSFORMS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLTransformHandleWidgetRepresentation : public vtkMRMLInteractionWidgetRepresentation
{
public:
  /// Instantiate this class.
  static vtkMRMLTransformHandleWidgetRepresentation* New();

  ///@{
  /// Standard VTK class macros.
  vtkTypeMacro(vtkMRMLTransformHandleWidgetRepresentation, vtkMRMLInteractionWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  ///@}

  virtual int GetActiveComponentType() override;
  virtual void SetActiveComponentType(int type) override;
  virtual int GetActiveComponentIndex() override;
  virtual void SetActiveComponentIndex(int type) override;

  virtual vtkMRMLTransformDisplayNode* GetDisplayNode();
  virtual void SetDisplayNode(vtkMRMLTransformDisplayNode* displayNode);

  virtual vtkMRMLTransformNode* GetTransformNode();

  void UpdateInteractionPipeline() override;

  using Superclass::UpdateHandleToWorldTransform;
  void UpdateHandleToWorldTransform(vtkTransform* handleToWorldTransform) override;

  /// Size relative to screen
  double GetInteractionScalePercent() override;
  /// Size in mm
  double GetInteractionSizeMm() override;
  /// True -> size in mm; false -> relative to screen
  bool GetInteractionSizeAbsolute() override;

  bool GetHandleVisibility(int type, int index) override;

  bool IsDisplayable() override;

protected:
  vtkMRMLTransformHandleWidgetRepresentation();
  ~vtkMRMLTransformHandleWidgetRepresentation() override;

  vtkSmartPointer<vtkMRMLTransformDisplayNode> DisplayNode;

private:
  vtkMRMLTransformHandleWidgetRepresentation(const vtkMRMLTransformHandleWidgetRepresentation&) = delete;
  void operator=(const vtkMRMLTransformHandleWidgetRepresentation&) = delete;
};

#endif
