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

#ifndef __qSlicerSegmentEditorAbstractLabelEffect_p_h
#define __qSlicerSegmentEditorAbstractLabelEffect_p_h

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

#include "qSlicerSegmentEditorAbstractLabelEffect.h"

// Qt includes
#include <QObject>

class QCheckBox;
class QLabel;
class ctkRangeWidget;

/// \ingroup SlicerRt_QtModules_Segmentations
/// \brief Private implementation of the segment editor abstract label effect
class qSlicerSegmentEditorAbstractLabelEffectPrivate: public QObject
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qSlicerSegmentEditorAbstractLabelEffect);
protected:
  qSlicerSegmentEditorAbstractLabelEffect* const q_ptr;
public:
  typedef QObject Superclass;
  qSlicerSegmentEditorAbstractLabelEffectPrivate(qSlicerSegmentEditorAbstractLabelEffect& object);
  ~qSlicerSegmentEditorAbstractLabelEffectPrivate() override;

protected slots:

public:
};

#endif
