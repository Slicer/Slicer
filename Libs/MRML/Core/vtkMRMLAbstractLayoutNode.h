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
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#ifndef __vtkMRMLAbstractLayoutNode_h
#define __vtkMRMLAbstractLayoutNode_h

// MRML includes
#include "vtkMRMLNode.h"

class vtkXMLDataElement;

/// \brief Node that describes the view layout of the application.
///
/// When the scene is closing (vtkMRMLScene::Clear), the view arrangement is
/// set to none due to the Copy() call on an empty node.
class VTK_MRML_EXPORT vtkMRMLAbstractLayoutNode : public vtkMRMLNode
{
public:
  vtkTypeMacro(vtkMRMLAbstractLayoutNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

protected:
  vtkMRMLAbstractLayoutNode();
  ~vtkMRMLAbstractLayoutNode() override;

  vtkMRMLAbstractLayoutNode(const vtkMRMLAbstractLayoutNode&);
  void operator=(const vtkMRMLAbstractLayoutNode&);
};

#endif
