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

#ifndef __qSlicerSegmentEditorPaintEffect_h
#define __qSlicerSegmentEditorPaintEffect_h

// Segmentations Editor Effects includes
#include "qSlicerSegmentationsEditorEffectsExport.h"

#include "qSlicerSegmentEditorAbstractLabelEffect.h"

class qSlicerSegmentEditorPaintEffectPrivate;
class vtkPolyData;

/// \ingroup SlicerRt_QtModules_Segmentations
class Q_SLICER_SEGMENTATIONS_EFFECTS_EXPORT qSlicerSegmentEditorPaintEffect :
  public qSlicerSegmentEditorAbstractLabelEffect
{
  Q_OBJECT

public:
  Q_PROPERTY(double minimumPaintPointDistance READ minimumPaintPointDistance WRITE setMinimumPaintPointDistance)
  Q_PROPERTY(bool delayedPaint READ delayedPaint WRITE setDelayedPaint)

  typedef qSlicerSegmentEditorAbstractLabelEffect Superclass;
  qSlicerSegmentEditorPaintEffect(QObject* parent = nullptr);
  ~qSlicerSegmentEditorPaintEffect() override;

public:
  /// Get icon for effect to be displayed in segment editor
  QIcon icon() override;

  /// Get help text for effect to be displayed in the help box
  Q_INVOKABLE const QString helpText()const override;

  /// Clone editor effect
  qSlicerSegmentEditorAbstractEffect* clone() override;

  /// Perform actions to deactivate the effect (such as destroy actors, etc.)
  Q_INVOKABLE void deactivate() override;

  /// Callback function invoked when interaction happens
  /// \param callerInteractor Interactor object that was observed to catch the event
  /// \param eid Event identifier
  /// \param viewWidget Widget of the Slicer layout view. Can be \sa qMRMLSliceWidget or \sa qMRMLThreeDWidget
  bool processInteractionEvents(vtkRenderWindowInteractor* callerInteractor, unsigned long eid, qMRMLWidget* viewWidget) override;

  /// Callback function invoked when view node is modified
  /// \param callerViewNode View node that was observed to catch the event. Can be either \sa vtkMRMLSliceNode or \sa vtkMRMLViewNode
  /// \param eid Event identifier
  /// \param viewWidget Widget of the Slicer layout view. Can be \sa qMRMLSliceWidget or \sa qMRMLThreeDWidget
  void processViewNodeEvents(vtkMRMLAbstractViewNode* callerViewNode, unsigned long eid, qMRMLWidget* viewWidget) override;

  /// Create options frame widgets, make connections, and add them to the main options frame using \sa addOptionsWidget
  void setupOptionsFrame() override;

  /// Set default parameters in the parameter MRML node
  void setMRMLDefaults() override;

  /// Perform actions needed on reference geometry change
  void referenceGeometryChanged() override;

  /// If a new point is added at less than this squared distance
  /// (in display coordinate system) then the point is not added.
  /// This is an experimental feature
  /// that may help in limiting number of paint points to
  /// improve performance.
  double minimumPaintPointDistance();

  /// If enabled then segmentation is only modified when the mouse button is released.
  bool delayedPaint();

public slots:
  /// Update user interface from parameter set node
  void updateGUIFromMRML() override;

  /// Update parameter set node from user interface
  void updateMRMLFromGUI() override;

  /// \sa minimumPaintPointDistance
  void setMinimumPaintPointDistance(double dist);

  /// \sa delayedPaint
  void setDelayedPaint(bool delayed);

protected:
  /// Flag determining to always erase (not just when smudge from empty region)
  /// Overridden in the \sa qSlicerSegmentEditorEraseEffect subclass
  bool m_AlwaysErase;

  /// Flag determining whether to paint or erase.
  bool m_Erase;

protected:
  QScopedPointer<qSlicerSegmentEditorPaintEffectPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSegmentEditorPaintEffect);
  Q_DISABLE_COPY(qSlicerSegmentEditorPaintEffect);
};

#endif
