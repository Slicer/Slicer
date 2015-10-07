/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLParser.cxx,v $
Date:      $Date: 2006/03/11 19:51:14 $
Version:   $Revision: 1.8 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLParser.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLNode.h"
#include "vtkTagTable.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkObjectFactory.h>
#include <vtkStdString.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLParser);

//------------------------------------------------------------------------------
void vtkMRMLParser::StartElement(const char* tagName, const char** atts)
{
  if (!strcmp(tagName, "MRML"))
    {
    //--- BEGIN test of user tags
    //--- pull out any tags describing the scene and fill the scene's tagtable.
    const char* attName;
    const char* attValue;
    while (*atts != NULL)
      {
      attName = *(atts++);
      attValue = *(atts++);
      if (!strcmp(attName, "version"))
        {
        this->GetMRMLScene()->SetLastLoadedVersion(attValue);
        }
      else if (!strcmp(attName, "userTags"))
        {
        if ( this->MRMLScene->GetUserTagTable() == NULL )
          {
          //--- null table, no tags are read.
          return;
          }
        std::stringstream ss(attValue);
        std::string kwd = "";
        std::string val = "";
        std::string::size_type i;
        while (!ss.eof())
          {
          std::string tags;
          ss >> tags;
          //--- now pull apart individual tags
          if ( tags.c_str() != NULL )
            {
            i = tags.find("=");
            if ( i != std::string::npos)
              {
              kwd = tags.substr(0, i);
              val = tags.substr(i+1, std::string::npos );
              if ( kwd.c_str() != NULL && val.c_str() != NULL )
                {
                this->MRMLScene->GetUserTagTable()->AddOrUpdateTag ( kwd.c_str(), val.c_str(), 0 );
                }
              }
            }
          }
        } //--- END test of user tags.
      } // while
    return;
    } // MRML

  const char* tmp = this->MRMLScene->GetClassNameByTag(tagName);
  std::string className = tmp ? tmp : "";

  // CreateNodeByClass should have a chance to instantiate non-registered node
  if (className.empty())
    {
    className = "vtkMRML";
    className += tagName;
    // Append 'Node' prefix only if required
    if (className.find("Node") != className.size() - 4)
      {
      className += "Node";
      }
    }

  vtkMRMLNode* node = this->MRMLScene->CreateNodeByClass( className.c_str() );
  if (!node)
    {
    vtkWarningMacro(<< "Failed to CreateNodeByClass: " << className);
    return;
    }

  // It is needed to have the scene set before ReadXMLAttributes is
  // called on storage nodes.
  if (vtkMRMLStorageNode::SafeDownCast(node) != 0)
    {
    node->SetScene(this->GetMRMLScene());
    }

  node->ReadXMLAttributes(atts);

  // Slicer3 snap shot nodes were hidden by default, show them so that
  // they show up in the tree views
#if MRML_SUPPORT_VERSION < 0x040000
  if (strcmp(tagName, "SceneSnapshot") == 0)
    {
    node->HideFromEditorsOff();
    }
#endif

  // Replace old-style label map nodes (vtkMRMLScalarVolumeNode with LabelMap custom attribute)
  // with new-style vtkMRMLLabelMapVolumeNode
  if (node->IsA("vtkMRMLScalarVolumeNode"))
    {
    const char* labelMapAttr = node->GetAttribute("LabelMap");
    bool isLabelMap = labelMapAttr ? (atoi(labelMapAttr)!=0) : false;
    if (isLabelMap)
      {
      // create a copy of the node of the correct class
      vtkMRMLNode* newTypeLabelMapNode = this->MRMLScene->CreateNodeByClass( "vtkMRMLLabelMapVolumeNode" );
      newTypeLabelMapNode->CopyWithScene(node); // copy all contents, including MRML node ID
      newTypeLabelMapNode->RemoveAttribute("LabelMap"); // this attribute is obsolete
      // replace the current node with the new one
      node->Delete();
      node=newTypeLabelMapNode;
      }
    }

  // ID will be set by AddNode
  /*
  if (node->GetID() == NULL)
    {
    node->SetID(this->MRMLScene->GetUniqueIDByClass(className));
    }
  */
  if (!this->NodeStack.empty())
    {
    vtkMRMLNode* parentNode = this->NodeStack.top();
    parentNode->ProcessChildNode(node);
    }

  this->NodeStack.push(node);

  if (this->NodeCollection)
    {
    if (node->GetAddToScene())
      {
      this->NodeCollection->vtkCollection::AddItem((vtkObject *)node);
      }
    }
  else
    {
    this->MRMLScene->AddNode(node);
    }
  node->Delete();
}

//-----------------------------------------------------------------------------

void vtkMRMLParser::EndElement (const char *name)
{
  if ( !strcmp(name, "MRML") || this->NodeStack.empty() )
    {
    return;
    }

  const char* className = this->MRMLScene->GetClassNameByTag(name);
  if (className == NULL)
    {
    // check for a renamed node
    if (strcmp(name, "SceneSnapshot") == 0)
      {
      className = this->MRMLScene->GetClassNameByTag("SceneView");
      if (className == NULL)
        {
        return;
        }
      }
    else
      {
      return;
      }
    }

  this->NodeStack.pop();
}
