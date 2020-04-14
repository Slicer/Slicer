/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#ifndef __vtkMRMLTableViewNode_h
#define __vtkMRMLTableViewNode_h

#include "vtkMRMLAbstractViewNode.h"

class vtkMRMLTableNode;

/// \brief MRML node to represent table view parameters.
///
/// TableViewNodes are associated one to one with a TableWidget.
class VTK_MRML_EXPORT vtkMRMLTableViewNode : public vtkMRMLAbstractViewNode
{
public:
  static vtkMRMLTableViewNode *New();
  vtkTypeMacro(vtkMRMLTableViewNode, vtkMRMLAbstractViewNode);
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
  const char* GetNodeTagName() override;

  ///
  /// Set the Table node id displayed in this Table View
  void SetTableNodeID(const char *);

  ///
  /// Get the Table node id displayed in this Table View
  const char * GetTableNodeID();

  ///
  /// Get the Table node displayed in this Table View
  vtkMRMLTableNode* GetTableNode();

  ///
  /// Configures the behavior of PropagateTableSelection().
  /// If DoPropagateTableSelection set to false then this
  /// view will not be affected by PropagateTableSelection.
  /// Default value is true.
  vtkSetMacro (DoPropagateTableSelection, bool );
  vtkGetMacro (DoPropagateTableSelection, bool );

  virtual const char* GetTableNodeReferenceRole();

protected:
  vtkMRMLTableViewNode();
  ~vtkMRMLTableViewNode() override;
  vtkMRMLTableViewNode(const vtkMRMLTableViewNode&);
  void operator=(const vtkMRMLTableViewNode&);

  virtual const char* GetTableNodeReferenceMRMLAttributeName();

  static const char* TableNodeReferenceRole;
  static const char* TableNodeReferenceMRMLAttributeName;

  bool DoPropagateTableSelection{true};
};

#endif
