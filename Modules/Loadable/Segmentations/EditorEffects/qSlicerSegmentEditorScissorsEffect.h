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

#ifndef __qSlicerSegmentEditorScissorsEffect_h
#define __qSlicerSegmentEditorScissorsEffect_h

// Segmentations Editor Effects includes
#include "qSlicerSegmentationsEditorEffectsExport.h"

#include "qSlicerSegmentEditorAbstractLabelEffect.h"

class qSlicerSegmentEditorScissorsEffectPrivate;
class vtkPolyData;

/// \ingroup SlicerRt_QtModules_Segmentations
class Q_SLICER_SEGMENTATIONS_EFFECTS_EXPORT qSlicerSegmentEditorScissorsEffect :
  public qSlicerSegmentEditorAbstractLabelEffect
{
public:
  Q_OBJECT

public:
  typedef qSlicerSegmentEditorAbstractLabelEffect Superclass;
  qSlicerSegmentEditorScissorsEffect(QObject* parent = nullptr);
  ~qSlicerSegmentEditorScissorsEffect() override;

public:
  /// Get icon for effect to be displayed in segment editor
  QIcon icon() override;

  /// Get help text for effect to be displayed in the help box
  Q_INVOKABLE const QString helpText()const override;

  /// Create options frame widgets, make connections, and add them to the main options frame using \sa addOptionsWidget
  void setupOptionsFrame() override;

  /// Set default parameters in the parameter MRML node
  void setMRMLDefaults() override;

  /// Clone editor effect
  qSlicerSegmentEditorAbstractEffect* clone() override;

  /// Callback function invoked when interaction happens
  /// \param callerInteractor Interactor object that was observed to catch the event
  /// \param eid Event identifier
  /// \param viewWidget Widget of the Slicer layout view. Can be \sa qMRMLSliceWidget or \sa qMRMLThreeDWidget
  bool processInteractionEvents(vtkRenderWindowInteractor* callerInteractor, unsigned long eid, qMRMLWidget* viewWidget) override;

  /// Perform actions to deactivate the effect (such as destroy actors, etc.)
  Q_INVOKABLE void deactivate() override;

public slots:
  /// Update user interface from parameter set node
  void updateGUIFromMRML() override;

  /// Update parameter set node from user interface
  void updateMRMLFromGUI() override;

  virtual void setOperation(int operationIndex);
  virtual void setShape(int shapeIndex);
  virtual void setSliceCutMode(int sliceCutModeIndex);
  virtual void onSliceCutDepthChanged(double value);

protected:
  QScopedPointer<qSlicerSegmentEditorScissorsEffectPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSegmentEditorScissorsEffect);
  Q_DISABLE_COPY(qSlicerSegmentEditorScissorsEffect);
};

#endif
