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

#ifndef __vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode_h
#define __vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode_h

// Volume Rendering includes
#include "vtkMRMLVolumeRenderingDisplayNode.h"

/// \ingroup Slicer_QtModules_VolumeRendering
/// \name vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode
/// \brief MRML node for storing information for NCI MultiVolume Raycast Volume Rendering
class VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode
  : public vtkMRMLVolumeRenderingDisplayNode
{
public:
  static vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode *New();
  vtkTypeMacro(vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode,vtkMRMLVolumeRenderingDisplayNode);
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
  virtual const char* GetNodeTagName() {return "NCIMultiVolumeRayCastVolumeRendering";};

  // Description:
  // Mark the fg volume and fg volume property nodes as references.
  virtual void SetSceneReferences();

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // Updates this node if it depends on other nodes
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Observe the reference transform node
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // the ID of a MRMLVolumeNode (fg volume)
  vtkGetStringMacro (FgVolumeNodeID);
  void SetAndObserveFgVolumeNodeID(const char *volumeNodeID);

  // Description:
  // Associated transform MRML node (fg volume)
  vtkMRMLVolumeNode* GetFgVolumeNode();

  // Description:
  // the ID of a parameter MRMLVolumePropertyNode (fg volume)
  vtkGetStringMacro (FgVolumePropertyNodeID);
  void SetAndObserveFgVolumePropertyNodeID(const char *volumePropertyNodeID);

  // Description:
  // Associated transform MRML node (fg volume)
  vtkMRMLVolumePropertyNode* GetFgVolumePropertyNode();

  vtkGetMacro (DepthPeelingThreshold, float);
  vtkSetMacro (DepthPeelingThreshold, float);

  vtkGetMacro (RaycastTechnique, int);
  vtkSetMacro (RaycastTechnique, int);

  vtkGetMacro (RaycastTechniqueFg, int);
  vtkSetMacro (RaycastTechniqueFg, int);

  vtkSetMacro(UseFgThreshold,int);
  vtkGetMacro(UseFgThreshold,int);
  vtkBooleanMacro(UseFgThreshold,int);

  enum FusionType
  {
    AlphaBlendingOR = 0, // (default)
    AlphaBlendingAND,
    AlphaBlendingNOT
  };
  vtkGetMacro (MultiVolumeFusionMethod, int);
  vtkSetMacro (MultiVolumeFusionMethod, int);

  vtkSetVector2Macro(ThresholdFg, double);
  vtkGetVectorMacro(ThresholdFg, double, 2);

  vtkGetMacro (BgFgRatio, float);
  vtkSetMacro (BgFgRatio, float);

  vtkSetVector2Macro(WindowLevelFg, double);
  vtkGetVectorMacro(WindowLevelFg, double, 2);

protected:
  vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode();
  ~vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode();
  vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode(const vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode&);
  void operator=(const vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode&);

  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData);

  char *FgVolumeNodeID;
  virtual void SetFgVolumeNodeID(const char* arg);
  vtkMRMLVolumeNode* FgVolumeNode;

  char *FgVolumePropertyNodeID;
  virtual void SetFgVolumePropertyNodeID(const char* arg);
  vtkMRMLVolumePropertyNode* FgVolumePropertyNode;

  float DepthPeelingThreshold;

  /* techniques in GPU ray cast
   * 0: composite with directional lighting (default)
   * 1: composite with fake lighting (edge coloring, faster)
   * 2: MIP
   * 3: MINIP
   * 4: Gradient Magnitude Opacity Modulation
   * 5: Illustrative Context Preserving Exploration
   * */
  int RaycastTechnique;
  int RaycastTechniqueFg;

  /*
   * fusion method in GPU ray cast II
   * 0: Alpha Blending OR (default)
   * 1: Alpha Blending AND
   * 2: Alpha Blending NOT
   * */
  int MultiVolumeFusionMethod;

  double ThresholdFg[2];
  int UseFgThreshold;

  float BgFgRatio;

  //fg volume window & level
  double WindowLevelFg[2];
};

#endif

