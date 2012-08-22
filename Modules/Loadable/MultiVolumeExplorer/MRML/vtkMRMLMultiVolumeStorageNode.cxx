/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLNRRDStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

#include <algorithm>

#include "vtkMRMLMultiVolumeStorageNode.h"
#include "vtkMRMLMultiVolumeNode.h"

#include "vtkNRRDReader.h"

#include "vtkObjectFactory.h"
#include "vtkImageChangeInformation.h"
#include "vtkImageData.h"

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
  return refNode->IsA("vtkMRMLMultiVolumeNode");
}

//----------------------------------------------------------------------------
int vtkMRMLMultiVolumeStorageNode::ReadDataInternal(vtkMRMLNode* refNode)
{
  if (!this->CanReadInReferenceNode(refNode))
    {
    return 0;
    }

  vtkMRMLMultiVolumeNode* volNode = dynamic_cast<vtkMRMLMultiVolumeNode*>(refNode);
  if (!volNode)
    {
    vtkErrorMacro("ReadDataInternal: not a MultiVolume node.");
    return 0;
    }
  
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  vtkSmartPointer<vtkNRRDReader> reader =  vtkSmartPointer<vtkNRRDReader>::New();
  reader->SetFileName(fullName.c_str());

  // Check if this is a NRRD file that we can read
  if (!reader->CanReadFile(fullName.c_str()))
    {
    vtkDebugMacro("vtkMRMLMultiVolumeStorageNode: This is not a nrrd file");
    return 0;
    }

  // Set up reader
  if (this->CenterImage) 
    {
    reader->SetUseNativeOriginOff();
    }
  else
    {
    reader->SetUseNativeOriginOn();
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

  // 
  // Finally have verified that we have a MultiVolume nrrd file
  //

  // prepare volume node
  if (volNode->GetImageData()) 
    {
    volNode->SetAndObserveImageData (NULL);
    }

  // Read the volume
  reader->Update();

  // Define the coordinate frame
  vtkMatrix4x4* mat = reader->GetRasToIjkMatrix();
  volNode->SetRASToIJKMatrix(mat);

  // parse non-specific key-value pairs 
  for ( kit = keys.begin(); kit != keys.end(); ++kit)
    {
    volNode->SetAttribute((*kit).c_str(), reader->GetHeaderValue((*kit).c_str()));      }


  // configure the canonical vtk meta data
  vtkSmartPointer<vtkImageChangeInformation> ici = vtkSmartPointer<vtkImageChangeInformation>::New();
  ici->SetInput (reader->GetOutput());
  ici->SetOutputSpacing( 1, 1, 1 );
  ici->SetOutputOrigin( 0, 0, 0 );
  ici->Update();

  // assign the buffer
  volNode->SetAndObserveImageData (ici->GetOutput());

  // 
  return 1;
}
