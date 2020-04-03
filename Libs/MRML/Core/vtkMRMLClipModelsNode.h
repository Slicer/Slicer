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

#include "vtkMRMLNode.h"

/// \brief MRML node to represent three clipping planes.
///
/// The vtkMRMLClipModelsNode MRML node stores
/// the direction of clipping for each of the three clipping planes.
/// It also stores the type of combined clipping operation as either an
/// intersection or union.
class VTK_MRML_EXPORT vtkMRMLClipModelsNode : public vtkMRMLNode
{
public:
  static vtkMRMLClipModelsNode *New();
  vtkTypeMacro(vtkMRMLClipModelsNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLClipModelsNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "ClipModels";}

  ///
  /// Indicates the type of clipping
  /// "Intersection" or "Union"
  vtkGetMacro(ClipType, int);
  vtkSetMacro(ClipType, int);

  enum
    {
      ClipIntersection = 0,
      ClipUnion = 1
    };

  ///
  /// Indicates if the Red slice clipping is Off,
  /// Positive space, or Negative space
  vtkGetMacro(RedSliceClipState, int);
  vtkSetMacro(RedSliceClipState, int);

  ///
  /// Indicates if the Yellow slice clipping is Off,
  /// Positive space, or Negative space
  vtkGetMacro(YellowSliceClipState, int);
  vtkSetMacro(YellowSliceClipState, int);

  ///
  /// Indicates if the Green slice clipping is Off,
  /// Positive space, or Negative space
  vtkGetMacro(GreenSliceClipState, int);
  vtkSetMacro(GreenSliceClipState, int);

  enum
    {
      ClipOff = 0,
      ClipPositiveSpace = 1,
      ClipNegativeSpace = 2,
    };

  ///
  ///Indicates what clipping method should be used
  ///Straight cut, whole cell extraction, or whole cell extraction with boundary cells
  typedef enum
  {
    Straight = 0,
    WholeCells,
    WholeCellsWithBoundary,
  } ClippingMethodType;

  vtkGetMacro(ClippingMethod, ClippingMethodType);
  vtkSetMacro(ClippingMethod, ClippingMethodType);

  //Convert between enum and string
  static int GetClippingMethodFromString(const char* name);
  static const char* GetClippingMethodAsString(ClippingMethodType id);

protected:
  vtkMRMLClipModelsNode();
  ~vtkMRMLClipModelsNode() override;
  vtkMRMLClipModelsNode(const vtkMRMLClipModelsNode&);
  void operator=(const vtkMRMLClipModelsNode&);

  int ClipType;

  int RedSliceClipState;
  int YellowSliceClipState;
  int GreenSliceClipState;

  ClippingMethodType ClippingMethod;


};

#endif
