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

#ifndef __vtkSegmentationConverterRule_h
#define __vtkSegmentationConverterRule_h

#include "vtkSegmentationCoreConfigure.h"

// VTK includes
#include <vtkObject.h>

// STD includes
#include <map>
#include <string>
#include <vector>

class vtkDataObject;
class vtkSegmentation;
class vtkSegment;

/// Helper macro for supporting cloning of rules
#ifndef vtkSegmentationConverterRuleNewMacro
#define vtkSegmentationConverterRuleNewMacro(newClass) \
  vtkStandardNewMacro(newClass); \
  vtkSegmentationConverterRule* newClass::CreateRuleInstance() \
  { \
    return newClass::New(); \
  }
#endif

/// \ingroup SegmentationCore
/// \brief Abstract converter rule class. Subclasses perform conversions between specific
///   representation types. They define source and target type and provide ways to create those
///   types of objects.
class vtkSegmentationCore_EXPORT vtkSegmentationConverterRule : public vtkObject
{
public:
  /// Conversion parameter list type. Maps the conversion parameter name to a pair consisting of the
  /// value of the parameter (the default value if it is defined in the converter rule) and the
  /// description of the parameter that appears as tooltip in the conversion parameters widget
  /// ( name => (value, description) )
  typedef std::map<std::string, std::pair<std::string, std::string> > ConversionParameterListType;

  /// Constant to use for converter rules with "infinite" computational cost (i.e. disabled)
  /// It's about UINT_MAX / 400 (allows us to have a few hundred disabled rules)
  static unsigned int GetConversionInfiniteCost() { return 10000000; };

public:
  //static vtkSegmentationConverterRule* New();
  vtkTypeMacro(vtkSegmentationConverterRule, vtkObject);

  /// Create instance of the default node. Similar to New but virtual method.
  /// Subclasses should implement this method by
  virtual vtkSegmentationConverterRule* CreateRuleInstance() = 0;

  /// Create a new instance of this rule and copy its contents
  virtual vtkSegmentationConverterRule* Clone();

  /// Constructs representation object from representation name for the supported representation classes
  /// (typically source and target representation VTK classes, subclasses of vtkDataObject)
  /// Note: Need to take ownership of the created object! For example using vtkSmartPointer<vtkDataObject>::Take
  virtual vtkDataObject* ConstructRepresentationObjectByRepresentation(std::string representationName) = 0;

  /// Constructs representation object from class name for the supported representation classes
  /// (typically source and target representation VTK classes, subclasses of vtkDataObject)
  /// Note: Need to take ownership of the created object! For example using vtkSmartPointer<vtkDataObject>::Take
  virtual vtkDataObject* ConstructRepresentationObjectByClass(std::string className) = 0;

  /// Perform pre-conversion steps across the specified segments in the segmentation
  /// This step should be unneccessary if only converting a single segment
  virtual bool PreConvert(vtkSegmentation* vtkNotUsed(segmentation)) { return true; };

  /// Update the target representation based on the source representation
  /// Initializes the target representation and calls ConvertInternal
  /// \sa ConvertInternal
  virtual bool Convert(vtkSegment* segment) = 0;

  /// Perform post-conversion steps across the specified segments in the segmentation
  /// This step should be unneccessary if only converting a single segment
  virtual bool PostConvert(vtkSegmentation* vtkNotUsed(segmentation)) { return true; };

  /// Get the cost of the conversion.
  /// \return Expected duration of the conversion in milliseconds. If the arguments are omitted, then a rough average can be
  ///   given just to indicate the relative computational cost of the algorithm. If the objects are given, then a more educated
  ///   guess can be made based on the object properties (dimensions, number of points, etc).
  virtual unsigned int GetConversionCost(vtkDataObject* sourceRepresentation=nullptr, vtkDataObject* targetRepresentation=nullptr)
    {
    (void)(sourceRepresentation); // unused
    (void)(targetRepresentation); // unused
    return 100;
    };

  /// Human-readable name of the converter rule
  virtual const char* GetName() = 0;

  /// Human-readable name of the source representation
  virtual const char* GetSourceRepresentationName() = 0;

  /// Human-readable name of the target representation
  virtual const char* GetTargetRepresentationName() = 0;

  /// Get rule conversion parameters for aggregated path parameters.
  /// Existing values in the map are overwritten, missing name&values are added.
  virtual void GetRuleConversionParameters(ConversionParameterListType& conversionParameters);

  /// Set a conversion parameter
  virtual void SetConversionParameter(const std::string& name, const std::string& value, const std::string& description="");

  /// Get a conversion parameter value
  virtual std::string GetConversionParameter(const std::string& name);

  /// Get a conversion parameter description
  virtual std::string GetConversionParameterDescription(const std::string& name);

  /// Determine if the rule has a parameter with a certain name
  bool HasConversionParameter(const std::string& name);

protected:
  /// Update the target representation based on the source representation
  virtual bool CreateTargetRepresentation(vtkSegment* segment);

  vtkSegmentationConverterRule();
  ~vtkSegmentationConverterRule() override;
  void operator=(const vtkSegmentationConverterRule&);

protected:
  /// Dictionary of conversion parameters in form of name -> default value, description.
  /// Each conversion rule defines its required/possible conversion parameters,
  /// and sets possible default values whenever applicable. Required parameters have empty defaults.
  /// When the user changes the parameter value, then the default is being overwritten to contain the
  /// custom value, but for new segmentations, it is initially the default.
  ConversionParameterListType ConversionParameters;

  /// Used when calling createTargetRepresentation
  /// If true, replaces the target representation of the segment with a new object, even if one already exists
  /// If false, will only create a target representation if one already doesn't exist.
  /// False by default.
  bool ReplaceTargetRepresentation{false};

  friend class vtkSegmentationConverter;
};

#endif // __vtkSegmentationConverterRule_h
