/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumesLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerVolumesLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerVolumesLogic_h
#define __vtkSlicerVolumesLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLVolumeNode.h"

// STD includes
#include <stdlib.h>

#include "vtkSlicerVolumesModuleLogicExport.h"

class vtkMRMLScalarVolumeNode;
class vtkMRMLScalarVolumeDisplayNode;
class vtkMRMLVolumeHeaderlessStorageNode;
class vtkStringArray;

class VTK_SLICER_VOLUMES_MODULE_LOGIC_EXPORT vtkSlicerVolumesLogic :
  public vtkSlicerModuleLogic
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerVolumesLogic *New();
  vtkTypeRevisionMacro(vtkSlicerVolumesLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The currently active mrml volume node 
  vtkGetObjectMacro (ActiveVolumeNode, vtkMRMLVolumeNode);
  void SetActiveVolumeNode (vtkMRMLVolumeNode *ActiveVolumeNode);

  // Description:
  // Sub type of loading an archetype volume that is known to be a scalar
  vtkMRMLScalarVolumeNode* AddArchetypeScalarVolume (const char *filename, const char* volname, int loadingOptions);

  vtkMRMLScalarVolumeNode* AddArchetypeScalarVolume (const char *filename, const char* volname) 
    {
    return this->AddArchetypeScalarVolume( filename, volname, 0);
    };

  // Description:
  // Overloaded function of AddArchetypeVolume to provide more 
  // loading options, where variable loadingOptions is bit-coded as following:
  // bit 0: label map
  // bit 1: centered
  // bit 2: loading single file
  // higher bits are reserved for future use
  vtkMRMLVolumeNode* AddArchetypeVolume (const char* filename, const char* volname, int loadingOptions) 
    {
    return (this->AddArchetypeVolume( filename, volname, loadingOptions, NULL));
    };
  vtkMRMLVolumeNode* AddArchetypeVolume (const char* filename, const char* volname, int loadingOptions, vtkStringArray *fileList);
  vtkMRMLVolumeNode* AddArchetypeVolume (const char *filename, const char* volname) 
    {
    return this->AddArchetypeVolume( filename, volname, 0, NULL);
    };

  // Description:
  // Create new mrml node and associated storage node.
  // Read image data from a specified file
  // vtkMRMLVolumeNode* AddArchetypeVolume (const char* filename, int centerImage, int labelMap, const char* volname);

  // Description:
  // Overloaded function of AddHeaderVolume to provide more 
  // loading options, where variable loadingOptions is bit-coded as following:
  // bit 0: label map
  // bit 1: centered
  // bit 2: loading signal file
  // higher bits are reserved for future use
  vtkMRMLVolumeNode* AddHeaderVolume (const char* filename, const char* volname, 
                                      vtkMRMLVolumeHeaderlessStorageNode *headerStorage,
                                      int loadingOptions);

  // Description:
  // Create new mrml node and associated storage node.
  // Read image data from a specified file
  // vtkMRMLVolumeNode* AddHeaderVolume (const char* filename, int centerImage, int labelMap, const char* volname, 
  //                                    vtkMRMLVolumeHeaderlessStorageNode *headerStorage);

  // Description:
  // Write volume's image data to a specified file
  int SaveArchetypeVolume (const char* filename, vtkMRMLVolumeNode *volumeNode);

  // Description:
  // Create a label map volume to match the given volume node and add it to
  // the scene
  vtkMRMLScalarVolumeNode *CreateLabelVolume (vtkMRMLScene *scene, vtkMRMLVolumeNode *volumeNode, const char *name);

  // Description:
  // Fill in a label map volume to match the given template volume node, under
  // the assumption that the given label map node is already added to the scene.
  // A display node will be added to it if the label node doesn't already have
  // one, and the image data associated with the label node will be allocated
  // according to the template volumeNode.
  vtkMRMLScalarVolumeNode *FillLabelVolumeFromTemplate(vtkMRMLScene *scene, vtkMRMLScalarVolumeNode *labelNode, vtkMRMLVolumeNode *templateNode);

  // Description:
  // Create a deep copy of a volume and add it to the scene
  vtkMRMLScalarVolumeNode *CloneVolume (vtkMRMLScene *scene, 
                                        vtkMRMLVolumeNode *volumeNode, 
                                        const char *name);

  // Description:
  // Computes matrix we need to register
  // V1Node to V2Node given the
  // "register.dat" matrix from tkregister2 (FreeSurfer)
  void TranslateFreeSurferRegistrationMatrixIntoSlicerRASToRASMatrix( vtkMRMLVolumeNode *V1Node,
                             vtkMRMLVolumeNode *V2Node,
                             vtkMatrix4x4 *FSRegistrationMatrix,
                             vtkMatrix4x4 *ResultsMatrix);
  // Description:
  // Convenience method to compute
  // a volume's Vox2RAS-tkreg Matrix
  void ComputeTkRegVox2RASMatrix ( vtkMRMLVolumeNode *VNode,
                                   vtkMatrix4x4 *M );

protected:
  vtkSlicerVolumesLogic();
  virtual ~vtkSlicerVolumesLogic();
  vtkSlicerVolumesLogic(const vtkSlicerVolumesLogic&);
  void operator=(const vtkSlicerVolumesLogic&);

  virtual void ProcessMRMLNodesEvents(vtkObject * caller,
                                  unsigned long event,
                                  void * callData);

  // Description:
  // Examine the file name to see if the extension is one of the supported
  // freesurfer volume formats. Used to assign the proper colour node to label
  // maps.
  int IsFreeSurferVolume (const char* filename);
  
  // Description:
  //
  vtkMRMLVolumeNode *ActiveVolumeNode;
};

#endif

