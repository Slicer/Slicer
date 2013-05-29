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
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------
  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  ///
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Unit";};

  ///
  /// Set/Get the quantity the unit belongs to. A unit can only
  /// have one quantity. Default is "".
  const char* GetQuantity();
  void SetQuantity(const char* quantity);

  ///
  /// Helper function that wrap the given string with the
  /// suffix and or the prefix. A space is used between the
  /// prefix and the value as well as between the value and the suffix.
  std::string WrapValueWithPrefix(std::string& value);
  std::string WrapValueWithSuffix(std::string& value);
  std::string WrapValueWithPrefixAndSuffix(std::string& value);

  ///
  /// Set the name of the unit. Since unit nodes are singleton,
  /// this name must be unique throughout the scene.
  virtual void SetName(const char* name);

  ///
  /// Set/Get the unit prefix.
  /// Default is "".
  // \sa SetSuffix(), GetSuffix()
  vtkGetStringMacro(Prefix);
  vtkSetStringMacro(Prefix);

  ///
  /// Set/Get the unit suffix. For example, the suffix for the unity
  /// meter would be "m".
  /// Default is "".
  /// \sa SetPrefix(), GetPrefix()
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

protected:
  vtkMRMLUnitNode();
  virtual ~vtkMRMLUnitNode();
  vtkMRMLUnitNode(const vtkMRMLUnitNode&);
  void operator=(const vtkMRMLUnitNode&);

  char* Prefix;
  char* Suffix;
  int Precision;
  double MinimumValue;
  double MaximumValue;
};

#endif
