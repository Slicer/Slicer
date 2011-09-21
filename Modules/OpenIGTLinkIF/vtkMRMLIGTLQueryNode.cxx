/*=auto=========================================================================

Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>
#include <map>

#include "vtkObjectFactory.h"

#include "vtkMRMLIGTLQueryNode.h"
#include "vtkMRMLScene.h"

#include "igtlOSUtil.h"
#include "igtlMessageBase.h"
#include "igtlMessageHeader.h"

#include "igtl_header.h"  // to define maximum length of message name

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLIGTLQueryNode);

//----------------------------------------------------------------------------
vtkMRMLIGTLQueryNode::vtkMRMLIGTLQueryNode()
{
  this->QueryStatus = 0;
  this->QueryType   = 0;

  this->ConnectorNodeID = "";

  this->TimeStamp = 0.0;
  this->TimeOut   = 0.0;

  
}

//----------------------------------------------------------------------------
vtkMRMLIGTLQueryNode::~vtkMRMLIGTLQueryNode()
{
}


//----------------------------------------------------------------------------
void vtkMRMLIGTLQueryNode::WriteXML(ostream& of, int nIndent)
{
  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

  //of << " serverPort=\"" << this->ServerPort << "\" ";
  //of << " restrictDeviceName=\"" << this->RestrictDeviceName << "\" ";

}


//----------------------------------------------------------------------------
void vtkMRMLIGTLQueryNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;

  /*
  const char* serverHostname = "";
  int port = 0;
  int type = -1;
  int restrictDeviceName = 0;
  */

  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    /*
    if (!strcmp(attName, "connectorType"))
      {
      if (!strcmp(attValue, "SERVER"))
        {
        type = TYPE_SERVER;
        }
      else if (!strcmp(attValue, "CLIENT"))
        {
        type = TYPE_CLIENT;
        }
      else
        {
        type = TYPE_NOT_DEFINED;
        }
      }
    if (!strcmp(attName, "serverHostname"))
      {
      serverHostname = attValue;
      }
    if (!strcmp(attName, "serverPort"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> port;
      }
    if (!strcmp(attName, "restrictDeviceName"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> restrictDeviceName;;
      }
    */
    }

  /*
  switch(type)
    {
    case TYPE_SERVER:
      this->SetTypeServer(port);
      this->SetRestrictDeviceName(restrictDeviceName);
      break;
    case TYPE_CLIENT:
      this->SetTypeClient(serverHostname, port);
      this->SetRestrictDeviceName(restrictDeviceName);
      break;
    default: // not defined
      // do nothing
      break;
    }
  */
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLIGTLQueryNode::Copy(vtkMRMLNode *anode)
{

  Superclass::Copy(anode);
  //vtkMRMLIGTLQueryNode *node = (vtkMRMLIGTLQueryNode *) anode;

  /*
  int type = node->GetType();
  
  switch(type)
    {
    case TYPE_SERVER:
      this->SetType(TYPE_SERVER);
      this->SetTypeServer(node->GetServerPort());
      this->SetRestrictDeviceName(node->GetRestrictDeviceName());
      break;
    case TYPE_CLIENT:
      this->SetType(TYPE_CLIENT);
      this->SetTypeClient(node->GetServerHostname(), node->GetServerPort());
      this->SetRestrictDeviceName(node->GetRestrictDeviceName());
      break;
    default: // not defined
      // do nothing
      this->SetType(TYPE_NOT_DEFINED);
      break;
    }
  */

}


//----------------------------------------------------------------------------
void vtkMRMLIGTLQueryNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  /*
  MRMLNodeListType::iterator iter;
  for (iter = this->OutgoingMRMLNodeList.begin(); iter != this->OutgoingMRMLNodeList.end(); iter ++)
    {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(caller);
    if (node == *iter)
      {
      int size;
      void* igtlMsg;
      vtkIGTLToMRMLBase* converter = this->MRMLIDToConverterMap[node->GetID()];
      if (converter->MRMLToIGTL(event, node, &size, &igtlMsg))
        {
        int r = this->SendData(size, (unsigned char*)igtlMsg);
        if (r == 0)
          {
          // TODO: error handling
          //std::cerr << "ERROR: send data." << std::endl;
          }
        return;
        }
      }
    }
  */
}


//----------------------------------------------------------------------------
void vtkMRMLIGTLQueryNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkMRMLIGTLQueryNode::SetIGTLName(const char* name)
{
  char buf[IGTL_HEADER_DEVSIZE+1];
  buf[IGTL_HEADER_DEVSIZE] = '\0';
  strncpy(buf, name, IGTL_HEADER_DEVSIZE);
  this->IGTLName = buf;

}


//----------------------------------------------------------------------------
const char* vtkMRMLIGTLQueryNode::GetErrorString()
{
  return "";

}
