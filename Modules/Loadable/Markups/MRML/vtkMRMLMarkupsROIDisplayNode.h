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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/
// .NAME vtkMRMLMarkupsROIDisplayNode - MRML node to represent display properties for markups ROI
// .SECTION Description
// Adjusts default display parameters for ROI such as fill opacity.
//

#ifndef __vtkMRMLMarkupsROIDisplayNode_h
#define __vtkMRMLMarkupsROIDisplayNode_h

#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLMarkupsDisplayNode.h"

/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsROIDisplayNode : public vtkMRMLMarkupsDisplayNode
{
public:
  static vtkMRMLMarkupsROIDisplayNode *New();
  vtkTypeMacro ( vtkMRMLMarkupsROIDisplayNode,vtkMRMLMarkupsDisplayNode );

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance (  ) override;

  // Get node XML tag name (like Volume, Markups)
  const char* GetNodeTagName() override {return "MarkupsROIDisplay";};

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentDefaultMacro(vtkMRMLMarkupsROIDisplayNode);

  enum
  {
    ComponentROI = vtkMRMLMarkupsDisplayNode::Component_Last,
    ComponentROI_Last
  };

  // Scale handle indexes
  enum
  {
    HandleLFace,
    HandleRFace,
    HandlePFace,
    HandleAFace,
    HandleIFace,
    HandleSFace,

    HandleLPICorner,
    HandleRPICorner,
    HandleLAICorner,
    HandleRAICorner,
    HandleLPSCorner,
    HandleRPSCorner,
    HandleLASCorner,
    HandleRASCorner,

    HandleLPEdge,
    HandleRPEdge,
    HandleLAEdge,
    HandleRAEdge,
    HandleLIEdge,
    HandleRIEdge,
    HandleLSEdge,
    HandleRSEdge,
    HandlePIEdge,
    HandleAIEdge,
    HandlePSEdge,
    HandleASEdge,

    HandleROI_Last
  };

protected:
  vtkMRMLMarkupsROIDisplayNode();
  ~vtkMRMLMarkupsROIDisplayNode() override;
  vtkMRMLMarkupsROIDisplayNode( const vtkMRMLMarkupsROIDisplayNode& );
  void operator= ( const vtkMRMLMarkupsROIDisplayNode& );
};
#endif
