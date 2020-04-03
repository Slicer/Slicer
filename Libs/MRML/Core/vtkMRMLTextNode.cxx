/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTextNode.cxx,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

// SlicerOpenIGTLink MRML includes
#include "vtkMRMLTextNode.h"
#include "vtkMRMLTextStorageNode.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include "vtkObjectFactory.h"
#include "vtkXMLUtilities.h"

const int MAX_STRING_LENGTH_FOR_SAVE_WITHOUT_STORAGE_NODE = 256;

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTextNode);

//-----------------------------------------------------------------------------
vtkMRMLTextNode::vtkMRMLTextNode()
  : Text("")
{
  this->ContentModifiedEvents->InsertNextValue(vtkMRMLTextNode::TextModifiedEvent);
}

//-----------------------------------------------------------------------------
vtkMRMLTextNode::~vtkMRMLTextNode()
{
  this->SetText("");
}

//----------------------------------------------------------------------------
void vtkMRMLTextNode::SetText(const std::string &text, int encoding/*-1*/)
{
  vtkDebugMacro( << this->GetClassName() << " (" << this << "): setting Text to " << text);

  int wasModifying = this->StartModify();
  if (encoding >= 0)
    {
    this->SetEncoding(encoding);
    }
  if (this->Text == text)
    {
    this->EndModify(wasModifying);
    return;
    }
  this->Text = text;
  this->InvokeCustomModifiedEvent(vtkMRMLTextNode::TextModifiedEvent);
  this->Modified();
  this->EndModify(wasModifying);
}

//----------------------------------------------------------------------------
void vtkMRMLTextNode::SetEncoding(int encoding)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting encoding to " << encoding);
  int clampedEncoding = std::max(VTK_ENCODING_NONE, std::min(encoding, VTK_ENCODING_UNKNOWN));
  if (this->Encoding != clampedEncoding)
    {
    this->Encoding = clampedEncoding;
    this->InvokeCustomModifiedEvent(vtkMRMLTextNode::TextModifiedEvent);
    this->Modified();
    }
}

//----------------------------------------------------------------------------
std::string vtkMRMLTextNode::GetEncodingAsString()
{
  switch (this->Encoding)
    {
    case VTK_ENCODING_NONE:
      return "None";
    case VTK_ENCODING_US_ASCII:
      return "ASCII";
    case VTK_ENCODING_UNICODE:
      return "Unicode";
    case VTK_ENCODING_UTF_8:
      return "UTF-8";
    case VTK_ENCODING_ISO_8859_1:
    case VTK_ENCODING_ISO_8859_2:
    case VTK_ENCODING_ISO_8859_3:
    case VTK_ENCODING_ISO_8859_4:
    case VTK_ENCODING_ISO_8859_5:
    case VTK_ENCODING_ISO_8859_6:
    case VTK_ENCODING_ISO_8859_7:
    case VTK_ENCODING_ISO_8859_8:
    case VTK_ENCODING_ISO_8859_10:
    case VTK_ENCODING_ISO_8859_11:
    case VTK_ENCODING_ISO_8859_12:
    case VTK_ENCODING_ISO_8859_13:
    case VTK_ENCODING_ISO_8859_14:
    case VTK_ENCODING_ISO_8859_15:
    case VTK_ENCODING_ISO_8859_16:
      return "ISO-8859-" + vtkVariant(this->Encoding - VTK_ENCODING_ISO_8859_1 + 1).ToString();
    }
  return "Unknown";
}

//----------------------------------------------------------------------------
void vtkMRMLTextNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  Superclass::ReadXMLAttributes(atts);
  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLStdStringMacro(text, Text);
  vtkMRMLReadXMLIntMacro(encoding, Encoding);
  vtkMRMLReadXMLEndMacro();
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLTextNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkMRMLWriteXMLBeginMacro(of);
  if (!this->GetStorageNode())
    {
    vtkMRMLWriteXMLStdStringMacro(text, Text);
    }
  vtkMRMLWriteXMLIntMacro(encoding, Encoding);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLTextNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyStringMacro(Text);
  vtkMRMLCopyIntMacro(Encoding);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLTextNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintStdStringMacro(Text);
  vtkMRMLPrintIntMacro(Encoding);
  vtkMRMLPrintEndMacro();
}

//---------------------------------------------------------------------------
std::string vtkMRMLTextNode::GetDefaultStorageNodeClassName(const char* vtkNotUsed(filename))
{
  if (!this->Scene)
    {
    return "";
    }

  if (this->ForceCreateStorageNode == CreateStorageNodeNever)
    {
    return "";
    }

  if (this->ForceCreateStorageNode == CreateStorageNodeAuto)
    {
    int length = this->Text.length();
    if (length < MAX_STRING_LENGTH_FOR_SAVE_WITHOUT_STORAGE_NODE)
      {
      return "";
      }
    }

  return "vtkMRMLTextStorageNode";
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLTextNode::CreateDefaultStorageNode()
{
  if (!this->Scene)
    {
    return nullptr;
    }

  if (!this->ForceCreateStorageNode)
    {
    int length = this->Text.length();
    if (length < MAX_STRING_LENGTH_FOR_SAVE_WITHOUT_STORAGE_NODE)
      {
      return nullptr;
      }
    }
  return vtkMRMLTextStorageNode::SafeDownCast(
    this->Scene->CreateNodeByClass(this->GetDefaultStorageNodeClassName().c_str()));
}
