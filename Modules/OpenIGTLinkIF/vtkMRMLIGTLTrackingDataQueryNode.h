/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCurveAnalysisNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLIGTLTrackingDataQueryNode_h
#define __vtkMRMLIGTLTrackingDataQueryNode_h

#include "vtkOpenIGTLinkIFWin32Header.h"
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkIGTLToMRMLBase.h"

#include <string>
#include <map>
#include <vector>
#include <set>

#include "vtkObject.h"
#include "vtkOpenIGTLinkIFWin32Header.h" 

class VTK_OPENIGTLINKIF_EXPORT vtkMRMLIGTLTrackingDataQueryNode : public vtkMRMLNode
{
 public:

  //----------------------------------------------------------------
  // Constants Definitions
  //----------------------------------------------------------------

  // Events
  enum {
    ResponseEvent        = 128940,
  };

  enum {
    TYPE_NOT_DEFINED,
    TYPE_GET,
    TYPE_START,
    TYPE_STOP,
    NUM_TYPE,
  };

  enum {
    STATUS_NOT_DEFINED,
    STATUS_PREPARED,     // Ready to query
    STATUS_WAITING,      // Waiting for response from server
    STATUS_SUCCESS,      // Server accepted query successfuly 
    STATUS_ERROR,        // Server failed to accept query
    NUM_STATUS,
  };

 public:

  //----------------------------------------------------------------
  // Access functions
  //----------------------------------------------------------------
  vtkGetMacro( QueryStatus, int );
  vtkSetMacro( QueryStatus, int );
  vtkGetMacro( QueryType, int );
  vtkSetMacro( QueryType, int );

  //----------------------------------------------------------------
  // Standard methods for MRML nodes
  //----------------------------------------------------------------

  static vtkMRMLIGTLTrackingDataQueryNode *New();
  vtkTypeMacro(vtkMRMLIGTLTrackingDataQueryNode,vtkMRMLNode);
  
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
  { return "IGTLQuery"; };

  // method to propagate events generated in mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  // Description:
  // Get OpenIGTLink device name. If the query node is for IMAGE, "IMAGE" is returned.
  virtual void SetIGTLName(const char* name);
  virtual const char* GetIGTLName() { return IGTLName.c_str(); };

  // Description:
  // Return error message after receiving requested message.
  virtual const char* GetErrorString();

  // Description:
  // Query node saves MRML node that holds received data as response to the query.
  virtual void SetResponseDataNodeID(const char* id) { this->DataNodeID = id; };
  virtual const char* GetResponseDataNodeID()        { return this->DataNodeID.c_str(); };

 protected:
  //----------------------------------------------------------------
  // Constructor and destroctor
  //----------------------------------------------------------------
  
  vtkMRMLIGTLTrackingDataQueryNode();
  ~vtkMRMLIGTLTrackingDataQueryNode();
  vtkMRMLIGTLTrackingDataQueryNode(const vtkMRMLIGTLTrackingDataQueryNode&);
  void operator=(const vtkMRMLIGTLTrackingDataQueryNode&);

 public:
  //----------------------------------------------------------------
  // Connector configuration
  //----------------------------------------------------------------


 private:
  //----------------------------------------------------------------
  // Data
  //----------------------------------------------------------------

  std::string IGTLName;
  
  int QueryStatus;
  int QueryType;

  std::string ConnectorNodeID;
  std::string DataNodeID;

  // Time when the querry issued. 
  double TimeStamp;
  double TimeOut;
  
};

#endif

