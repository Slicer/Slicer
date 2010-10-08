/*==========================================================================

  Portions (c) Copyright 2008-2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Modules/OpenIGTLinkIF/vtkIGTLToMRMLImageMetaList.cxx $
  Date:      $Date: 2009-10-05 17:37:20 -0400 (Mon, 05 Oct 2009) $
  Version:   $Revision: 10577 $

==========================================================================*/

#include <vtksys/SystemTools.hxx>

#include "vtkObjectFactory.h"
#include "vtkIGTLToMRMLImageMetaList.h"

#include "vtkSlicerColorLogic.h"

#include "vtkImageData.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "igtlImageMessage.h"

#include "vtkMRMLImageMetaListNode.h"
#include "vtkMRMLIGTLQueryNode.h"

vtkStandardNewMacro(vtkIGTLToMRMLImageMetaList);
vtkCxxRevisionMacro(vtkIGTLToMRMLImageMetaList, "$Revision: 10577 $");


//---------------------------------------------------------------------------
vtkIGTLToMRMLImageMetaList::vtkIGTLToMRMLImageMetaList()
{
}


//---------------------------------------------------------------------------
vtkIGTLToMRMLImageMetaList::~vtkIGTLToMRMLImageMetaList()
{
}


//---------------------------------------------------------------------------
void vtkIGTLToMRMLImageMetaList::PrintSelf(ostream& os, vtkIndent indent)
{
}


//---------------------------------------------------------------------------
vtkMRMLNode* vtkIGTLToMRMLImageMetaList::CreateNewNode(vtkMRMLScene* scene, const char* name)
{

  vtkMRMLImageMetaListNode *imetaNode = vtkMRMLImageMetaListNode::New();
  imetaNode->SetName(name);
  imetaNode->SetDescription("Received by OpenIGTLink");

  scene->AddNode(imetaNode);

  return imetaNode;
}


//---------------------------------------------------------------------------
vtkIntArray* vtkIGTLToMRMLImageMetaList::GetNodeEvents()
{
  vtkIntArray* events;

  events = vtkIntArray::New();
  //events->InsertNextValue(vtkMRMLImageMetaListNode::ModifiedEvent); 

  return events;
}


//---------------------------------------------------------------------------
int vtkIGTLToMRMLImageMetaList::IGTLToMRML(igtl::MessageBase::Pointer buffer, vtkMRMLNode* node)
{
  if (strcmp(node->GetNodeTagName(), "ImageMetaList") != 0)
    {
    //std::cerr << "Invalid node!!!!" << std::endl;
    return 0;
    }

  // Create a message buffer to receive image meta data
  igtl::ImageMetaMessage::Pointer imgMeta;
  imgMeta = igtl::ImageMetaMessage::New();
  imgMeta->Copy(buffer); // !! TODO: copy makes performance issue.

  // Deserialize the image meta data
  // If CheckCRC==0, CRC check is skipped.
  int c = imgMeta->Unpack(this->CheckCRC);

  if ((c & igtl::MessageHeader::UNPACK_BODY) == 0) // if CRC check fails
    {
    // TODO: error handling
    return 0;
    }

  if (node == NULL)
    {
    return 0;
    }

  vtkMRMLImageMetaListNode* imetaNode = vtkMRMLImageMetaListNode::SafeDownCast(node);
  if (imetaNode == NULL)
    {
    return 0;
    }

  imetaNode->ClearImageMetaElement();

  int nElements = imgMeta->GetNumberOfImageMetaElement();
  for (int i = 0; i < nElements; i ++)
    {
    igtl::ImageMetaElement::Pointer imgMetaElement;
    imgMeta->GetImageMetaElement(i, imgMetaElement);

    igtlUint16 size[3];
    imgMetaElement->GetSize(size);

    igtl::TimeStamp::Pointer ts;
    imgMetaElement->GetTimeStamp(ts);
    double time = ts->GetTimeStamp();

    vtkMRMLImageMetaListNode::ImageMetaElement element;
    element.Name        = imgMetaElement->GetName();
    element.DeviceName  = imgMetaElement->GetDeviceName();
    element.Modality    = imgMetaElement->GetModality();
    element.PatientName = imgMetaElement->GetPatientName();
    element.PatientID   = imgMetaElement->GetPatientID();
    element.TimeStamp   = time;
    element.Size[0]     = size[0];
    element.Size[1]     = size[1];
    element.Size[2]     = size[2];
    element.ScalarType  = imgMetaElement->GetScalarType();

    // DEBUG
    std::cerr << "========== Element #" << i << " ==========" << std::endl;
    std::cerr << " Name       : " << imgMetaElement->GetName() << std::endl;
    std::cerr << " DeviceName : " << imgMetaElement->GetDeviceName() << std::endl;
    std::cerr << " Modality   : " << imgMetaElement->GetModality() << std::endl;
    std::cerr << " PatientName: " << imgMetaElement->GetPatientName() << std::endl;
    std::cerr << " PatientID  : " << imgMetaElement->GetPatientID() << std::endl;
    std::cerr << " TimeStamp  : " << std::fixed << time << std::endl;
    std::cerr << " Size       : ( " << size[0] << ", " << size[1] << ", " << size[2] << ")" << std::endl;
    std::cerr << " ScalarType : " << (int) imgMetaElement->GetScalarType() << std::endl;
    std::cerr << "================================" << std::endl;

    imetaNode->AddImageMetaElement(element);
    }

  imetaNode->Modified();

  return 1;

}


//---------------------------------------------------------------------------
int vtkIGTLToMRMLImageMetaList::MRMLToIGTL(unsigned long event, vtkMRMLNode* mrmlNode, int* size, void** igtlMsg)
{
  if (!mrmlNode)
    {
    return 0;
    }

  // If mrmlNode is query node
  if (strcmp(mrmlNode->GetNodeTagName(), "IGTLQuery") == 0 ) // Query Node
    {
    vtkMRMLIGTLQueryNode* qnode = vtkMRMLIGTLQueryNode::SafeDownCast(mrmlNode);
    if (qnode)
      {
      if (qnode->GetQueryType() == vtkMRMLIGTLQueryNode::TYPE_GET)
        {

        //igtl::TransformMessage::Pointer OutTransformMsg;
        if (this->GetImageMetaMessage.IsNull())
          {
          this->GetImageMetaMessage = igtl::GetImageMetaMessage::New();
          }
        this->GetImageMetaMessage->SetDeviceName(mrmlNode->GetName());
        this->GetImageMetaMessage->Pack();
        *size = this->GetImageMetaMessage->GetPackSize();
        *igtlMsg = this->GetImageMetaMessage->GetPackPointer();
        return 1;
        }
      else if (qnode->GetQueryType() == vtkMRMLIGTLQueryNode::TYPE_START)
        {
        *size = 0;
        return 0;
        }
      else if (qnode->GetQueryType() == vtkMRMLIGTLQueryNode::TYPE_STOP)
        {
        *size = 0;
        return 0;
        }
      return 0;
      }
    else
      {
      return 0;
      }
    }

  // If mrmlNode is data node
  if (event == vtkMRMLVolumeNode::ImageDataModifiedEvent)
    {
    return 1;
    }
  else
    {
    return 0;
    }

}



