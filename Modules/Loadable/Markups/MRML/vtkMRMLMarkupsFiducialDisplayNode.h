/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// .NAME vtkMRMLMarkupsFiducialDisplayNode - MRML node to represent display properties for markups fiducials
// .SECTION Description
// Currently, the only difference compared to the generic markups display node is that point labels
// are displayed by default.
//

#ifndef __vtkMRMLMarkupsFiducialDisplayNode_h
#define __vtkMRMLMarkupsFiducialDisplayNode_h

#include "vtkSlicerMarkupsModuleMRMLExport.h"

#include "vtkMRMLMarkupsDisplayNode.h"

class vtkMRMLProceduralColorNode;

/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsFiducialDisplayNode : public vtkMRMLMarkupsDisplayNode
{
public:
  static vtkMRMLMarkupsFiducialDisplayNode *New();
  vtkTypeMacro ( vtkMRMLMarkupsFiducialDisplayNode,vtkMRMLMarkupsDisplayNode );

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance (  ) override;

  // Get node XML tag name (like Volume, Markups)
  const char* GetNodeTagName() override {return "MarkupsFiducialDisplay";};

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentDefaultMacro(vtkMRMLMarkupsFiducialDisplayNode);

protected:
  vtkMRMLMarkupsFiducialDisplayNode();
  ~vtkMRMLMarkupsFiducialDisplayNode() override;
  vtkMRMLMarkupsFiducialDisplayNode( const vtkMRMLMarkupsFiducialDisplayNode& );
  void operator= ( const vtkMRMLMarkupsFiducialDisplayNode& );
};
#endif
