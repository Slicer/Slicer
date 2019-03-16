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

#ifndef __vtkBinaryLabelmapToClosedSurfaceConversionRule_h
#define __vtkBinaryLabelmapToClosedSurfaceConversionRule_h

// SegmentationCore includes
#include "vtkSegmentationConverterRule.h"
#include "vtkSegmentationConverter.h"

#include "vtkSegmentationCoreConfigure.h"

/// \ingroup SegmentationCore
/// \brief Convert binary labelmap representation (vtkOrientedImageData type) to
///   closed surface representation (vtkPolyData type). The conversion algorithm
///   performs a marching cubes operation on the image data followed by an optional
///   decimation step.
class vtkSegmentationCore_EXPORT vtkBinaryLabelmapToClosedSurfaceConversionRule
  : public vtkSegmentationConverterRule
{
public:
  /// Conversion parameter: decimation factor
  static const std::string GetDecimationFactorParameterName() { return "Decimation factor"; };
  /// Conversion parameter: smoothing factor
  static const std::string GetSmoothingFactorParameterName() { return "Smoothing factor"; };
  /// Conversion parameter: compute surface normals
  static const std::string GetComputeSurfaceNormalsParameterName() { return "Compute surface normals"; };

public:
  static vtkBinaryLabelmapToClosedSurfaceConversionRule* New();
  vtkTypeMacro(vtkBinaryLabelmapToClosedSurfaceConversionRule, vtkSegmentationConverterRule);
  vtkSegmentationConverterRule* CreateRuleInstance() override;

  /// Constructs representation object from representation name for the supported representation classes
  /// (typically source and target representation VTK classes, subclasses of vtkDataObject)
  /// Note: Need to take ownership of the created object! For example using vtkSmartPointer<vtkDataObject>::Take
  vtkDataObject* ConstructRepresentationObjectByRepresentation(std::string representationName) override;

  /// Constructs representation object from class name for the supported representation classes
  /// (typically source and target representation VTK classes, subclasses of vtkDataObject)
  /// Note: Need to take ownership of the created object! For example using vtkSmartPointer<vtkDataObject>::Take
  vtkDataObject* ConstructRepresentationObjectByClass(std::string className) override;

  /// Update the target representation based on the source representation
  bool Convert(vtkDataObject* sourceRepresentation, vtkDataObject* targetRepresentation) override;

  /// Get the cost of the conversion.
  unsigned int GetConversionCost(vtkDataObject* sourceRepresentation=nullptr, vtkDataObject* targetRepresentation=nullptr) override;

  /// Human-readable name of the converter rule
  const char* GetName()  override { return "Binary labelmap to closed surface"; };

  /// Human-readable name of the source representation
  const char* GetSourceRepresentationName() override { return vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(); };

  /// Human-readable name of the target representation
  const char* GetTargetRepresentationName() override { return vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(); };

protected:
  /// If input labelmap has non-background border voxels, then those regions remain open in the output closed surface.
  /// This function checks whether this is the case.
  bool IsLabelmapPaddingNecessary(vtkImageData* binaryLabelMap);

protected:
  vtkBinaryLabelmapToClosedSurfaceConversionRule();
  ~vtkBinaryLabelmapToClosedSurfaceConversionRule() override;
  void operator=(const vtkBinaryLabelmapToClosedSurfaceConversionRule&);
};

#endif // __vtkBinaryLabelmapToClosedSurfaceConversionRule_h
