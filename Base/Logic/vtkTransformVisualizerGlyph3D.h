/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/

#ifndef __vtkTransformVisualizerGlyph3D_h
#define __vtkTransformVisualizerGlyph3D_h

#include "vtkSlicerBaseLogic.h"

#include "vtkGlyph3D.h"

/// \brief Specialized glyph3d filter optimized for visualizing transforms
///
/// This class extends the vtkGlyph3D filter with the following features:
/// - anisotropic scaling of glyphs: e.g., only the length of an arrow
///   is scaled with the vector magnitude
/// - use a different scalar for scaling and coloring
/// - generate glyphs only if the corresponding scalar is in specified range
/// - simplified, optimized generation of glyphs
///
/// Supported options:
/// - Scaling: VTK_SCALE_BY_SCALAR and VTK_SCALE_BY_VECTOR
/// - Color: only by scalar (set by SetColorArray)
/// - Vector: only VTK_USE_VECTOR (oriented and scaled by active vector)
/// - Indexing: not supported
///
class VTK_SLICER_BASE_LOGIC_EXPORT vtkTransformVisualizerGlyph3D : public vtkGlyph3D
{
public:
  vtkTypeMacro(vtkTransformVisualizerGlyph3D,vtkGlyph3D);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkTransformVisualizerGlyph3D *New();

  /// If true then glyphs are only scaled with the scalar value along x axis (y and z scaling is 1).
  /// If false then glyphs are scaled uniformly with the scalar value.
  vtkSetMacro(ScaleDirectional,bool);
  vtkGetMacro(ScaleDirectional,bool);

  /// Set the array used for orienting (and optionally scaling) the glyphs.
  /// Default: active vector array.
  void SetVectorArray(const char* vectorArrayName);

  /// Set the array used for (optionally) scaling the glyphs.
  /// Default: active scalar array.
  void SetScalarArray(const char* scalarArrayName);

  /// Set the array used for coloring the glyphs.
  /// Default: active scalar array.
  void SetColorArray(const char* colorArrayName);

  /// If true then points with scalar value outside the lower or upper threshold are ignored
  vtkSetMacro(MagnitudeThresholding,bool);
  vtkGetMacro(MagnitudeThresholding,bool);

  /// Points with scalar value below this value are ignored (if thresholding is enabled)
  vtkSetMacro(MagnitudeThresholdLower,double);
  vtkGetMacro(MagnitudeThresholdLower,double);

  /// Points with scalar value above this value are ignored (if thresholding is enabled)
  vtkSetMacro(MagnitudeThresholdUpper,double);
  vtkGetMacro(MagnitudeThresholdUpper,double);

protected:
  vtkTransformVisualizerGlyph3D();
  ~vtkTransformVisualizerGlyph3D() override  = default;

  bool ScaleDirectional;

  bool MagnitudeThresholding;
  double MagnitudeThresholdLower;
  double MagnitudeThresholdUpper;

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

private:
  vtkTransformVisualizerGlyph3D(const vtkTransformVisualizerGlyph3D&) = delete;
  void operator=(const vtkTransformVisualizerGlyph3D&) = delete;
};

#endif
