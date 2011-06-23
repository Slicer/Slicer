/*==========================================================================

  Portions (c) Copyright 2008-2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#include <vtksys/SystemTools.hxx>

#include "vtkObjectFactory.h"
#include "vtkIGTLToMRMLPosition.h"

#include "vtkMRMLLinearTransformNode.h"
#include "igtlPositionMessage.h"
#include "igtlWin32Header.h"
#include "igtlMath.h"

vtkStandardNewMacro(vtkIGTLToMRMLPosition);
vtkCxxRevisionMacro(vtkIGTLToMRMLPosition, "$Revision$");


//---------------------------------------------------------------------------
vtkIGTLToMRMLPosition::vtkIGTLToMRMLPosition()
{
}


//---------------------------------------------------------------------------
vtkIGTLToMRMLPosition::~vtkIGTLToMRMLPosition()
{
}


//---------------------------------------------------------------------------
void vtkIGTLToMRMLPosition::PrintSelf(ostream& os, vtkIndent indent)
{
}


//---------------------------------------------------------------------------
vtkMRMLNode* vtkIGTLToMRMLPosition::CreateNewNode(vtkMRMLScene* scene, const char* name)
{
  vtkMRMLLinearTransformNode* transformNode;

  transformNode = vtkMRMLLinearTransformNode::New();
  transformNode->SetName(name);
  transformNode->SetDescription("Received by OpenIGTLink");

  vtkMatrix4x4* transform = vtkMatrix4x4::New();
  transform->Identity();
  //transformNode->SetAndObserveImageData(transform);
  transformNode->ApplyTransform(transform);
  transform->Delete();

  scene->AddNode(transformNode);  

  return transformNode;
}


//---------------------------------------------------------------------------
vtkIntArray* vtkIGTLToMRMLPosition::GetNodeEvents()
{
  vtkIntArray* events;

  events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);

  return events;
}


//---------------------------------------------------------------------------
int vtkIGTLToMRMLPosition::IGTLToMRML(igtl::MessageBase::Pointer buffer, vtkMRMLNode* node)
{
  // Create a message buffer to receive transform data
  igtl::PositionMessage::Pointer positionMsg;
  positionMsg = igtl::PositionMessage::New();
  positionMsg->Copy(buffer);  // !! TODO: copy makes performance issue.

  // Deserialize the transform data
  // If CheckCRC==0, CRC check is skipped.
  int c = positionMsg->Unpack(this->CheckCRC);

  if (!(c & igtl::MessageHeader::UNPACK_BODY)) // if CRC check fails
    {
    // TODO: error handling
    return 0;
    }

  if (node == NULL)
    {
    return 0;
    }

  vtkMRMLLinearTransformNode* transformNode = 
    vtkMRMLLinearTransformNode::SafeDownCast(node);

  float position[3];
  float quaternion[4];
  igtl::Matrix4x4 matrix;
  positionMsg->GetPosition(position);
  positionMsg->GetQuaternion(quaternion);

  igtl::QuaternionToMatrix(quaternion, matrix);
  
  vtkMatrix4x4* transformToParent = transformNode->GetMatrixTransformToParent();
  int row, column;
  for (row = 0; row < 3; row++)
    {
    for (column = 0; column < 3; column++)
      {
      transformToParent->Element[row][column] = matrix[row][column];
      }
    transformToParent->Element[row][3] = position[row];
    }
  transformToParent->Element[3][0] = 0.0;
  transformToParent->Element[3][1] = 0.0;
  transformToParent->Element[3][2] = 0.0;
  transformToParent->Element[3][3] = 1.0;

  transformToParent->Modified();

  return 1;
}


//---------------------------------------------------------------------------
int vtkIGTLToMRMLPosition::MRMLToIGTL(unsigned long event, vtkMRMLNode* mrmlNode, int* size, void** igtlMsg)
{

  if (mrmlNode && event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
    vtkMRMLLinearTransformNode* transformNode =
      vtkMRMLLinearTransformNode::SafeDownCast(mrmlNode);
    vtkMatrix4x4* matrix = transformNode->GetMatrixTransformToParent();
    
    //igtl::PositionMessage::Pointer OutPositionMsg;
    if (this->OutPositionMsg.IsNull())
      {
      this->OutPositionMsg = igtl::PositionMessage::New();
      }
    
    this->OutPositionMsg->SetDeviceName(mrmlNode->GetName());

    igtl::Matrix4x4 igtlmatrix;
    
    igtlmatrix[0][0]  = matrix->Element[0][0];
    igtlmatrix[1][0]  = matrix->Element[1][0];
    igtlmatrix[2][0]  = matrix->Element[2][0];
    igtlmatrix[3][0]  = matrix->Element[3][0];
    igtlmatrix[0][1]  = matrix->Element[0][1];
    igtlmatrix[1][1]  = matrix->Element[1][1];
    igtlmatrix[2][1]  = matrix->Element[2][1];
    igtlmatrix[3][1]  = matrix->Element[3][1];
    igtlmatrix[0][2]  = matrix->Element[0][2];
    igtlmatrix[1][2]  = matrix->Element[1][2];
    igtlmatrix[2][2]  = matrix->Element[2][2];
    igtlmatrix[3][2]  = matrix->Element[3][2];
    igtlmatrix[0][3]  = matrix->Element[0][3];
    igtlmatrix[1][3]  = matrix->Element[1][3];
    igtlmatrix[2][3]  = matrix->Element[2][3];
    igtlmatrix[3][3]  = matrix->Element[3][3];

    float position[3];
    float quaternion[4];
    
    position[0] = igtlmatrix[0][3];
    position[1] = igtlmatrix[1][3];
    position[2] = igtlmatrix[2][3];
    igtl::MatrixToQuaternion(igtlmatrix, quaternion);
    
    //this->OutPositionMsg->SetMatrix(igtlmatrix);
    this->OutPositionMsg->SetPosition(position);
    this->OutPositionMsg->SetQuaternion(quaternion);
    this->OutPositionMsg->Pack();

    *size = this->OutPositionMsg->GetPackSize();
    *igtlMsg = (void*)this->OutPositionMsg->GetPackPointer();

    return 1;
    }

  return 0;

}

