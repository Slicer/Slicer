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

#ifndef __vtkMRMLCPURayCastVolumeRenderingDisplayNode_h
#define __vtkMRMLCPURayCastVolumeRenderingDisplayNode_h

// Volume Rendering includes
#include "vtkMRMLVolumeRenderingDisplayNode.h"

/// \ingroup Slicer_QtModules_VolumeRendering
/// \name vtkMRMLCPURayCastCPURayCastVolumeRenderingDisplayNode
/// \brief MRML node for storing information for CPU Raycast Volume Rendering
class VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT vtkMRMLCPURayCastVolumeRenderingDisplayNode
  : public vtkMRMLVolumeRenderingDisplayNode
{
public:
  static vtkMRMLCPURayCastVolumeRenderingDisplayNode *New();
  vtkTypeMacro(vtkMRMLCPURayCastVolumeRenderingDisplayNode,vtkMRMLVolumeRenderingDisplayNode);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "CPURayCastVolumeRendering";};

  vtkGetMacro (RaycastTechnique, int);
  vtkSetMacro (RaycastTechnique, int);

protected:
  vtkMRMLCPURayCastVolumeRenderingDisplayNode();
  ~vtkMRMLCPURayCastVolumeRenderingDisplayNode();
  vtkMRMLCPURayCastVolumeRenderingDisplayNode(const vtkMRMLCPURayCastVolumeRenderingDisplayNode&);
  void operator=(const vtkMRMLCPURayCastVolumeRenderingDisplayNode&);

  /* techniques in GPU ray cast
   * 0: composite with directional lighting (default)
   * 1: composite with fake lighting (edge coloring, faster)
   * 2: MIP
   * 3: MINIP
   * 4: Gradient Magnitude Opacity Modulation
   * 5: Illustrative Context Preserving Exploration
   * */
  int RaycastTechnique;
};

#endif

