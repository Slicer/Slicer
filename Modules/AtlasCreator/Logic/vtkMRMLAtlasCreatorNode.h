/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAtlasCreatorNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLAtlasCreatorNode_h
#define __vtkMRMLAtlasCreatorNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

#include "vtkSlicerAtlasCreatorModuleLogicExport.h"

// Description:
// The atlas creator node is simply a MRMLNode container for
// a configuration of the Atlas Creator module

class VTK_SLICER_ATLASCREATOR_MODULE_LOGIC_EXPORT vtkMRMLAtlasCreatorNode : public vtkMRMLNode
{
  public:
  static vtkMRMLAtlasCreatorNode *New();
  vtkTypeMacro(vtkMRMLAtlasCreatorNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  virtual const char* GetNodeTagName()
    {return "AtlasCreator";};

  //BTX
  enum
  {
    LaunchComputationEvent = 31337
  };
  //ETX

  // Description: Initialize the configuration with default values
  //   The default values are:
  //   - all input and output filePaths, filePathLists and labelLists to ""
  //   - Toolkit: BRAINSFit
  //   - Template Type: Fixed
  //   - DynamicTemplateIterations: 5
  //   - Affine Registration
  //   - Save Transforms and keep aligned images
  //   - Deactivate Normalization including all settings
  //   - Deactivate PCA including all settings
  //   - Deactivate Cluster including all settings
  //   - Deactivate SkipRegistration including all settings
  //   - Deactivate Debug and Dryrun Mode
  void InitializeByDefault();

  // Description: Launch the Atlas Creator computation with the assigned parameters
  void Launch();

  // Description: The originalImages FilePaths divided by space
  vtkGetStringMacro (OriginalImagesFilePathList);
  vtkSetStringMacro (OriginalImagesFilePathList);

  // Description: The segmentation FilePaths divided by space
  vtkGetStringMacro (SegmentationsFilePathList);
  vtkSetStringMacro (SegmentationsFilePathList);

  // Description: The output Directory
  vtkGetStringMacro (OutputDirectory);
  vtkSetStringMacro (OutputDirectory);


  // Description: The Toolkit: "BRAINSFit" or "CMTK"
  vtkGetStringMacro (Toolkit);
  vtkSetStringMacro (Toolkit);


  // Description: The Template Type: "fixed", "dynamic" or "group"
  vtkGetStringMacro (TemplateType);
  vtkSetStringMacro (TemplateType);

  // Description: The Number of Iterations for DynamicTemplate
  vtkGetMacro(DynamicTemplateIterations,int);
  vtkSetMacro(DynamicTemplateIterations,int);

  // Description: The Default Case FilePath for FixedTemplate
  vtkGetStringMacro (FixedTemplateDefaultCaseFilePath);
  vtkSetStringMacro (FixedTemplateDefaultCaseFilePath);

  // Description: 1: Ignore the template segmentation in fixed mode 0: Don't ignore
  vtkGetMacro(IgnoreTemplateSegmentation,int);
  vtkSetMacro(IgnoreTemplateSegmentation,int);

  // Description: The Labels divided by space
  vtkGetStringMacro (LabelsList);
  vtkSetStringMacro (LabelsList);


  // Description: The Registration Type: "Affine" or "Non-Rigid"
  vtkGetStringMacro (RegistrationType);
  vtkSetStringMacro (RegistrationType);


  // Description: 1: Save Transforms 0: Don't save
  vtkGetMacro(SaveTransforms,int);
  vtkSetMacro(SaveTransforms,int);

  // Description: 1: Delete Aligned Images 0: Don't delete
  vtkGetMacro(DeleteAlignedImages,int);
  vtkSetMacro(DeleteAlignedImages,int);

  // Description: 1: Delete Aligned Segmentations 0: Don't delete
  vtkGetMacro(DeleteAlignedSegmentations,int);
  vtkSetMacro(DeleteAlignedSegmentations,int);

  // Description: 1: Normalize Atlases 0: Don't normalize
  vtkGetMacro(NormalizeAtlases,int);
  vtkSetMacro(NormalizeAtlases,int);

  // Description: Set NormalizeTo Value
  vtkGetMacro(NormalizeTo,int);
  vtkSetMacro(NormalizeTo,int);


  // Description: The Output Cast
  //   "Char"
  //   "Unsigned Char"
  //   "Double"
  //   "Float"
  //   "Int"
  //   "Unsigned Int"
  //   "Long"
  //   "Unsigned Long"
  //   "Short"
  //   "Unsigned Short"
  vtkGetStringMacro (OutputCast);
  vtkSetStringMacro (OutputCast);


  // Description: 1: Use PCA Analysis 0: Don't use PCA Analysis
  vtkGetMacro(PCAAnalysis,int);
  vtkSetMacro(PCAAnalysis,int);

  // Description: The Number of Max EigenVectors to use for PCA Analysis
  vtkGetMacro(PCAMaxEigenVectors,int);
  vtkSetMacro(PCAMaxEigenVectors,int);

  // Description: 1: Combine all PCA output 0: Don't combine all PCA output
  vtkGetMacro(PCACombine,int);
  vtkSetMacro(PCACombine,int);


  // Description: 1: Use Cluster 0: Don't use cluster
  vtkGetMacro(UseCluster,int);
  vtkSetMacro(UseCluster,int);

  // Description: The Scheduler Command
  vtkGetStringMacro (SchedulerCommand);
  vtkSetStringMacro (SchedulerCommand);


  // Description: The number of threads to use for Registration and Resampling, -1: Use Maximum Number of Threads
  vtkGetMacro(NumberOfThreads,int);
  vtkSetMacro(NumberOfThreads,int);


  // Description: 1: Skip Registration 0: Don't skip registration
  vtkGetMacro(SkipRegistration,int);
  vtkSetMacro(SkipRegistration,int);

  // Description: The Existing Template (Filepath)
  vtkGetStringMacro (ExistingTemplate);
  vtkSetStringMacro (ExistingTemplate);

  // Description: The Transforms Directory
  vtkGetStringMacro (TransformsDirectory);
  vtkSetStringMacro (TransformsDirectory);


  // Description: 1: Use Debug Mode 0: Don't use Debug Mode
  vtkGetMacro(DebugMode,int);
  vtkSetMacro(DebugMode,int);

  // Description: 1: Use Dryrun Mode 0: Don't use Dryrun Mode
  vtkGetMacro(DryrunMode,int);
  vtkSetMacro(DryrunMode,int);

  // Description: 1: Use Test Mode 0: Don't use Test Mode
  vtkGetMacro(TestMode,int);
  vtkSetMacro(TestMode,int);

protected:

  char *OriginalImagesFilePathList;
  char *SegmentationsFilePathList;
  char *OutputDirectory;

  char *Toolkit;

  char *TemplateType;
  int DynamicTemplateIterations;
  char *FixedTemplateDefaultCaseFilePath;
  int IgnoreTemplateSegmentation;
  
  char *LabelsList;

  char *RegistrationType;

  int SaveTransforms;
  int DeleteAlignedImages;
  int DeleteAlignedSegmentations;
  int NormalizeAtlases;
  int NormalizeTo;

  char *OutputCast;

  int PCAAnalysis;
  int PCAMaxEigenVectors;
  int PCACombine;

  int UseCluster;
  char *SchedulerCommand;

  int NumberOfThreads;

  int SkipRegistration;
  char *ExistingTemplate;
  char *TransformsDirectory;

  int DebugMode;
  int DryrunMode;

  // the testmode is only for internal use and will not be saved to MRML
  int TestMode;


private:
  vtkMRMLAtlasCreatorNode();
  ~vtkMRMLAtlasCreatorNode();
  vtkMRMLAtlasCreatorNode(const vtkMRMLAtlasCreatorNode&);
  void operator=(const vtkMRMLAtlasCreatorNode&);

};

#endif

