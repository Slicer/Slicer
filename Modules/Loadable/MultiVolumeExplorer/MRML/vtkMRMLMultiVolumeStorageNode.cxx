/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLNRRDStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

#include "vtkMRMLMultiVolumeStorageNode.h"
#include "vtkMRMLMultiVolumeNode.h"

#include "vtkNRRDReader.h"

#include "vtkObjectFactory.h"


//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMultiVolumeStorageNode);

//----------------------------------------------------------------------------
vtkMRMLMultiVolumeStorageNode::vtkMRMLMultiVolumeStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLMultiVolumeStorageNode::~vtkMRMLMultiVolumeStorageNode()
{
}

//----------------------------------------------------------------------------
bool vtkMRMLMultiVolumeStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return this->vtkMRMLNRRDStorageNode::CanReadInReferenceNode(refNode) || 
    refNode->IsA("vtkMRMLMultiVolumeNode");
}

//----------------------------------------------------------------------------
int vtkMRMLMultiVolumeStorageNode::ReadDataInternal(vtkMRMLNode* refNode)
{
  vtkSmartPointer<vtkNRRDReader> reader =  vtkSmartPointer<vtkNRRDReader>::New();

  if (!this->CanReadInReferenceNode(refNode))
    {
    return 0;
    }
  
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  reader->SetFileName(fullName.c_str());

  // Check if this is a NRRD file that we can read
  if (!reader->CanReadFile(fullName.c_str()))
    {
    vtkDebugMacro("vtkMRMLNRRDStorageNode: This is not a nrrd file");
    return 0;
    }

  // Read the header to see if the NRRD file corresponds to the
  // MRML Node
  reader->UpdateInformation();
  
  // Check to see if the information contains MultiVolume attributes
  typedef std::vector<std::string> KeyVector;
  KeyVector keys = reader->GetHeaderKeysVector();
  KeyVector::iterator kit = std::find(keys.begin(), keys.end(), "MultiVolume.NumberOfFrames");
  if (kit == keys.end())
    {
    // not a MultiVolume file
    return 0;
    }
  else
    {
    // verified as a MultiVolume file.  need to set the number of frames.
    vtkMRMLMultiVolumeNode* mvNode = dynamic_cast<vtkMRMLMultiVolumeNode*>(refNode);
    if (mvNode)
      {
      mvNode->SetNumberOfFrames(atoi(reader->GetHeaderValue("MultiVolume.NumberOfFrames")));
      }
    }
  
  // delegate to the superclass
  return Superclass::ReadDataInternal(refNode);
}
