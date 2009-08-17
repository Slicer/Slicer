/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLMRIBiasFieldCorrectionNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLMRIBiasFieldCorrectionNode_h
#define __vtkMRMLMRIBiasFieldCorrectionNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"

#include "vtkMRIBiasFieldCorrection.h"

class vtkImageData;

class VTK_MRIBiasFieldCorrection_EXPORT vtkMRMLMRIBiasFieldCorrectionNode : public vtkMRMLNode
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
  virtual void ReadXMLAttributes( const char** atts);

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
  // Get/Set Conductance (module parameter)
  vtkGetMacro(Shrink, double);
  vtkSetMacro(Shrink, double);
  
    // Description:
  // Get/Set Conductance (module parameter)
  vtkGetMacro(Max, double);
  vtkSetMacro(Max, double);
  
      // Description:
  // Get/Set Conductance (module parameter)
  vtkGetMacro(Num, double);
  vtkSetMacro(Num, double);
  
        // Description:
  // Get/Set Conductance (module parameter)
  vtkGetMacro(Wien, double);
  vtkSetMacro(Wien, double);
 
 
         // Description:
  // Get/Set Conductance (module parameter)
  vtkGetMacro(Field, double);
  vtkSetMacro(Field, double);
  
  
          // Description:
  // Get/Set Conductance (module parameter)
  vtkGetMacro(Con, double);
  vtkSetMacro(Con, double);
  
  // Description:
  // Get/Set input volume MRML Id
  vtkGetStringMacro(InputVolumeRef);
  vtkSetStringMacro(InputVolumeRef);
  
  // Description:
  // Get/Set output volume MRML Id
  vtkGetStringMacro(OutputVolumeRef);
  vtkSetStringMacro(OutputVolumeRef);
  
  vtkGetStringMacro(StorageVolumeRef);
  vtkSetStringMacro(StorageVolumeRef);
  
  vtkGetStringMacro(MaskVolumeRef);
  vtkSetStringMacro(MaskVolumeRef);


  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

 
protected:
  vtkMRMLMRIBiasFieldCorrectionNode();
  ~vtkMRMLMRIBiasFieldCorrectionNode();
  vtkMRMLMRIBiasFieldCorrectionNode(const vtkMRMLMRIBiasFieldCorrectionNode&);
  void operator=(const vtkMRMLMRIBiasFieldCorrectionNode&);
  
  char* InputVolumeRef;
  char* OutputVolumeRef;
  char* StorageVolumeRef;
  char* MaskVolumeRef;

  double Shrink;
  double Max;
  double Num;
  double Wien;
  double Field;
  double Con;

};

#endif

