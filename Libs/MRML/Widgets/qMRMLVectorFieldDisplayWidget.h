/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

#ifndef __qMRMLVectorFieldDisplayWidget_h
#define __qMRMLVectorFieldDisplayWidget_h

// MRMLWidgets includes
#include "qMRMLWidget.h"

// CTK includes
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLWidgetsExport.h"

class qMRMLVectorFieldDisplayWidgetPrivate;
class vtkMRMLVectorFieldDisplayNode;
class vtkMRMLNode;

/// \brief Widget to edit the properties of a vtkMRMLVectorFieldDisplayNode.
class QMRML_WIDGETS_EXPORT qMRMLVectorFieldDisplayWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qMRMLWidget Superclass;
  qMRMLVectorFieldDisplayWidget(QWidget* parent = nullptr);
  ~qMRMLVectorFieldDisplayWidget() override;

  /// Get the vector field display node currently shown/edited by this widget.
  vtkMRMLVectorFieldDisplayNode* mrmlVectorFieldDisplayNode() const;

  /// Add a source-specific widget (e.g., transform color map editor) below the coloring controls, inside the
  /// Coloring section. The widget is reparented.
  void addColoringWidget(QWidget* widget);

signals:
  /// Emitted when the widget is set to show another display node, so that sections that are
  /// added to it (addColoringWidget()) can follow, for example a color legend.
  void mrmlVectorFieldDisplayNodeChanged(vtkMRMLVectorFieldDisplayNode* displayNode);

public slots:
  /// Set the vector field display node shown/edited by this widget.
  void setMRMLVectorFieldDisplayNode(vtkMRMLVectorFieldDisplayNode* displayNode);
  /// Utility function to be connected with generic signals.
  void setMRMLVectorFieldDisplayNode(vtkMRMLNode* node);

protected slots:
  void updateWidgetFromMRML();

  // Field and visualization mode. The index parameters are the row of the combo box.
  void setVisibility(bool visible);
  void setVisibility3D(bool visible);
  void setVisibility2D(bool visible);
  void setFieldArray(int index);
  void setInterpolateBetweenPoints(bool enabled);
  void setInterpolationMaximumDistance(double distance);
  /// Sets the mode of the mode button that sent the signal, when it is the one checked
  void setVisualizationModeFromButton(bool checked);
  void setScaleFactor(double value);
  void setScaleFactorReversed(bool reversed);
  void setThresholdEnabled(bool enabled);
  void setThresholdRange(double minimum, double maximum);

  // Sampling and masking
  void setSamplingRegionNode(vtkMRMLNode* node);
  void setSamplingRegionPart(int index);
  void setSamplingPattern(int index);
  void setSamplingSpacing(double spacing);
  void setSamplingPointsNumber(int number);
  void setMaskingRegionNode(vtkMRMLNode* node);
  void setMaskingMode(int index);
  void setMaskingSegments();

  // Glyphs
  void setGlyphType(int index);
  void setOrientationArray(int index);
  void setScaleArray(int index);
  void setVectorScaleMode(int index);
  void setGlyphDiameter(double diameter);
  void setGlyphDiameterAbsolute(bool absolute);
  void setGlyphShaftDiameter(double percent);
  void setGlyphTipLength(double percent);
  void setGlyphResolution(int resolution);
  void setGlyphTipLength2D(double percent);
  void setGlyphResolution2D(int resolution);

  // Grid, contours and streamlines
  void setGridSpacing(double spacing);
  void setGridShowNonWarped(bool show);
  void setGridNonWarpedColor(const QColor& color);
  void setGridLinesVisibility(bool visible);
  void setGridFill(bool fill);
  void setGridLineDiameter(double diameter);
  void setContourLevels();
  void setStreamlineBidirectional(bool bidirectional);
  void setStreamlineInitialStep(double step);
  void setStreamlineMaximumPropagation(double length);
  void setStreamlineTubeDiameter(double diameter);
  void setStreamlineSliceFadeDistance(double distance);

  // Coloring and appearance in the views
  void setColorBy(int index);
  void setSameColoringAsSurface(bool enabled);
  void setColor(const QColor& color);
  void setOpacity(double opacity);
  void setSliceOpacity(double opacity);
  void setSliceLineWidth(int width);

protected:
  QScopedPointer<qMRMLVectorFieldDisplayWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLVectorFieldDisplayWidget);
  Q_DISABLE_COPY(qMRMLVectorFieldDisplayWidget);
};

#endif
