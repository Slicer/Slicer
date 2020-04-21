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

==============================================================================*/

#ifndef __vtkMRMLSequenceNode_h
#define __vtkMRMLSequenceNode_h

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLStorableNode.h>

// std includes
#include <deque>
#include <set>


/// \brief MRML node for representing a sequence of MRML nodes
///
/// This node contains a sequence of nodes (data nodes).
///
/// The data nodes can be referred to by an index, for example:
/// IndexName: time
/// IndexUnit: s
/// IndexNumeric: YES
/// IndexValue: 3.4567
///
/// If an index is numeric then it is sorted differently and equality determined using
/// a numerical tolerance instead of exact string matching.

class VTK_MRML_EXPORT vtkMRMLSequenceNode : public vtkMRMLStorableNode
{
public:
  static vtkMRMLSequenceNode *New();
  vtkTypeMacro(vtkMRMLSequenceNode,vtkMRMLStorableNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create instance of a sequence node
  vtkMRMLNode* CreateNodeInstance() override;

  /// Set node attributes from name/value pairs
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
/// Copy the node's attributes to this object
  void Copy(vtkMRMLNode* node) override;

  /// Copy sequence index information (index name, unit, type, values, etc)
  /// Does not copy data nodes.
  virtual void CopySequenceIndex(vtkMRMLNode *node);

  /// Update sequence index to point to nodes
  void UpdateSequenceIndex();

  /// Get unique node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "Sequence";};

  /// Set index name (example: time)
  void SetIndexName(const std::string& str);
  /// Get index name (example: time)
  vtkGetMacro(IndexName, std::string);

  /// Set unit for the index (example: s)
  void SetIndexUnit(const std::string& str);
  /// Get unit for the index (example: s)
  vtkGetMacro(IndexUnit, std::string);

  /// Set the type of the index (numeric, text, ...)
  void SetIndexType(int indexType);
  void SetIndexTypeFromString(const char *indexTypeString);
  /// Get the type of the index (numeric, text, ...)
  vtkGetMacro(IndexType, int);
  virtual std::string GetIndexTypeAsString();

  /// Get tolerance value for comparing numerix index values. If index values differ by less than the tolerance
  /// then the index values considered to be equal.
  vtkGetMacro(NumericIndexValueTolerance, double);
  /// Set tolerance value for comparing numerix index values.
  void SetNumericIndexValueTolerance(double tolerance);

  /// Helper functions for converting between string and code representation of the index type
  static std::string GetIndexTypeAsString(int indexType);
  static int GetIndexTypeFromString(const std::string &indexTypeString);

  /// Add a copy of the provided node to this sequence as a data node.
  /// If a sequence item is not found by that index, a new item is added.
  /// Always performs deep-copy.
  /// Returns the data node copy that has just been created.
  vtkMRMLNode* SetDataNodeAtValue(vtkMRMLNode* node, const std::string& indexValue);

  /// Update an existing data node.
  /// Return true if a data node was found by that index.
  bool UpdateDataNodeAtValue(vtkMRMLNode* node, const std::string& indexValue, bool shallowCopy = false);

  /// Remove data node corresponding to the specified index
  void RemoveDataNodeAtValue(const std::string& indexValue);

  /// Remove all data nodes from the sequence
  void RemoveAllDataNodes();

  /// Get the node corresponding to the specified index value
  /// If exact match is not required and index is numeric then the best matching data node is returned.
  vtkMRMLNode* GetDataNodeAtValue(const std::string& indexValue, bool exactMatchRequired = true);

  /// Get the data node corresponding to the n-th index value
  vtkMRMLNode* GetNthDataNode(int itemNumber);

  /// Index value of n-th data node.
  std::string GetNthIndexValue(int itemNumber);

  /// If exact match is not required and index is numeric then the best matching data node is returned.
  /// If the sequences has numeric index, uses data node just before the index value in the case of non-exact match
  int GetItemNumberFromIndexValue(const std::string& indexValue, bool exactMatchRequired = true);

  /// Change index value of an existing data node.
  bool UpdateIndexValue(const std::string& oldIndexValue, const std::string& newIndexValue);

  /// Return the number of nodes stored in this sequence.
  int GetNumberOfDataNodes();

  /// Return the class name of the data nodes (e.g., vtkMRMLTransformNode). If there are no data nodes yet then it returns empty string.
  std::string GetDataNodeClassName();

  /// Return the human-readable type name of the data nodes (e.g., TransformNode). If there are no data nodes yet then it returns the string "undefined".
  std::string GetDataNodeTagName();

  /// Return the internal scene that stores all the data nodes.
  /// If autoCreate is enabled then the sequence scene is created
  /// (if it has not been created already).
  vtkMRMLScene* GetSequenceScene(bool autoCreate=true);

  /// Create default storage node. Uses vtkMRMLSequenceStorageNode unless the data node
  /// requests a more specific storage node class.
  vtkMRMLStorageNode* CreateDefaultStorageNode() override;

  /// Returns vtkMRMLVolumeSequenceStorageNode or vtkMRMLLinearTransformSequenceStorageNode if applicable
  /// (sequence contains volumes with the same type and geometry; or a list of linear transforms)
  /// and generic vtkMRMLSequenceStorageNode otherwise.
  std::string GetDefaultStorageNodeClassName(const char* filename = nullptr) override;

  /// Update node IDs in case of node ID conflicts on scene import
  void UpdateScene(vtkMRMLScene *scene) override;

  /// Type of the index. Controls the behavior of sorting, finding, etc.
  /// Additional types may be added in the future, such as tag cloud, two-dimensional index, ...
  enum IndexTypes
    {
    NumericIndex = 0,
    TextIndex,
    NumberOfIndexTypes // this line must be the last one
    };

protected:
  vtkMRMLSequenceNode();
  ~vtkMRMLSequenceNode() override;
  vtkMRMLSequenceNode(const vtkMRMLSequenceNode&);
  void operator=(const vtkMRMLSequenceNode&);

  /// Get the index where an item would need to be inserted to.
  /// If numeric index then insert it by respecting sorting order, otherwise insert to the end.
  int GetInsertPosition(const std::string& indexValue);

  void ReadIndexValues(const std::string& indexText);

  vtkMRMLNode* DeepCopyNodeToScene(vtkMRMLNode* source, vtkMRMLScene* scene);

  struct IndexEntryType
    {
    std::string IndexValue;
    vtkMRMLNode* DataNode;
    std::string DataNodeID; // only used temporarily, during scene load
    };

protected:

  /// Describes index of the sequence node
  std::string IndexName;
  std::string IndexUnit;
  int IndexType{vtkMRMLSequenceNode::NumericIndex};
  double NumericIndexValueTolerance{0.001};

  /// Need to store the nodes in the scene, because for reading/writing nodes
  /// we need MRML storage nodes, which only work if they refer to a data node in the same scene
  vtkMRMLScene* SequenceScene{0};

  /// List of data items (the scene may contain some more nodes, such as storage nodes)
  std::deque< IndexEntryType > IndexEntries;
};

#endif
