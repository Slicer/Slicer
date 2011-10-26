/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTimeSeriesBundleNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
// .NAME vtkMRMLTimeSeriesBundleNode - MRML node for representing 
// a linear transformation to the parent node
// .SECTION Description
// MRML node for representing 
// a linear transformation to the parent node in the form vtkMatrix4x4
// MatrixTransformToParent

#ifndef __vtkMRMLTimeSeriesBundleNode_h
#define __vtkMRMLTimeSeriesBundleNode_h

#include "vtkFourDImageWin32Header.h"

#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScalarVolumeNode.h"

class vtkMRMLStorageNode;



class VTK_FourDImage_EXPORT vtkMRMLTimeSeriesBundleNode : public vtkMRMLLinearTransformNode
{
  public:
  typedef struct {
    unsigned int second;
    unsigned int nanosecond;
  } TimeStamp;

  static vtkMRMLTimeSeriesBundleNode *New();
  vtkTypeMacro(vtkMRMLTimeSeriesBundleNode,vtkMRMLLinearTransformNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "TimeSeriesBundle";};

  // Description:
  // alternative method to propagate events generated in Transform nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );

  virtual bool CanApplyNonLinearTransform() { return true; } 
 
  // Description:
  // Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode()
    {
    return Superclass::CreateDefaultStorageNode();
    };

  int GetNumberOfFrames();
  int InsertFrame(int i, const char* nodeID, TimeStamp* ts = NULL);
  int InsertFrame(int i, const char* nodeID, unsigned int second, unsigned int microsecond);
  int AddFrame(const char* nodeID, TimeStamp* ts = NULL);
  int AddFrame(const char* nodeID, unsigned int second, unsigned int microsecond);
  int RemoveFrame(int i);              // Delete a frame by index number (not remove from the scene)
  int RemoveFrame(const char* nodeID); // Delete a frame by node ID (not remove from the scene)
  void RemoveAllFrames();

  vtkMRMLNode* GetFrameNode(int i);

  int          GetTimeStamp(int i, TimeStamp* ts);
  int          SetTimeStamp(int i, TimeStamp* ts);

  int          SetTimeStamp(int i, unsigned int second, unsigned int microsecond);
  unsigned int GetTimeStampSecondComponent(int i);
  unsigned int GetTimeStampMicrosecondComponent(int i);

  int          SetDisplayBufferNodeID(int bufferIndex, const char* nodeID);
  vtkMRMLNode* GetDisplayBufferNode(int bufferIndex);
  void         SwitchDisplayBuffer(int bufferIndex, int i);
  
protected:
  vtkMRMLTimeSeriesBundleNode();
  ~vtkMRMLTimeSeriesBundleNode();
  vtkMRMLTimeSeriesBundleNode(const vtkMRMLTimeSeriesBundleNode&);
  void operator=(const vtkMRMLTimeSeriesBundleNode&);

  typedef std::vector<std::string> NodeIDListType;
  typedef std::vector<TimeStamp> TimeStampListType;

  NodeIDListType    FrameNodeIDList;
  //NodeIDListType    TransformNodeIDList;
  NodeIDListType    DisplayBufferNodeIDList;
  TimeStampListType TimeStampList;

};

#endif

