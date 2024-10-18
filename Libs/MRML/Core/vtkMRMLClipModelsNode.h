/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLClipModelsNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLClipModelsNode_h
#define __vtkMRMLClipModelsNode_h

#include "vtkMRMLClipNode.h"

/// \brief MRML node to represent three clipping planes.
///
/// The vtkMRMLClipModelsNode MRML node stores
/// the direction of clipping for each of the three clipping planes.
/// It also stores the type of combined clipping operation as either an
/// intersection or union.
class VTK_MRML_EXPORT vtkMRMLClipModelsNode : public vtkMRMLClipNode
{
public:
  static vtkMRMLClipModelsNode* New();
  vtkTypeMacro(vtkMRMLClipModelsNode, vtkMRMLClipNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLClipNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "ClipModels"; }

  //@{
  /// Indicates if the Red slice clipping is Off, Positive space, or Negative space
  int GetRedSliceClipState();
  void SetRedSliceClipState(int);
  //@}

  //@{
  /// Indicates if the Yellow slice clipping is Off, Positive space, or Negative space
  int GetYellowSliceClipState();
  void SetYellowSliceClipState(int);
  //@}

  //@{
  /// Indicates if the Green slice clipping is Off, Positive space, or Negative space
  int GetGreenSliceClipState();
  void SetGreenSliceClipState(int);
  //@}

protected:
  vtkMRMLClipModelsNode();
  ~vtkMRMLClipModelsNode() override;
  vtkMRMLClipModelsNode(const vtkMRMLClipModelsNode&);
  void operator=(const vtkMRMLClipModelsNode&);

  int GetSliceClipState(const char* nodeID);
  void SetSliceClipState(const char* nodeID, int state);
};

#endif
