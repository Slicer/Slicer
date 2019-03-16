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

#ifndef __qSlicerSegmentEditorEraseEffect_h
#define __qSlicerSegmentEditorEraseEffect_h

// Segmentations Editor Effects includes
#include "qSlicerSegmentationsEditorEffectsExport.h"

#include "qSlicerSegmentEditorPaintEffect.h"

class qSlicerSegmentEditorEraseEffectPrivate;

/// \ingroup SlicerRt_QtModules_Segmentations
class Q_SLICER_SEGMENTATIONS_EFFECTS_EXPORT qSlicerSegmentEditorEraseEffect :
  public qSlicerSegmentEditorPaintEffect
{
public:
  Q_OBJECT

public:
  typedef qSlicerSegmentEditorPaintEffect Superclass;
  qSlicerSegmentEditorEraseEffect(QObject* parent = nullptr);
  ~qSlicerSegmentEditorEraseEffect() override;

public:
  /// Get icon for effect to be displayed in segment editor
  QIcon icon() override;

  /// Get help text for effect to be displayed in the help box
  Q_INVOKABLE const QString helpText()const override;

  /// Clone editor effect
  qSlicerSegmentEditorAbstractEffect* clone() override;

protected:
  QScopedPointer<qSlicerSegmentEditorEraseEffectPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSegmentEditorEraseEffect);
  Q_DISABLE_COPY(qSlicerSegmentEditorEraseEffect);
};

#endif
