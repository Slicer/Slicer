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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __vtkFractionalLabelmapToClosedSurfaceConversionRule_h
#define __vtkFractionalLabelmapToClosedSurfaceConversionRule_h

// SegmentationCore includes
#include "vtkBinaryLabelmapToClosedSurfaceConversionRule.h"
#include "vtkSegmentationConverter.h"
#include "vtkSegmentationCoreConfigure.h"

/// \ingroup SegmentationCore
/// \brief Convert Fractional labelmap representation (vtkOrientedImageData type) to
///   closed surface representation (vtkPolyData type). The conversion algorithm
///   performs a marching cubes operation on the image data followed by an optional
///   decimation step.
class vtkSegmentationCore_EXPORT vtkFractionalLabelmapToClosedSurfaceConversionRule
  : public vtkBinaryLabelmapToClosedSurfaceConversionRule
{
public:
  /// Conversion parameter: magnification factor
  static const std::string GetFractionalLabelMapOversamplingFactorParameterName() { return "Fractional labelmap oversampling factor"; };
  static const std::string GetThresholdFractionParameterName() { return "Threshold fraction"; };

public:
  static vtkFractionalLabelmapToClosedSurfaceConversionRule* New();
  vtkTypeMacro(vtkFractionalLabelmapToClosedSurfaceConversionRule, vtkBinaryLabelmapToClosedSurfaceConversionRule);
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
  bool Convert(vtkSegment* segment) override;

  /// Get the cost of the conversion.
  unsigned int GetConversionCost(vtkDataObject* sourceRepresentation=nullptr, vtkDataObject* targetRepresentation=nullptr) override;

  /// Human-readable name of the converter rule
  const char* GetName() override { return "Fractional labelmap to closed surface"; };

  /// Human-readable name of the source representation
  const char* GetSourceRepresentationName() override { return vtkSegmentationConverter::GetSegmentationFractionalLabelmapRepresentationName(); };

  /// Human-readable name of the target representation
  const char* GetTargetRepresentationName() override { return vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(); };

protected:

  /// This function adds a border around the image that contains the paddingConstant value
  /// \param FractionalLabelMap The image that is being padded
  /// \param paddingConstant The value that is used to fill the new voxels
  void PadLabelmap(vtkOrientedImageData* fractionalLabelMap, double paddingConstant);

protected:
  vtkFractionalLabelmapToClosedSurfaceConversionRule();
  ~vtkFractionalLabelmapToClosedSurfaceConversionRule() override;

private:
  vtkFractionalLabelmapToClosedSurfaceConversionRule(const vtkFractionalLabelmapToClosedSurfaceConversionRule&) = delete;
  void operator=(const vtkFractionalLabelmapToClosedSurfaceConversionRule&) = delete;
};

#endif // __vtkFractionalLabelmapToClosedSurfaceConversionRule_h
