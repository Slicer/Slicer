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

==============================================================================*/


#ifndef __qMRMLTransformDisplayNodeWidget_h
#define __qMRMLTransformDisplayNodeWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Slicer includes
#include "qMRMLWidget.h"

#include "qSlicerTransformsModuleWidgetsExport.h"

class qMRMLTransformDisplayNodeWidgetPrivate;
class vtkMRMLTransformNode;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_Transforms
class Q_SLICER_MODULE_TRANSFORMS_WIDGETS_EXPORT
qMRMLTransformDisplayNodeWidget
  : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qMRMLWidget Superclass;
  qMRMLTransformDisplayNodeWidget(QWidget *newParent = nullptr);
  ~qMRMLTransformDisplayNodeWidget() override;

public slots:

  /// Set the MRML node of interest
  /// Note that setting transformNode to 0 will disable the widget
  void setMRMLTransformNode(vtkMRMLTransformNode* transformNode);

  /// Utility function that calls setMRMLTransformNode(vtkMRMLTransformNode* transformNode)
  /// It's useful to connect to vtkMRMLNode* signals
  void setMRMLTransformNode(vtkMRMLNode* node);

  void setVisibility(bool);
  void setVisibility2d(bool);
  void setVisibility3d(bool);

  void setGlyphVisualizationMode(bool);
  void setGridVisualizationMode(bool);
  void setContourVisualizationMode(bool);

  void updateGlyphSourceOptions(int sourceOption);
  void regionNodeChanged(vtkMRMLNode* node);
  void glyphPointsNodeChanged(vtkMRMLNode* node);
  void setGlyphSpacingMm(double spacing);
  void setGlyphScalePercent(double scale);
  void setGlyphDisplayRangeMm(double min, double max);
  void setGlyphType(int glyphType);
  void setGlyphTipLengthPercent(double length);
  void setGlyphDiameterMm(double diameterMm);
  void setGlyphShaftDiameterPercent(double diameterPercent);
  void setGlyphResolution(double resolution);
  void setGridScalePercent(double scale);
  void setGridSpacingMm(double spacing);
  void setGridLineDiameterMm(double diameterMm);
  void setGridResolutionMm(double resolutionMm);
  void setGridShowNonWarped(bool show);
  void setContourLevelsMm(QString values_str);
  void setContourResolutionMm(double resolutionMm);
  void setContourOpacityPercent(double opacity);

  void setEditorVisibility(bool enabled);
  void setEditorTranslationEnabled(bool enabled);
  void setEditorRotationEnabled(bool enabled);
  void setEditorScalingEnabled(bool enabled);
  void updateEditorBounds();

  void setColorTableNode(vtkMRMLNode* colorTableNode);

  void colorUpdateRange();
  void onColorInteractionEvent();
  void onColorModifiedEvent();

protected slots:
  void updateWidgetFromDisplayNode();

protected:
  QScopedPointer<qMRMLTransformDisplayNodeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLTransformDisplayNodeWidget);
  Q_DISABLE_COPY(qMRMLTransformDisplayNodeWidget);

};

#endif
