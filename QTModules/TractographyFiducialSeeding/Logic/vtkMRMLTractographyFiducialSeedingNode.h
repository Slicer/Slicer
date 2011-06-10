/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTractographyFiducialSeedingNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLTractographyFiducialSeedingNode_h
#define __vtkMRMLTractographyFiducialSeedingNode_h

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

// VTK includes
#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"

#include "vtkSlicerTractographyModuleLogicExport.h"

class vtkImageData;

class VTK_SLICER_TRACTOGRAPHY_MODULE_LOGIC_EXPORT vtkMRMLTractographyFiducialSeedingNode :
  public vtkMRMLNode
{
  public:
  static vtkMRMLTractographyFiducialSeedingNode *New();
  vtkTypeMacro(vtkMRMLTractographyFiducialSeedingNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create instance of a GAD node.
  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes from name/value pairs
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get unique node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "FiducialSeedingParameters";};

  // Description:
  // Get/Set Stopping Mode (module parameter)
  // 0 - Linear Measure
  // 1 - FA
  vtkGetMacro(StoppingMode, int);
  vtkSetMacro(StoppingMode, int);

  // Description:
  // Get/Set Stopping Value (module parameter)
  vtkGetMacro(StoppingValue, double);
  vtkSetMacro(StoppingValue, double);

  // Description:
  // Get/Set Stopping Curvature (module parameter)
  vtkGetMacro(StoppingCurvature, double);
  vtkSetMacro(StoppingCurvature, double);
  
  // Description:
  // Get/Set Integration Step (module parameter)
  vtkGetMacro(IntegrationStep, double);
  vtkSetMacro(IntegrationStep, double);

    // Description:
  // Get/Set Seeding Region Size  (module parameter)
  vtkGetMacro(SeedingRegionSize, double);
  vtkSetMacro(SeedingRegionSize, double);

  // Description:
  // Get/Set Seeding Region Step (module parameter)
  vtkGetMacro(SeedingRegionStep, double);
  vtkSetMacro(SeedingRegionStep, double);

  // Description:
  // Get/Set Seeding max seeding size (module parameter)
  vtkGetMacro(MaxNumberOfSeeds, int);
  vtkSetMacro(MaxNumberOfSeeds, int);

  // Description
  // Minimum length in mm for a path (otherwise the path will be deleted).
  // Currently only used in SeedAndSaveStreamlinesInROI.
  vtkGetMacro(MinimumPathLength,double);
  vtkSetMacro(MinimumPathLength,double);
 
  // Description:
  // Specifies whether seed only from selected fiducials
  vtkBooleanMacro(SeedSelectedFiducials, int);
  vtkGetMacro(SeedSelectedFiducials, int);
  vtkSetMacro(SeedSelectedFiducials, int);

  // Description:
  // Get/Set Display Mode (module parameter)
  // 0 - Lines
  // 1 - Tubes
  vtkGetMacro(DisplayMode, int);
  vtkSetMacro(DisplayMode, int);

  
  // Description:
  // Enable/Disable fiducial seeding
  vtkBooleanMacro(EnableSeeding, int);
  vtkGetMacro(EnableSeeding, int);
  vtkSetMacro(EnableSeeding, int);


  // Description:
  // Get/Set input volume MRML Id
  vtkGetStringMacro(InputVolumeRef);
  vtkSetStringMacro(InputVolumeRef);

  // Description:
  // Get/Set input fiducial MRML Id
  vtkGetStringMacro(InputFiducialRef);
  vtkSetStringMacro(InputFiducialRef);
  
  // Description:
  // Get/Set output volume MRML Id
  vtkGetStringMacro(OutputFiberRef);
  vtkSetStringMacro(OutputFiberRef);

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

 
protected:
  vtkMRMLTractographyFiducialSeedingNode();
  ~vtkMRMLTractographyFiducialSeedingNode();
  vtkMRMLTractographyFiducialSeedingNode(const vtkMRMLTractographyFiducialSeedingNode&);
  void operator=(const vtkMRMLTractographyFiducialSeedingNode&);

  int StoppingMode;
  int DisplayMode;

  double StoppingValue;
  double StoppingCurvature;
  double IntegrationStep;
  double SeedingRegionSize;
  double SeedingRegionStep;
  double MinimumPathLength;
  int MaxNumberOfSeeds;
  int SeedSelectedFiducials;
  int EnableSeeding;

  char* InputVolumeRef;
  char* InputFiducialRef;
  char* OutputFiberRef;

};

#endif

