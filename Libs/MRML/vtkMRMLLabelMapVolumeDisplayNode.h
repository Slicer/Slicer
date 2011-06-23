/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLLabelMapVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkMRMLLabelMapVolumeDisplayNode - MRML node for representing a volume display attributes
/// 
/// vtkMRMLLabelMapVolumeDisplayNode nodes describe how volume is displayed.

#ifndef __vtkMRMLLabelMapVolumeDisplayNode_h
#define __vtkMRMLLabelMapVolumeDisplayNode_h

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLVolumeDisplayNode.h"

#include "vtkImageData.h"
#include "vtkImageMapToColors.h"
#include <vtkLookupTable.h>
#include <cassert>

class VTK_MRML_EXPORT vtkMRMLLabelMapVolumeDisplayNode : public vtkMRMLVolumeDisplayNode
{
  public:
  static vtkMRMLLabelMapVolumeDisplayNode *New();
  vtkTypeMacro(vtkMRMLLabelMapVolumeDisplayNode,vtkMRMLVolumeDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "LabelMapVolumeDisplay";};

  /// 
  /// alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
  /// 
  /// set gray colormap
  virtual void SetDefaultColorMap();

  virtual void UpdateImageDataPipeline();

  /// 
  /// Sets ImageData to be converted to lables
  void SetImageData(vtkImageData *imageData) 
    {
    this->UpdateImageDataPipeline();
    this->MapToColors->SetInput( imageData);
    };

  /// 
  /// Gets ImageData converted from the real data in the node
  virtual vtkImageData* GetImageData() 
    {
    if (this->MapToColors->GetInput() == NULL)
      {
      return NULL;
      }
    assert(!this->MapToColors->GetLookupTable() ||
           !this->MapToColors->GetLookupTable()->IsA("vtkLookupTable") ||
           vtkLookupTable::SafeDownCast(this->MapToColors->GetLookupTable())->GetNumberOfTableValues());
    this->MapToColors->Update();
    return this->MapToColors->GetOutput();
    };

protected:
  vtkMRMLLabelMapVolumeDisplayNode();
  ~vtkMRMLLabelMapVolumeDisplayNode();
  vtkMRMLLabelMapVolumeDisplayNode(const vtkMRMLLabelMapVolumeDisplayNode&);
  void operator=(const vtkMRMLLabelMapVolumeDisplayNode&);

  vtkImageMapToColors *MapToColors;

};

#endif

