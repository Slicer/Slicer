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

#ifndef __vtkClosedSurfaceToBinaryLabelmapConversionRule_h
#define __vtkClosedSurfaceToBinaryLabelmapConversionRule_h

// SegmentationCore includes
#include "vtkSegmentationConverterRule.h"
#include "vtkSegmentationConverter.h"

#include "vtkSegmentationCoreConfigure.h"

class vtkPolyData;

/// \ingroup SegmentationCore
/// \brief Convert closed surface representation (vtkPolyData type) to binary
///   labelmap representation (vtkOrientedImageData type). The conversion algorithm
///   is based on image stencil.
class vtkSegmentationCore_EXPORT vtkClosedSurfaceToBinaryLabelmapConversionRule
  : public vtkSegmentationConverterRule
{
public:
  /// Conversion parameter: oversampling factor
  /// Determines the oversampling of the reference image geometry. If it's a number, then all segments
  /// are oversampled with the same value (value of 1 means no oversampling). If it has the value "A",
  /// then automatic oversampling is calculated.
  static const std::string GetOversamplingFactorParameterName() { return "Oversampling factor"; };
  static const std::string GetCropToReferenceImageGeometryParameterName() { return "Crop to reference image geometry"; };
  /// Determines if the output binary labelmaps should be reduced to as few shared labelmaps as possible after conversion.
  /// A value of 1 means that the labelmaps will be collapsed, while a value of 0 means that they will not be collapsed.
  static const std::string GetCollapseLabelmapsParameterName() { return "Collapse labelmaps"; };

public:
  static vtkClosedSurfaceToBinaryLabelmapConversionRule* New();
  vtkTypeMacro(vtkClosedSurfaceToBinaryLabelmapConversionRule, vtkSegmentationConverterRule);
  vtkSegmentationConverterRule* CreateRuleInstance() override;

  /// Constructs representation object from representation name for the supported representation classes
  /// (typically source and target representation VTK classes, subclasses of vtkDataObject)
  /// Note: Need to take ownership of the created object! For example using vtkSmartPointer<vtkDataObject>::Take
  vtkDataObject* ConstructRepresentationObjectByRepresentation(std::string representationName)  override;

  /// Constructs representation object from class name for the supported representation classes
  /// (typically source and target representation VTK classes, subclasses of vtkDataObject)
  /// Note: Need to take ownership of the created object! For example using vtkSmartPointer<vtkDataObject>::Take
  vtkDataObject* ConstructRepresentationObjectByClass(std::string className) override;

  /// Update the target representation based on the source representation
  bool Convert(vtkSegment* segment) override;

  /// Perform postprocesing steps on the output
  /// Collapses the segments to as few labelmaps as is possible
  bool PostConvert(vtkSegmentation* segmentation) override;

  /// Get the cost of the conversion.
  unsigned int GetConversionCost(vtkDataObject* sourceRepresentation=nullptr, vtkDataObject* targetRepresentation=nullptr) override;

  /// Human-readable name of the converter rule
  const char* GetName() override { return "Closed surface to binary labelmap (simple image stencil)"; };

  /// Human-readable name of the source representation
  const char* GetSourceRepresentationName() override { return vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(); };

  /// Human-readable name of the target representation
  const char* GetTargetRepresentationName() override { return vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(); };

  vtkSetMacro(UseOutputImageDataGeometry, bool);

protected:
  /// Calculate actual geometry of the output labelmap volume by verifying that the reference image geometry
  /// encompasses the input surface model, and extending it to the proper directions if necessary.
  /// \param closedSurfacePolyData Input closed surface poly data to convert
  /// \param geometryImageData Output image data containing output labelmap geometry. Memory is not allocated for voxel data.
  /// \return Success flag indicating sane calculated extents
  bool CalculateOutputGeometry(vtkPolyData* closedSurfacePolyData, vtkOrientedImageData* geometryImageData);

  /// Get default image geometry string in case of absence of parameter.
  /// The default geometry has identity directions and 1 mm uniform spacing,
  /// with origin and extent defined using the argument poly data.
  /// \param polyData Poly data defining the origin and extent of the default geometry
  /// \return Serialized image geometry for input poly data with identity directions and 1 mm spacing.
  std::string GetDefaultImageGeometryStringForPolyData(vtkPolyData* polyData);

protected:
  /// Flag determining whether to use the geometry of the given output oriented image data as is,
  /// or use the conversion parameters and the extent of the input surface. False by default,
  /// because pre-calculating the geometry of the output image data is not trivial and should be done
  /// only when there is a specific reason to do that (such as doing the conversion for sub-volumes and
  /// then stitching them back together).
  bool UseOutputImageDataGeometry{false};

protected:
  vtkClosedSurfaceToBinaryLabelmapConversionRule();
  ~vtkClosedSurfaceToBinaryLabelmapConversionRule() override;

private:
  vtkClosedSurfaceToBinaryLabelmapConversionRule(const vtkClosedSurfaceToBinaryLabelmapConversionRule&) = delete;
  void operator=(const vtkClosedSurfaceToBinaryLabelmapConversionRule&) = delete;
};

#endif // __vtkClosedSurfaceToBinaryLabelmapConversionRule_h
