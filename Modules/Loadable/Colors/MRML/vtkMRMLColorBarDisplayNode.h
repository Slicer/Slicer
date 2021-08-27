/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLColorBarDisplayNode_h
#define __vtkMRMLColorBarDisplayNode_h

// MRML includes
#include <vtkMRMLDisplayNode.h>

class vtkMRMLColorTableNode;

class VTK_MRML_EXPORT vtkMRMLColorBarDisplayNode : public vtkMRMLDisplayNode
{
public:
  /// Color bar position preset on a view node
  enum PositionPresetType : int { Horizontal = 0, Vertical, PositionPreset_Last };

  static vtkMRMLColorBarDisplayNode *New();
  vtkTypeMacro(vtkMRMLColorBarDisplayNode,vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "ColorBarDisplay"; }

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLColorBarDisplayNode);

  /// Get displayable node
  vtkMRMLDisplayableNode* GetDisplayableNode();
  /// Set and observe displayable node
  void SetAndObserveDisplayableNode(vtkMRMLDisplayableNode* node);

  /// Get color table node
  vtkMRMLColorTableNode* GetColorTableNode();
  /// Set and observe color table node
  void SetAndObserveColorTableNode(vtkMRMLColorTableNode* node);

  vtkGetMacro(PositionPreset, PositionPresetType);
  vtkSetMacro(PositionPreset, PositionPresetType);

protected:
  vtkMRMLColorBarDisplayNode();
  ~vtkMRMLColorBarDisplayNode() override;
  vtkMRMLColorBarDisplayNode(const vtkMRMLColorBarDisplayNode&);
  void operator=(const vtkMRMLColorBarDisplayNode&);

  static const char* GetPositionPresetAsString(int id);
  static int GetPositionPresetFromString(const char* name);
  void SetPositionPreset(int id);

private:
  PositionPresetType PositionPreset;
};

#endif
