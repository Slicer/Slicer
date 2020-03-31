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

#ifndef __vtkSegmentationConverter_h
#define __vtkSegmentationConverter_h

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>

// STD includes
#include <map>
#include <set>
#include <utility>
#include <vector>

// Segmentation includes
#include "vtkSegmentationCoreConfigure.h"

#include "vtkSegmentationConverterRule.h"

class vtkAbstractTransform;
class vtkSegment;
class vtkMatrix4x4;
class vtkImageData;
class vtkOrientedImageData;

/// \ingroup SegmentationCore
/// \brief Class that can convert between different representations of a segment.
class vtkSegmentationCore_EXPORT vtkSegmentationConverter : public vtkObject
{
public:
  typedef std::vector< vtkSmartPointer<vtkSegmentationConverterRule> > ConverterRulesListType;

  typedef std::vector<vtkSegmentationConverterRule*> ConversionPathType; // Contains a list of converter rule names
  typedef std::pair<ConversionPathType, unsigned int> ConversionPathAndCostType;
  typedef std::vector<ConversionPathAndCostType> ConversionPathAndCostListType;

  /// Default representation types
  /// In binary and fractional labelmaps values <=0 are considered background voxels (outside), values>0 are foreground (inside).
  static const char* GetSegmentationBinaryLabelmapRepresentationName()     { return "Binary labelmap"; };
  static const char* GetSegmentationFractionalLabelmapRepresentationName() { return "Fractional labelmap"; };
  static const char* GetSegmentationPlanarContourRepresentationName()      { return "Planar contour"; };
  static const char* GetSegmentationClosedSurfaceRepresentationName()      { return "Closed surface"; };
  static const char* GetBinaryLabelmapRepresentationName()     { return GetSegmentationBinaryLabelmapRepresentationName(); };
  static const char* GetFractionalLabelmapRepresentationName() { return GetSegmentationFractionalLabelmapRepresentationName(); };
  static const char* GetPlanarContourRepresentationName()      { return GetSegmentationPlanarContourRepresentationName(); };
  static const char* GetClosedSurfaceRepresentationName()      { return GetSegmentationClosedSurfaceRepresentationName(); };

  // Common conversion parameters
  // ----------------------------
  /// Reference image geometry conversion parameter
  /// Contains serialized matrix and extent
  static const std::string GetReferenceImageGeometryParameterName() { return "Reference image geometry"; };

