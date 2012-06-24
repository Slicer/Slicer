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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLNCIRayCastVolumeRenderingDisplayNode_h
#define __vtkMRMLNCIRayCastVolumeRenderingDisplayNode_h

// Volume Rendering includes
#include "vtkMRMLVolumeRenderingDisplayNode.h"

/// \ingroup Slicer_QtModules_VolumeRendering
/// \name vtkMRMLNCIRayCastVolumeRenderingDisplayNode
/// \brief MRML node for storing information for NCI Raycast Volume Rendering
class VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT vtkMRMLNCIRayCastVolumeRenderingDisplayNode
  : public vtkMRMLVolumeRenderingDisplayNode
{
public:
  static vtkMRMLNCIRayCastVolumeRenderingDisplayNode *New();
  vtkTypeMacro(vtkMRMLNCIRayCastVolumeRenderingDisplayNode,vtkMRMLVolumeRenderingDisplayNode);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "NCIRayCastVolumeRendering";};

  vtkGetMacro (DepthPeelingThreshold, float);
  vtkSetMacro (DepthPeelingThreshold, float);

  vtkGetMacro (DistanceColorBlending, float);
  vtkSetMacro (DistanceColorBlending, float);

  vtkGetMacro (ICPEScale, float);
  vtkSetMacro (ICPEScale, float);

  vtkGetMacro (ICPESmoothness, float);
  vtkSetMacro (ICPESmoothness, float);

  vtkGetMacro (RaycastTechnique, int);
  vtkSetMacro (RaycastTechnique, int);

protected:
  vtkMRMLNCIRayCastVolumeRenderingDisplayNode();
  ~vtkMRMLNCIRayCastVolumeRenderingDisplayNode();
  vtkMRMLNCIRayCastVolumeRenderingDisplayNode(const vtkMRMLNCIRayCastVolumeRenderingDisplayNode&);
  void operator=(const vtkMRMLNCIRayCastVolumeRenderingDisplayNode&);

  float DepthPeelingThreshold;
  float DistanceColorBlending;

  float ICPEScale;
  float ICPESmoothness;

  int RaycastTechnique;
};

#endif

