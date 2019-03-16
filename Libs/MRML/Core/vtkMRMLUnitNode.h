/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLUnitNode_h
#define __vtkMRMLUnitNode_h

// MRML includes
#include "vtkMRMLNode.h"

/// \brief Node that holds the information about a unit.
///
/// A unit node holds all the information regarding a given unit.
/// A unit belongs to a quantity. A quantity can have multiple different
/// units. For example, the units meter and millimeter belong to the quantity
/// "length". Units are singleton.
class VTK_MRML_EXPORT vtkMRMLUnitNode : public vtkMRMLNode
{
public:
  static vtkMRMLUnitNode *New();
  vtkTypeMacro(vtkMRMLUnitNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------
  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "Unit";}

  /// Reimplemented to prevent reset if unit node is a singleton.
  void Reset(vtkMRMLNode* defaultNode) override;

  ///
  /// Set/Get the quantity the unit belongs to. A unit can only
  /// have one quantity. Default is "".
  const char* GetQuantity();
  void SetQuantity(const char* quantity);

  /// Return the value multiplied by DisplayCoefficient and summed by
  /// DisplayOffset.
  /// \sa GetValueFromDisplayValue(), GetDisplayStringFromValue(),
  /// GetDisplayCoefficient(), GetDisplayOffset()
  virtual double GetDisplayValueFromValue(double value);
  /// Return the value subtracted from DisplayOffset and divided by
  /// DisplayCoefficient.
  /// \sa GetDisplayValueFromValue(), GetDisplayStringFromValue()
  virtual double GetValueFromDisplayValue(double value);
  /// Return the display value with prefix and suffix.
  /// \sa GetDisplayValueFromValue(), GetValueFromDisplayValue()
  const char* GetDisplayStringFromValue(double value);

  /// Return the display string format to use with printf/sprintf.
  /// Note that the value passed to the format must be the DisplayValue.
  /// For example: "%#6.3g mm" if the precision is 3 and the suffix mm.
  /// \sa GetDisplayValueFromValue(), GetDisplayStringFromValue()
  const char* GetDisplayStringFormat();

  ///
  /// Set the name of the unit. Since unit nodes are singleton,
  /// this name must be unique throughout the scene.
  void SetName(const char* name) override;

  ///
  /// Set/Get the unit prefix.
  /// Default is "".
  /// \sa SetPrefix(), GetPrefix()
  vtkGetStringMacro(Prefix);
  vtkSetStringMacro(Prefix);

  ///
  /// Set/Get the unit suffix. For example, the suffix for the unity
  /// meter would be "m".
  /// Default is "".
  /// \sa SetSuffix(), GetSuffix()
  vtkGetStringMacro(Suffix);
  vtkSetStringMacro(Suffix);

  ///
  /// Set/Get the precision (i.e. number of decimals) of the unit.
  /// Default is 3.
  vtkGetMacro(Precision, int);
  vtkSetClampMacro(Precision, int, 0, VTK_INT_MAX);

  ///
  /// Set/Get the minimum value that can be attributed to the unit.
  /// For example, the minimum value for Kelvins should be 0.
  /// \sa SetMaximumValue(), GetMaximumValue()
  vtkGetMacro(MinimumValue, double);
  vtkSetMacro(MinimumValue, double);

  ///
  /// Set/Get the maximum value that can be attributed to the unit.
  /// For example, the maximum value for a speed should (probably) be 3e6.
  /// \sa SetMinimumValue(), GetMinimumValue()
  vtkGetMacro(MaximumValue, double);
  vtkSetMacro(MaximumValue, double);

  ///
  /// Multiply the value with DisplayCoefficient for display.
  /// \sa GetDisplayOffset(), GetDisplayValueFromValue()
  vtkGetMacro(DisplayCoefficient, double);
  vtkSetMacro(DisplayCoefficient, double);

  ///
  /// Addition the value with DisplayOffset for display.
  /// \sa GetDisplayCoefficient(), GetDisplayValueFromValue()
  vtkGetMacro(DisplayOffset, double);
  vtkSetMacro(DisplayOffset, double);

protected:
  vtkMRMLUnitNode();
  ~vtkMRMLUnitNode() override;
  vtkMRMLUnitNode(const vtkMRMLUnitNode&);
  void operator=(const vtkMRMLUnitNode&);

  virtual const char* GetDisplayValueStringFromDisplayValue(double displayValue);
  virtual const char* GetDisplayStringFromDisplayValueString(const char* displayValue);
  ///
  /// Helper functions that wrap the given string with the
  /// suffix and or the prefix. A space is used between the
  /// prefix and the value as well as between the value and the suffix.
  std::string WrapValueWithPrefix(const std::string& value) const;
  std::string WrapValueWithSuffix(const std::string& value) const;
  std::string WrapValueWithPrefixAndSuffix(const std::string& value) const;

  char* Prefix;
  char* Suffix;
  int Precision;
  double MinimumValue;
  double MaximumValue;

  double DisplayCoefficient;
  double DisplayOffset;

  std::string LastValueString;
  std::string LastDisplayString;
};

#endif
