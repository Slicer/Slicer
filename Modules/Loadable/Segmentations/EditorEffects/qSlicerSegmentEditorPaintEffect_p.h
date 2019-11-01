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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qSlicerSegmentEditorPaintEffect_p_h
#define __qSlicerSegmentEditorPaintEffect_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Segmentations Editor Effects includes
#include "qSlicerSegmentationsEditorEffectsExport.h"

#include "qSlicerSegmentEditorPaintEffect.h"

// VTK includes
#include <vtkCutter.h>
#include <vtkCylinderSource.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkWeakPointer.h>

// Qt includes
#include <QObject>
#include <QList>
#include <QMap>

class BrushPipeline;
class ctkDoubleSlider;
class QPoint;
class QIcon;
class QFrame;
class QCheckBox;
class QToolButton;
class qMRMLSliceWidget;
class qMRMLSpinBox;
class vtkActor2D;
class vtkGlyph3D;
class vtkPoints;
class vtkPolyDataNormals;
class vtkPolyDataToImageStencil;

/// \ingroup SlicerRt_QtModules_Segmentations
/// \brief Private implementation of the segment editor paint effect
class qSlicerSegmentEditorPaintEffectPrivate: public QObject
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qSlicerSegmentEditorPaintEffect);
protected:
  qSlicerSegmentEditorPaintEffect* const q_ptr;
public:
  typedef QObject Superclass;
  qSlicerSegmentEditorPaintEffectPrivate(qSlicerSegmentEditorPaintEffect& object);
  ~qSlicerSegmentEditorPaintEffectPrivate() override;

  /// Depending on the \sa DelayedPaint mode, either paint the given point or queue
  /// it up with a marker for later painting
  void paintAddPoint(qMRMLWidget* viewWidget, double pixelPositionWorld[3]);

  /// Update paint circle glyph
  void updateBrush(qMRMLWidget* viewWidget, BrushPipeline* brush);

  /// Update brushes
  void updateBrushes();

  /// Update brush model (shape and position)
  void updateBrushModel(qMRMLWidget* viewWidget, double brushPosition_World[3]);

  /// Updates the brush stencil that can be used to quickly paint the brush shape into
  /// modifierLabelmap at many different positions.
  void updateBrushStencil(qMRMLWidget* viewWidget);

protected:
  /// Get brush object for widget. Create if does not exist
  BrushPipeline* brushForWidget(qMRMLWidget* viewWidget);

  /// Delete all brush pipelines
  void clearBrushPipelines();

  /// Paint labelmap
  void paintApply(qMRMLWidget* viewWidget);

  /// Paint brushes to the modifier labelmap
  void paintBrushes(vtkOrientedImageData* modifierLabelmap, qMRMLWidget* viewWidget, vtkPoints* pixelPositions_World, int extent[6]=nullptr);

  /// Paint one pixel at coordinate
  void paintPixel(vtkOrientedImageData* modifierLabelmap, qMRMLWidget* viewWidget, double pixelPosition_World[3]);

  /// Paint pixels at the coordinates
  void paintPixels(vtkOrientedImageData* modifierLabelmap, vtkPoints* pixelPositions_World, int extent[6]=nullptr);

  /// Transform points from World to IJK
  void transformPointsFromWorldToIJK(vtkOrientedImageData* image, vtkMRMLSegmentationNode* segmentationNode, vtkPoints* inputPoints, vtkPoints* outputPoints);

  /// Scale brush diameter and save it in parameter node
  void scaleDiameter(double scaleFactor);

  // Compute absolute size (in mm) from relative size (percentage of screen height)
  void updateAbsoluteBrushDiameter();

  bool brushPositionInWorld(qMRMLWidget* viewWidget, int brushPositionInView[2], double brushPosition_World[3]);

  std::string segmentAtPosition(qMRMLWidget* viewWidget, double* ras);

public slots:
  void onDiameterUnitsClicked();
  void onQuickDiameterButtonClicked();
  void onDiameterValueChanged(double);

public:
  QIcon PaintIcon;

  vtkSmartPointer<vtkCylinderSource> BrushCylinderSource;
  vtkSmartPointer<vtkSphereSource> BrushSphereSource;
  vtkSmartPointer<vtkTransformPolyDataFilter> BrushToWorldOriginTransformer;
  vtkSmartPointer<vtkTransform> BrushToWorldOriginTransform;

  vtkSmartPointer<vtkTransformPolyDataFilter> WorldOriginToWorldTransformer;
  vtkSmartPointer<vtkTransform> WorldOriginToWorldTransform;
  vtkSmartPointer<vtkPolyDataNormals> BrushPolyDataNormals;
  vtkSmartPointer<vtkTransformPolyDataFilter> WorldOriginToModifierLabelmapIjkTransformer;
  vtkSmartPointer<vtkTransform> WorldOriginToModifierLabelmapIjkTransform; // transforms from polydata source to modifierLabelmap's IJK coordinate system (brush origin in IJK origin)
  vtkSmartPointer<vtkPolyDataToImageStencil> BrushPolyDataToStencil;

  vtkSmartPointer<vtkGlyph3D> FeedbackGlyphFilter;

  vtkSmartPointer<vtkPoints> PaintCoordinates_World;
  vtkSmartPointer<vtkPolyData> FeedbackPointsPolyData;

  // If a new point is added at less than this squared distance
  // then the point is not added. It can be used for limiting number of
  // points to improve performance.
  double MinimumPaintPointDistance2;

  QList<vtkActor2D*> FeedbackActors;
  QMap<qMRMLWidget*, BrushPipeline*> BrushPipelines;
  bool DelayedPaint;
  bool IsPainting;

  // Observed view node
  qMRMLWidget* ActiveViewWidget;
  int ActiveViewLastInteractionPosition[2];
  int ActiveViewLastPaintPosition[2];

  QFrame* BrushDiameterFrame;
  qMRMLSpinBox* BrushDiameterSpinBox;
  ctkDoubleSlider* BrushDiameterSlider;
  QToolButton* BrushDiameterRelativeToggle;
  QCheckBox* BrushSphereCheckbox;
  QCheckBox* EditIn3DViewsCheckbox;
  QCheckBox* ColorSmudgeCheckbox;
  QCheckBox* EraseAllSegmentsCheckbox;
  QCheckBox* BrushPixelModeCheckbox;
};

#endif
