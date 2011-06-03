/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumeRenderingLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerVolumeRenderingLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerVolumeRenderingLogic_h
#define __vtkSlicerVolumeRenderingLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLVolumePropertyNode.h>
#include <vtkMRMLAnnotationROINode.h>
#include "vtkMRMLVolumeRenderingDisplayNode.h"
#include "vtkMRMLVolumeRenderingScenarioNode.h"

// STD includes
#include <cstdlib>

#include "vtkSlicerVolumeRenderingModuleLogicExport.h"

class vtkMRMLScalarVolumeNode;
class vtkMRMLViewNode;
class vtkStringArray;

/// \ingroup Slicer_QtModules_VolumeRendering
class VTK_SLICER_VOLUMERENDERING_MODULE_LOGIC_EXPORT vtkSlicerVolumeRenderingLogic :
  public vtkSlicerModuleLogic
{
public:
  
  static vtkSlicerVolumeRenderingLogic *New();
  vtkTypeRevisionMacro(vtkSlicerVolumeRenderingLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Register local MRML nodes
  virtual void RegisterNodes();

  ///
  /// Update MRML events
  virtual void ProcessMRMLEvents(vtkObject * caller, unsigned long event, void * callData);
  
  ///
  /// Update logic events
  virtual void ProcessLogicEvents(vtkObject * caller, unsigned long event, void * callData);  
  
  //BTX
  using vtkMRMLAbstractLogic::ProcessLogicEvents; 
  //ETX

  // Description:
  // Create DisplayNode
  vtkMRMLVolumeRenderingDisplayNode* CreateVolumeRenderingDisplayNode();

  // Description:
  // Update DisplayNode from VolumeNode,
  // if needed create vtkMRMLVolumePropertyNode and vtkMRMLAnnotationROINode
  // and initialize them from VolumeNode
  void UpdateDisplayNodeFromVolumeNode(vtkMRMLVolumeRenderingDisplayNode *paramNode, 
                                          vtkMRMLVolumeNode *volumeNode, 
                                          vtkMRMLVolumePropertyNode **propNode,
                                          vtkMRMLAnnotationROINode **roiNode);
  void UpdateDisplayNodeFromVolumeNode(vtkMRMLVolumeRenderingDisplayNode *paramNode, 
                                          vtkMRMLVolumeNode *volumeNode)
    {
    vtkMRMLVolumePropertyNode *propNode = NULL;
    vtkMRMLAnnotationROINode            *roiNode = NULL;
    this->UpdateDisplayNodeFromVolumeNode(paramNode, volumeNode, &propNode, &roiNode);
    };

  // Description:
  // Create VolumeRenderingScenarioNode
  vtkMRMLVolumeRenderingScenarioNode* CreateScenarioNode();

  // Description:
  // Remove ViewNode from VolumeRenderingDisplayNode for a VolumeNode,
  void RemoveViewFromVolumeDisplayNodes(vtkMRMLVolumeNode *volumeNode, 
                                        vtkMRMLViewNode *viewNode);

  // Description:
  // Find volume rendering display node reference in the volume
  vtkMRMLVolumeRenderingDisplayNode* GetVolumeRenderingDisplayNodeByID(vtkMRMLVolumeNode *volumeNode, 
                                                                    char *displayNodeID);
  // Description:
  // Find volume rendering display node referencing the view node and volume node
  vtkMRMLVolumeRenderingDisplayNode* GetVolumeRenderingDisplayNodeForViewNode(
                                                        vtkMRMLVolumeNode *volumeNode, 
                                                        vtkMRMLViewNode *viewNode);
  // Description:
  // Find volume rendering display node referencing the view node in the scene
  vtkMRMLVolumeRenderingDisplayNode* GetVolumeRenderingDisplayNodeForViewNode(
                                                        vtkMRMLViewNode *viewNode);

  // Description:
  // Find first volume rendering display node
  vtkMRMLVolumeRenderingDisplayNode* GetFirstVolumeRenderingDisplayNode(vtkMRMLVolumeNode *volumeNode);

  // Description
  // Find the first volume rendering display node that uses the ROI
  vtkMRMLVolumeRenderingDisplayNode* GetFirstVolumeRenderingDisplayNodeByROINode(vtkMRMLAnnotationROINode* roiNode);

  void UpdateTranferFunctionRangeFromImage(vtkMRMLVolumeRenderingDisplayNode* vspNode);

  void UpdateFgTranferFunctionRangeFromImage(vtkMRMLVolumeRenderingDisplayNode* vspNode);

  void UpdateVolumePropertyFromDisplayNode(vtkMRMLVolumeRenderingDisplayNode* vspNode);

  void UpdateVolumePropertyFromImageData(vtkMRMLVolumeRenderingDisplayNode* vspNode);

  void SetupFgVolumePropertyFromImageData(vtkMRMLVolumeRenderingDisplayNode* vspNode);

  void FitROIToVolume(vtkMRMLVolumeRenderingDisplayNode* vspNode);

  vtkMRMLVolumePropertyNode* AddVolumePropertyFromFile (const char* filename);

protected:
  vtkSlicerVolumeRenderingLogic();
  virtual ~vtkSlicerVolumeRenderingLogic();
  
private:

  vtkSlicerVolumeRenderingLogic(const vtkSlicerVolumeRenderingLogic&); // Not implemented
  void operator=(const vtkSlicerVolumeRenderingLogic&);               // Not implemented
};

#endif

