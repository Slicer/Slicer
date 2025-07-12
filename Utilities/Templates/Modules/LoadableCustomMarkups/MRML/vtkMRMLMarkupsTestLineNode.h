/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

==============================================================================*/

#ifndef __vtkMRMLMarkupsTestLineNode_h
#define __vtkMRMLMarkupsTestLineNode_h

#include <vtkMRMLMarkupsLineNode.h>

#include "vtkSlicerTemplateKeyModuleMRMLExport.h"

//-----------------------------------------------------------------------------
class VTK_SLICER_TEMPLATEKEY_MODULE_MRML_EXPORT vtkMRMLMarkupsTestLineNode : public vtkMRMLMarkupsLineNode
{
public:
  static vtkMRMLMarkupsTestLineNode* New();
  vtkTypeMacro(vtkMRMLMarkupsTestLineNode, vtkMRMLMarkupsLineNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------------
  const char* GetIcon() override { return ":/Icons/MarkupsGeneric.png"; }
  const char* GetAddIcon() override { return ":/Icons/MarkupsGenericMouseModePlace.png"; }
  const char* GetPlaceAddIcon() override { return ":/Icons/MarkupsGenericMouseModePlaceAdd.png"; }

  vtkMRMLNode* CreateNodeInstance() override;
  /// Get node XML tag name (like Volume, Model)
  ///
  const char* GetNodeTagName() override { return "MarkupsTestLine"; }

  /// Get markup type internal name
  const char* GetMarkupType() override { return "TestLine"; }

  // Get markup type GUI display name
  const char* GetTypeDisplayName() override { return "Test Line"; };

  /// Get markup short name
  const char* GetDefaultNodeNamePrefix() override { return "SC"; }

  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentDefaultMacro(vtkMRMLMarkupsTestLineNode);

  vtkPolyData* GetTargetOrgan() const { return this->TargetOrgan; }
  void SetTargetOrgan(vtkPolyData* targetOrgan) { this->TargetOrgan = targetOrgan; }

protected:
  vtkMRMLMarkupsTestLineNode();
  ~vtkMRMLMarkupsTestLineNode() override;
  vtkMRMLMarkupsTestLineNode(const vtkMRMLMarkupsTestLineNode&);
  void operator=(const vtkMRMLMarkupsTestLineNode&);

private:
  vtkPolyData* TargetOrgan = nullptr;
};

#endif
