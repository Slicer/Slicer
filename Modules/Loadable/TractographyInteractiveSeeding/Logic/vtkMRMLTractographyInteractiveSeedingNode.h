/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTractographyInteractiveSeedingNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLTractographyInteractiveSeedingNode_h
#define __vtkMRMLTractographyInteractiveSeedingNode_h

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLStorageNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>

#include "vtkSlicerTractographyInteractiveSeedingModuleLogicExport.h"

class vtkImageData;

class VTK_SLICER_TRACTOGRAPHYINTERACTIVESEEDING_MODULE_LOGIC_EXPORT vtkMRMLTractographyInteractiveSeedingNode :
  public vtkMRMLNode
{
public:
  static vtkMRMLTractographyInteractiveSeedingNode *New();
  vtkTypeMacro(vtkMRMLTractographyInteractiveSeedingNode,vtkMRMLNode);
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
  // 0 - FA
  // 1 - Linear Measure
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

  // Description
  // Maximum length in mm for a path (otherwise the path will be deleted).
  // Currently only used in SeedAndSaveStreamlinesInROI.
  vtkGetMacro(MaximumPathLength,double);
  vtkSetMacro(MaximumPathLength,double);

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
  // Enable/Disable Lable Map seeding random grid
  vtkBooleanMacro(RandomGrid, int);
  vtkGetMacro(RandomGrid, int);
  vtkSetMacro(RandomGrid, int);

  // Description:
  // Array of labels to seed from
  vtkGetObjectMacro(ROILabels, vtkIntArray);
  vtkSetObjectMacro(ROILabels, vtkIntArray);

  // Description:
  // Enable/Disable Lable Map seeding use index space
  vtkBooleanMacro(UseIndexSpace, int);
  vtkGetMacro(UseIndexSpace, int);
  vtkSetMacro(UseIndexSpace, int);

  // Description
  // Lable Map seeding Linear Measure start threshold.
  vtkGetMacro(LinearMeasureStart,double);
  vtkSetMacro(LinearMeasureStart,double);

  // Description
  // Lable Map seeding seed spacing.
  vtkGetMacro(SeedSpacing,double);
  vtkSetMacro(SeedSpacing,double);

  // Description:
  // Enable/Disable WriteToFile
  vtkBooleanMacro(WriteToFile, int);
  vtkGetMacro(WriteToFile, int);
  vtkSetMacro(WriteToFile, int);

  // Description
  // file directory.
  vtkGetStringMacro(FileDirectoryName);
  vtkSetStringMacro(FileDirectoryName);

  // Description
  // file prefix.
  vtkGetStringMacro(FilePrefix);
  vtkSetStringMacro(FilePrefix);

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

  void StringToROILabels(std::string labels);

  std::string ROILabelsToString();

protected:
  vtkMRMLTractographyInteractiveSeedingNode();
  ~vtkMRMLTractographyInteractiveSeedingNode();
  vtkMRMLTractographyInteractiveSeedingNode(const vtkMRMLTractographyInteractiveSeedingNode&);
  void operator=(const vtkMRMLTractographyInteractiveSeedingNode&);

  int StoppingMode;
  int DisplayMode;

  double StoppingValue;
  double StoppingCurvature;
  double IntegrationStep;
  double SeedingRegionSize;
  double SeedingRegionStep;
  double MinimumPathLength;
  double MaximumPathLength;
  int MaxNumberOfSeeds;
  int SeedSelectedFiducials;
  int EnableSeeding;

  vtkIntArray *ROILabels;
  int RandomGrid;
  int UseIndexSpace;
  double LinearMeasureStart;
  double SeedSpacing;

  int WriteToFile;
  char* FileDirectoryName;
  char* FilePrefix;

  char* InputVolumeRef;
  char* InputFiducialRef;
  char* OutputFiberRef;

};

#endif

