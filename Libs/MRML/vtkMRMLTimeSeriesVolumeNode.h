/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTimeSeriesVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
// .NAME vtkMRMLTimeSeriesVolumeNode - MRML node for representing timeseries data
// .SECTION Description
// Time Series Volume Node describes time series datasets, e.g. fMRI data
// dynamic contrast enhanced MR, CT cardiac data, etc.  A primary goal of this class
// is to provide fast access to extremely large data sets, in theory several
// times larger than main memory.  To accomplish this goal, data will be read
// from the primary volumes and cached on the local file system.
//
// Several modes of access will be available to Slicer.
// 
// <b>Linear access</b>
// The most straightforward
// use of this class is to fetch each volume one at a time in serial order.
// This use is facilitated by these functions:
// \li <tt>GetImageData()</tt> - returns the current volume
// \li <tt>GetImageData(int idx)</tt> - returns the volume at index \c idx
// \li <tt>GetImageDataAndIncrement()</tt> - returns the current volume, and increments the index
// \li <tt>SetImageIndex ( int idx )</tt> - set the index of the next volume to return
// \li <tt>PrefetchImage ( int idx )</tt> - background prefetch the indicated volume
//
// <b>Slice access</b>
// Data may also be accessed by slice, mainly for display purposes.  Slices may be accessed
// in any orientation.  A slice may be selected from the current volume, or from an arbitrary
// volume in the timeseries.
// \li <tt>GetSlice(int s)</tt> - get the specified slice from the current volume
// \li <tt>GetSlice(int s, int v)</tt> - get the specified slice from the specified volume
// \li <tt>GetSlice(int s, int v, int o)</tt> - get a specified slice in the requested orientation
// \li <tt>GetSliceVolume(int s[, int o])</tt> - returns a volume composed of slices from each index, may be oriented
//
// <b>Voxel access</b>
// Perhaps of most use in fMRI, individual time series may be access for each pixel.
// \li <tt>GetVoxelData(int x, int y, int z)</tt> - return in a vtkDoubleArray, the voxel time course
// \li <tt>GetNextVoxelData(int &x, int &y, int &z)</tt> - walk the voxels in the most efficient manner, returns NULL when finished
// \li <tt>ResetVoxelIndex()</tt> - restart the voxel walk
//
// If voxels are walked using ResetVoxelIndex and GetNextVoxelData, the class
// will prefetch voxel data and enable rapid processing of all the image
// data.  This will be the most efficient method of accessing all the voxel 
// data will likely look something like this:
//
// \code
// vtkDoubleArray* voxelTS;
// vtkImageData* volume = vtkImageData::New();
// volume->SetDimensions ( x, y, z );
// volume->SetScalarTypeToDouble();
// volume->AllocateScalars();
//
// node->ResetVoxelIndex();
// while ( (voxelTS = node->GetNextVoxelData ( x, y, z ) ) != NULL )
// {
//   double v = SomeProcessing ( voxelTS );
//   volume->SetScalarComponentFromDouble ( x, y, z, 0, v );
// }
//


#ifndef __vtkMRMLTimeSeriesVolumeNode_h
#define __vtkMRMLTimeSeriesVolumeNode_h


#include "vtkMRMLVolumeNode.h"

class vtkImageData;
class vtkDoubleArray;

class VTK_MRML_EXPORT vtkMRMLTimeSeriesVolumeNode : public vtkMRMLVolumeNode
{
  public:
  static vtkMRMLTimeSeriesVolumeNode *New();
  vtkTypeMacro(vtkMRMLTimeSeriesVolumeNode,vtkMRMLVolumeNode);
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
  virtual const char* GetNodeTagName() {return "TimeSeries";};

  // Description:
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  virtual void UpdateReferences()
    { Superclass::UpdateReferences(); };

 // Description:
 // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID) 
    { Superclass::UpdateReferenceID(oldID, newID); };

protected:
  vtkMRMLTimeSeriesVolumeNode();
  ~vtkMRMLTimeSeriesVolumeNode();
  vtkMRMLTimeSeriesVolumeNode(const vtkMRMLTimeSeriesVolumeNode&);
  void operator=(const vtkMRMLTimeSeriesVolumeNode&);
};

#endif


 

