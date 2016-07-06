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

#ifndef __qSlicerSegmentEditorAbstractIslandEffect_h
#define __qSlicerSegmentEditorAbstractIslandEffect_h

// Segmentations Editor Effects includes
#include "qSlicerSegmentationsEditorEffectsExport.h"

#include "qSlicerSegmentEditorAbstractEffect.h"

class qSlicerSegmentEditorAbstractIslandEffectPrivate;

/// \ingroup SlicerRt_QtModules_Segmentations
/// \brief Base class for all "island" effects.
///
/// This base class provides common GUI and MRML for the island (connected component)
/// operations (fully connected, minimum size)
class Q_SLICER_SEGMENTATIONS_EFFECTS_EXPORT qSlicerSegmentEditorAbstractIslandEffect :
  public qSlicerSegmentEditorAbstractEffect
{
  Q_OBJECT

public:
  typedef qSlicerSegmentEditorAbstractEffect Superclass;
  qSlicerSegmentEditorAbstractIslandEffect(QObject* parent = NULL);
  virtual ~qSlicerSegmentEditorAbstractIslandEffect();

  Q_INVOKABLE static QString fullyConnectedParameterName() { return QString("FullyConnected"); };
  Q_INVOKABLE static QString minimumSizeParameterName() { return QString("MinimumSize"); };

public:
  /// Clone editor effect
  /// (redefinition of pure virtual function to allow python wrapper to identify this as abstract class)
  virtual qSlicerSegmentEditorAbstractEffect* clone() = 0;

  /// Create options frame widgets, make connections, and add them to the main options frame using \sa addOptionsWidget
  virtual void setupOptionsFrame();

  /// Set default parameters in the parameter MRML node
  virtual void setMRMLDefaults();

public slots:
  /// Update user interface from parameter set node
  virtual void updateGUIFromMRML();

  /// Update parameter set node from user interface
  virtual void updateMRMLFromGUI();

protected:
  QScopedPointer<qSlicerSegmentEditorAbstractIslandEffectPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSegmentEditorAbstractIslandEffect);
  Q_DISABLE_COPY(qSlicerSegmentEditorAbstractIslandEffect);
};

#endif
