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

#ifndef __qSlicerSegmentEditorAbstractLabelEffect_h
#define __qSlicerSegmentEditorAbstractLabelEffect_h

// Segmentations Editor Effects includes
#include "qSlicerSegmentationsEditorEffectsExport.h"

#include "qSlicerSegmentEditorAbstractEffect.h"

class qSlicerSegmentEditorAbstractLabelEffectPrivate;

class vtkMatrix4x4;
class vtkOrientedImageData;
class vtkPolyData;
class vtkMRMLVolumeNode;
class vtkMRMLSegmentationNode;

/// \ingroup SlicerRt_QtModules_Segmentations
/// \brief Base class for all "label" effects.
///
/// This base class provides common GUI and MRML for the options PaintOver and Threshold.
class Q_SLICER_SEGMENTATIONS_EFFECTS_EXPORT qSlicerSegmentEditorAbstractLabelEffect :
  public qSlicerSegmentEditorAbstractEffect
{
public:
  Q_OBJECT

public:
  typedef qSlicerSegmentEditorAbstractEffect Superclass;
  qSlicerSegmentEditorAbstractLabelEffect(QObject* parent = nullptr);
  ~qSlicerSegmentEditorAbstractLabelEffect() override;

public:
  /// Clone editor effect
  /// (redefinition of pure virtual function to allow python wrapper to identify this as abstract class)
  qSlicerSegmentEditorAbstractEffect* clone() override = 0;

  /// Create options frame widgets, make connections, and add them to the main options frame using \sa addOptionsWidget
  void setupOptionsFrame() override;

  /// Set default parameters in the parameter MRML node
  void setMRMLDefaults() override;

  /// Perform actions needed on reference geometry change
  void referenceGeometryChanged() override;

  /// Perform actions needed on master volume change
  void masterVolumeNodeChanged() override;

public slots:
  /// Update user interface from parameter set node
  void updateGUIFromMRML() override;

  /// Update parameter set node from user interface
  void updateMRMLFromGUI() override;

// Utility functions
public:

  /// Rasterize a poly data onto the input image into the slice view
  Q_INVOKABLE static void appendPolyMask(vtkOrientedImageData* input, vtkPolyData* polyData, qMRMLSliceWidget* sliceWidget, vtkMRMLSegmentationNode* segmentationNode=nullptr);

  /// Create a slice view screen space (2D) mask image for the given polydata
  Q_INVOKABLE static void createMaskImageFromPolyData(vtkPolyData* polyData, vtkOrientedImageData* outputMask, qMRMLSliceWidget* sliceWidget);

  /// Append image onto image. Resamples appended image and saves result in input image
  Q_INVOKABLE static void appendImage(vtkOrientedImageData* inputImage, vtkOrientedImageData* appendedImage);

  /// Return matrix for volume node that takes into account the IJKToRAS
  /// and any linear transforms that have been applied
  Q_INVOKABLE static void imageToWorldMatrix(vtkMRMLVolumeNode* node, vtkMatrix4x4* ijkToRas);

  /// Return matrix for oriented image data that takes into account the image to world
  /// and any linear transforms that have been applied on the given segmentation
  Q_INVOKABLE static void imageToWorldMatrix(vtkOrientedImageData* image, vtkMRMLSegmentationNode* node, vtkMatrix4x4* ijkToRas);

protected:
  QScopedPointer<qSlicerSegmentEditorAbstractLabelEffectPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSegmentEditorAbstractLabelEffect);
  Q_DISABLE_COPY(qSlicerSegmentEditorAbstractLabelEffect);
};

#endif
