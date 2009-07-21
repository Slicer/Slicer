/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLMRIBiasFieldCorrectionNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=======================================================================auto=*/

#ifndef __vtkMRMLMRIBiasFieldCorrectionNode_h
#define __vtkMRMLMRIBiasFieldCorrectionNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRIBiasFieldCorrection.h"
#include "vtkImageData.h"

class VTK_MRIBiasFieldCorrection_EXPORT vtkMRMLMRIBiasFieldCorrectionNode :
  public vtkMRMLNode
{
  public:
  static vtkMRMLMRIBiasFieldCorrectionNode *New();
  vtkTypeMacro(vtkMRMLMRIBiasFieldCorrectionNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create instance of a GAD node.
  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes from name/value pairs
  virtual void ReadXMLAttributes(const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get unique node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "GADParameters";};

  // Description:
  // Get/Set the threshold to create the mask
  vtkGetMacro(MaskThreshold, double);
  vtkSetMacro(MaskThreshold, double);

  // Description:
  // Get/Set the factor to downsample the image
  vtkGetMacro(OutputSize, double);
  vtkSetMacro(OutputSize, double);

  // Description:
  // Get/Set the number of fitting levels
  vtkGetMacro(NumberOfFittingLevels, unsigned int);
  vtkSetMacro(NumberOfFittingLevels, unsigned int);

  // Description:
  // Get/Set number of iterations
  vtkGetMacro(NumberOfIterations, unsigned int);
  vtkSetMacro(NumberOfIterations, unsigned int);

  // Description:
  // Get/Set the Wiener filter noise
  vtkGetMacro(WienerFilterNoise, double);
  vtkSetMacro(WienerFilterNoise, double);

  // Description:
  // Get/Set the bias field value
  vtkGetMacro(BiasField, double);
  vtkSetMacro(BiasField, double);

  // Description:
  // Get/Set convergence value
  vtkGetMacro(ConvergenceThreshold, double);
  vtkSetMacro(ConvergenceThreshold, double);

  // Description:
  // Get/Set input volume MRML id
  vtkGetStringMacro(InputVolumeRef);
  vtkSetStringMacro(InputVolumeRef);

  // Description:
  // Get/Set output volume MRML id
  vtkGetStringMacro(OutputVolumeRef);
  vtkSetStringMacro(OutputVolumeRef);

  // Description:
  // Get/Set preview volume MRML id
  vtkGetStringMacro(PreviewVolumeRef);
  vtkSetStringMacro(PreviewVolumeRef);

  // Description:
  // Get/Set mask volume MRML id
  vtkGetStringMacro(MaskVolumeRef);
  vtkSetStringMacro(MaskVolumeRef);

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);
/*
  // Description:
  // Get/Set demo image
  vtkImageData *GetDemoImage() { return this->DemoImage; }
  void SetDemoImage(vtkImageData *image)
    {
    if (this->DemoImage)
      {
      this->DemoImage->Delete();
      }
    this->DemoImage = image;
    this->DemoImage->Register(NULL);
    }
*/
protected:
  vtkMRMLMRIBiasFieldCorrectionNode();
  ~vtkMRMLMRIBiasFieldCorrectionNode();
  vtkMRMLMRIBiasFieldCorrectionNode(const vtkMRMLMRIBiasFieldCorrectionNode&);
  void operator=(const vtkMRMLMRIBiasFieldCorrectionNode&);

  //vtkImageData *DemoImage;

  char *InputVolumeRef;
  char *OutputVolumeRef;
  char *PreviewVolumeRef;
  char *MaskVolumeRef;

  unsigned int NumberOfIterations;
  unsigned int NumberOfFittingLevels;

  double MaskThreshold;
  double OutputSize;
  double WienerFilterNoise;
  double BiasField;
  double ConvergenceThreshold;
};

#endif

