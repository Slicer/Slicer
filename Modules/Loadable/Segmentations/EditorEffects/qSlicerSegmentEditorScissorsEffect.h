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
  qSlicerSegmentEditorScissorsEffect(QObject* parent = NULL);
  virtual ~qSlicerSegmentEditorScissorsEffect();

public:
  /// Get icon for effect to be displayed in segment editor
  virtual QIcon icon();

  /// Get help text for effect to be displayed in the help box
  Q_INVOKABLE virtual const QString helpText()const;

  /// Create options frame widgets, make connections, and add them to the main options frame using \sa addOptionsWidget
  virtual void setupOptionsFrame();

  /// Set default parameters in the parameter MRML node
  virtual void setMRMLDefaults();

  /// Clone editor effect
  virtual qSlicerSegmentEditorAbstractEffect* clone();

  /// Callback function invoked when interaction happens
  /// \param callerInteractor Interactor object that was observed to catch the event
  /// \param eid Event identifier
  /// \param viewWidget Widget of the Slicer layout view. Can be \sa qMRMLSliceWidget or \sa qMRMLThreeDWidget
  virtual bool processInteractionEvents(vtkRenderWindowInteractor* callerInteractor, unsigned long eid, qMRMLWidget* viewWidget);

  /// Perform actions to deactivate the effect (such as destroy actors, etc.)
  Q_INVOKABLE virtual void deactivate();

public slots:
  /// Update user interface from parameter set node
  virtual void updateGUIFromMRML();

  /// Update parameter set node from user interface
  virtual void updateMRMLFromGUI();

  virtual void setOperation(int operationIndex);
  virtual void setShape(int shapeIndex);

protected:
  QScopedPointer<qSlicerSegmentEditorScissorsEffectPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSegmentEditorScissorsEffect);
  Q_DISABLE_COPY(qSlicerSegmentEditorScissorsEffect);
};

#endif
