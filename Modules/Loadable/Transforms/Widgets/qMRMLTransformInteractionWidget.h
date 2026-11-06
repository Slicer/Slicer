/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

  This file was further developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

#ifndef __qMRMLTransformInteractionWidget_h
#define __qMRMLTransformInteractionWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Slicer includes
#include "qMRMLWidget.h"

#include "qSlicerTransformsModuleWidgetsExport.h"

class qMRMLTransformInteractionWidgetPrivate;
class vtkMRMLTransformNode;
class vtkMRMLNode;

/// \brief Edit how a transform can be grabbed and changed in the views.
///
/// Edits the vtkMRMLTransformInteractionDisplayNode properties (which vtkMRMLTransformDisplayNode accessors
/// forward to); the displacement field display is edited by qMRMLVectorFieldDisplayWidget.
///
/// \sa qMRMLTransformDisplayNodeWidget, vtkMRMLTransformInteractionDisplayNode
class Q_SLICER_MODULE_TRANSFORMS_WIDGETS_EXPORT qMRMLTransformInteractionWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qMRMLWidget Superclass;
  qMRMLTransformInteractionWidget(QWidget* newParent = nullptr);
  ~qMRMLTransformInteractionWidget() override;

public slots:

  /// Set the transform whose interaction is edited. Setting it to nullptr disables the
  /// widget.
  void setMRMLTransformNode(vtkMRMLTransformNode* transformNode);

  /// Utility function that calls setMRMLTransformNode(vtkMRMLTransformNode*)
  void setMRMLTransformNode(vtkMRMLNode* node);

  void setEditorVisibility(bool enabled);
  void setEditorVisibility3d(bool enabled);
  void setEditorVisibility2d(bool enabled);

  void setEditorTranslationEnabled(bool enabled);
  void setEditorTranslationSliceEnabled(bool enabled);
  void setEditorTranslationSliceAnywhereEnabled(bool enabled);
  void setEditorTranslationSliceAnywhereSensitivity(double sensitivity);

  void setEditorRotationEnabled(bool enabled);
  void setEditorRotationSliceEnabled(bool enabled);

  void setEditorScalingEnabled(bool enabled);
  void setEditorScalingSliceEnabled(bool enabled);

  void setInteractionHandleScale(double scalePercent);
  void setInteractionHandleOpacity(double opacity);

  /// Store which components (X, Y, Z, view plane) of each kind of handle are shown, as the
  /// check boxes say
  void updateHandleComponentVisibility();

protected slots:
  void updateWidgetFromDisplayNode();

protected:
  QScopedPointer<qMRMLTransformInteractionWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLTransformInteractionWidget);
  Q_DISABLE_COPY(qMRMLTransformInteractionWidget);
};

#endif
