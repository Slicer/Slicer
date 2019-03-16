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

#ifndef __qSlicerSegmentEditorAbstractEffect_p_h
#define __qSlicerSegmentEditorAbstractEffect_p_h

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

#include "qSlicerSegmentEditorAbstractEffect.h"

// VTK includes
#include <vtkWeakPointer.h>
#include <vtkSmartPointer.h>
#include <vtkActor2DCollection.h>
#include <vtkProp3DCollection.h>

// Qt includes
#include <QObject>
#include <QCursor>
#include <QHash>

class vtkMRMLScene;
class vtkMRMLSegmentEditorNode;
class qMRMLWidget;
class vtkProp;
class QFrame;

//-----------------------------------------------------------------------------
class qSlicerSegmentEditorAbstractEffectPrivate: public QObject
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qSlicerSegmentEditorAbstractEffect);
protected:
  qSlicerSegmentEditorAbstractEffect* const q_ptr;
public:
  qSlicerSegmentEditorAbstractEffectPrivate(qSlicerSegmentEditorAbstractEffect& object);
  ~qSlicerSegmentEditorAbstractEffectPrivate() override;
signals:
  // Signals that are used for effects to request operations from the editor
  // without having any dependency on the editor.
  void selectEffectSignal(QString);
  void updateVolumeSignal(void*,bool&);
  void saveStateForUndoSignal();
public:
  /// Segment editor parameter set node
  vtkWeakPointer<vtkMRMLSegmentEditorNode> ParameterSetNode;

  /// MRML scene
  vtkMRMLScene* Scene;

  /// Cursor to restore after custom cursor is not needed any more
  QCursor SavedCursor;

  /// Frame containing the effect options UI.
  /// Populating the frame is possible using the \sa addOptionsWidget method from the base classes
  QFrame* OptionsFrame;

  /// Aligned master volume is a copy of image in master volume node
  /// resampled into the reference image geometry of the segmentation.
  /// If the master volume geometry is the same as the reference image geometry
  /// then only a shallow copy is performed.
  vtkWeakPointer<vtkOrientedImageData> AlignedMasterVolume;

  /// Active labelmap for editing. Mainly needed because the segment binary labelmaps are shrunk
  /// to the smallest possible extent, but the user wants to draw on the whole master volume.
  /// It also allows modifying a segment by adding/removing regions (and performing inverse
  /// of that on all other segments).
  vtkWeakPointer<vtkOrientedImageData> ModifierLabelmap;

  /// Mask labelmap containing a merged silhouette of all the segments other than the selected one.
  /// Used if the paint over feature is turned off.
  vtkWeakPointer<vtkOrientedImageData> MaskLabelmap;

  /// SelectedSegmentLabelmap is a copy of the labelmap of the current segment
  /// resampled into the reference image geometry of the segmentation.
  vtkWeakPointer<vtkOrientedImageData> SelectedSegmentLabelmap;

  /// Image that holds current reference geometry information. Scalars are not allocated.
  /// Changing it does not change the reference geometry of the segment, it is just a copy,
  /// for convenience.
  vtkWeakPointer<vtkOrientedImageData> ReferenceGeometryImage;
};

#endif