  /// Field names for 2D display parameters
  static const char* GetScalarRangeFieldName() {return "ScalarRange";};
  static const char* GetThresholdValueFieldName() {return "ThresholdValue";};
  static const char* GetInterpolationTypeFieldName() {return "InterpolationType";};

public:
  static vtkSegmentationConverter* New();
  vtkTypeMacro(vtkSegmentationConverter, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Deep copy one converter into another
  virtual void DeepCopy(vtkSegmentationConverter* aConverter);

  /// Get all representations supported by the converter
  void GetAvailableRepresentationNames(std::set<std::string>& representationNames);

  /// Get all possible conversions between two representations
  void GetPossibleConversions(const std::string& sourceRepresentationName, const std::string& targetRepresentationName, ConversionPathAndCostListType &pathsCosts);

  /// Get all conversion parameters used by the selected conversion path
  void GetConversionParametersForPath(vtkSegmentationConverterRule::ConversionParameterListType& conversionParameters, const ConversionPathType& path);

  /// Get all conversion parameters in this converter. Aggregates all parameters from all rules
  void GetAllConversionParameters(vtkSegmentationConverterRule::ConversionParameterListType& conversionParameters);

  /// Set a list of conversion parameters to all rules (cannot change the description, only the value)
  void SetConversionParameters(vtkSegmentationConverterRule::ConversionParameterListType parameters);

  /// Set a conversion parameter to all rules having this parameter
  void SetConversionParameter(const std::string& name, const std::string& value, const std::string& description="");

  /// Get a conversion parameter value from first rule containing this parameter
  /// Note: all parameters with the same name should contain the same value
  std::string GetConversionParameter(const std::string& name);

  /// Get a conversion parameter description from first rule containing this parameter
  /// Note: all parameters with the same name should contain the same value
  std::string GetConversionParameterDescription(const std::string& description);

  /// Serialize all conversion parameters.
  /// The resulting string can be parsed in a segmentation converter object using /sa DeserializeConversionParameters
  std::string SerializeAllConversionParameters();

  /// Parse conversion parameters in string and set it to the converter
  /// Such a string can be constructed in a segmentation converter object using /sa SerializeAllConversionParameters
  void DeserializeConversionParameters(std::string conversionParametersString);

  /// Apply a transform on the reference image geometry
  /// Linear: simply multiply the geometry matrix with the applied matrix, extent stays the same
  /// Non-linear: calculate new extents and change only the extents
  void ApplyTransformOnReferenceImageGeometry(vtkAbstractTransform* transform);

// Utility functions
public:
  /// Return cheapest path from a list of paths with costs
  static ConversionPathType GetCheapestPath(const ConversionPathAndCostListType &pathsCosts);

  /// Utility function for serializing geometry of oriented image data
  static std::string SerializeImageGeometry(vtkOrientedImageData* orientedImageData);

  /// Utility function for serializing geometry of a complete geometry matrix and regular image data (providing only extent)
  static std::string SerializeImageGeometry(vtkMatrix4x4* geometryMatrix, vtkImageData* imageData);

  /// Utility function for serializing geometry of a complete geometry matrix and given extents
  static std::string SerializeImageGeometry(vtkMatrix4x4* geometryMatrix, int extent[6]);

  /// Utility function for de-serializing reference image geometry into a dummy oriented image data
  /// \param geometryString String containing the serialized image geometry
  /// \param orientedImageData Dummy oriented image data containing the de-serialized geometry information
  /// \param allocateScalars Reallocate scalar array to match the new geometry. Can be set to false if image buffer is not used (e.g., an image is only created to parse image geometry)
  /// \param scalarType If specified then scalar type will be set to the requested value, otherwise current value will be kept.
  /// \param numberOfScalarsComponents If specified then the number of scalar components will be set to the requested value, otherwise current value will be kept.
  /// \return Success flag
  static bool DeserializeImageGeometry(std::string geometryString, vtkOrientedImageData* orientedImageData, bool allocateScalars=true, int scalarType=VTK_VOID, int numberOfScalarsComponents=-1);

  /// Utility function for de-serializing reference image geometry into a matrix and a dimensions vector
  /// \param geometryString String containing the serialized image geometry
  /// \param geometryMatrix Matrix containing the de-serialized directions, scaling and origin
  /// \param extent Vector containing the de-serialized extent
  /// \return Success flag
  static bool DeserializeImageGeometry(std::string geometryString, vtkMatrix4x4* geometryMatrix, int extent[6]);

protected:
  /// Build a graph from ConverterRules list to facilitate faster finding of rules from a specific representation
  void RebuildRulesGraph();

  /// Find a transform path between the specified coordinate frames.
  /// \param sourceRepresentationName representation to convert from
  /// \param targetRepresentationName representation to convert to
  /// \param conversionList Stores the list of converter rules to get from source to the target
  ///   representation (if not found then returns with empty list). The caller should pass an
  ///   empty list (when the method is called recursively the list is not empty).
  /// \param skipRepresentations Representations that should be ignored (e.g., because they are
  ///   used already). The caller should pass an empty set (when the method is called recursively
  ///   the set is not empty).
  void FindPath(const std::string& sourceRepresentationName, const std::string& targetRepresentationName, ConversionPathAndCostListType &pathsCosts, std::set<std::string>& skipRepresentations);

protected:
  vtkSegmentationConverter();
  ~vtkSegmentationConverter() override;

protected:
  /// Converter rules. When the class is created it contains just the default converter rules but then
  /// rules may be customized with parameters and may store segment-specific information.
  /// Therefore, the rules should not be reused in other segments.
  ConverterRulesListType ConverterRules;

  /// For each "to" representation (first) stores a rule (second)
  typedef std::vector<vtkSegmentationConverterRule*> RulesListType;
  /// For each "from" representation (first) stores an array of rules (second)
  typedef std::map<std::string, RulesListType> RepresentationToRepresentationToRuleMapType;

  /// Source representation to target representation rule graph
  RepresentationToRepresentationToRuleMapType RulesGraph;

private:
  vtkSegmentationConverter(const vtkSegmentationConverter&) = delete;
  void operator=(const vtkSegmentationConverter&) = delete;
};

#endif // __vtkSegmentationConverter_h
