/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerModuleTemplateLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerModuleTemplateLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerModuleTemplateLogic_h
#define __vtkSlicerModuleTemplateLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLVolumeNode.h>

// STD includes
#include <cstdlib>

#include "vtkSlicerModuleTemplateModuleLogicExport.h"

class vtkMRMLScalarVolumeNode;
class vtkMRMLScalarVolumeDisplayNode;
class vtkMRMLVolumeHeaderlessStorageNode;
class vtkStringArray;

class VTK_SLICER_MODULETEMPLATE_MODULE_LOGIC_EXPORT vtkSlicerModuleTemplateLogic :
  public vtkSlicerModuleLogic
{
public:
  
  static vtkSlicerModuleTemplateLogic *New();
  vtkTypeRevisionMacro(vtkSlicerModuleTemplateLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// The currently active mrml volume node 
  vtkGetObjectMacro(ActiveVolumeNode, vtkMRMLVolumeNode);
  void SetActiveVolumeNode(vtkMRMLVolumeNode *ActiveVolumeNode);

  ///
  /// Sub type of loading an archetype volume that is known to be a scalar
  vtkMRMLScalarVolumeNode* AddArchetypeScalarVolume(const char *filename, const char* volname, int loadingOptions);
  vtkMRMLScalarVolumeNode* AddArchetypeScalarVolume(const char *filename, const char* volname);

  /// Overloaded function of AddArchetypeVolume to provide more 
  /// loading options, where variable loadingOptions is bit-coded as following:
  /// bit 0: label map
  /// bit 1: centered
  /// bit 2: loading single file
  /// higher bits are reserved for future use
  vtkMRMLVolumeNode* AddArchetypeVolume(const char* filename, 
                                        const char* volname, int loadingOptions);
  vtkMRMLVolumeNode* AddArchetypeVolume(const char* filename, 
                                        const char* volname, int loadingOptions, 
                                        vtkStringArray *fileList);
  vtkMRMLVolumeNode* AddArchetypeVolume(const char *filename, const char* volname);

  ///
  /// Create new mrml node and associated storage node.
  /// Read image data from a specified file
  /// vtkMRMLVolumeNode* AddArchetypeVolume (const char* filename, int centerImage, int labelMap, const char* volname);

  /// Overloaded function of AddHeaderVolume to provide more 
  /// loading options, where variable loadingOptions is bit-coded as following:
  /// bit 0: label map
  /// bit 1: centered
  /// bit 2: loading signal file
  /// higher bits are reserved for future use
  vtkMRMLVolumeNode* AddHeaderVolume(const char* filename, const char* volname, 
                                     vtkMRMLVolumeHeaderlessStorageNode *headerStorage,
                                     int loadingOptions);

  ///
  /// Write volume's image data to a specified file
  int SaveArchetypeVolume(const char* filename, vtkMRMLVolumeNode *volumeNode);

  ///
  /// Create a label map volume to match the given volume node and add it to
  /// the scene
  vtkMRMLScalarVolumeNode *CreateLabelVolume(vtkMRMLScene *scene, 
                                             vtkMRMLVolumeNode *volumeNode, 
                                             const char *name);

  ///
  /// Create a deep copy of a volume and add it to the scene
  vtkMRMLScalarVolumeNode *CloneVolume(vtkMRMLScene *scene, 
                                       vtkMRMLVolumeNode *volumeNode, 
                                       const char *name);

  ///
  /// Update MRML events
  virtual void ProcessMRMLEvents(vtkObject * caller, unsigned long event, void * callData);
  
  ///
  /// Update logic events
  virtual void ProcessLogicEvents(vtkObject * caller, unsigned long event, void * callData);  
  
  //BTX
  using vtkSlicerLogic::ProcessLogicEvents; 
  //ETX
  
  ///
  /// Computes matrix we need to register
  /// V1Node to V2Node given the
  /// "register.dat" matrix from tkregister2 (FreeSurfer)
  void TranslateFreeSurferRegistrationMatrixIntoSlicerRASToRASMatrix(vtkMRMLVolumeNode *V1Node,
                             vtkMRMLVolumeNode *V2Node,
                             vtkMatrix4x4 *FSRegistrationMatrix,
                             vtkMatrix4x4 *ResultsMatrix);
  ///
  /// Convenience method to compute
  /// a volume's Vox2RAS-tkreg Matrix
  void ComputeTkRegVox2RASMatrix(vtkMRMLVolumeNode *VNode, vtkMatrix4x4 *M);

protected:
  vtkSlicerModuleTemplateLogic();
  virtual ~vtkSlicerModuleTemplateLogic();

  ///
  /// Examine the file name to see if the extension is one of the supported
  /// freesurfer volume formats. Used to assign the proper colour node to label maps.
  int IsFreeSurferVolume(const char* filename);
  
  vtkMRMLVolumeNode *ActiveVolumeNode;
  
private:

  vtkSlicerModuleTemplateLogic(const vtkSlicerModuleTemplateLogic&); // Not implemented
  void operator=(const vtkSlicerModuleTemplateLogic&);               // Not implemented
};

#endif

