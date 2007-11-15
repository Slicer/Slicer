/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLLabelStatisticsNode.h,v $
  Date:      $Date: 2007/08/30 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLLabelStatisticsNode_h
#define __vtkMRMLLabelStatisticsNode_h

#include <list>

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkLabelStatistics.h"


class vtkImageData;

class VTK_LABELSTATISTICS_EXPORT vtkMRMLLabelStatisticsNode : public vtkMRMLNode
{
  public:
 //BTX
  typedef struct LabelStatsEntry{
    int Label;
    int Count;
    int Min;
    int Max;
    double Mean;
    double StdDev;
  } LabelStatsEntry;
 
  //  LabelStatsEntry* LabelStats;
  std::list <LabelStatsEntry> LabelStats;

  //ETX
 
  static vtkMRMLLabelStatisticsNode *New();
  vtkTypeMacro(vtkMRMLLabelStatisticsNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Save LabelStatistics result to text file
  virtual void SaveResultToTextFile( const char *fileName );
 
  // Description:
  // Create instance of a LabelStatistics node.
  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes from name/value pairs
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get unique node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "LabelStatisticsParameters";};

 
  // Description:
  // Get/Set input grayscale volume MRML Id
  vtkGetStringMacro(InputGrayscaleRef);
  vtkSetStringMacro(InputGrayscaleRef);
  
  // Description:
  // Get/Set input labelmap volume MRML Id

  vtkGetStringMacro(InputLabelmapRef);
  vtkSetStringMacro(InputLabelmapRef);

  // Description:
  // Get/Set result text
  vtkGetStringMacro(ResultText);
  vtkSetStringMacro(ResultText);

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

 
protected:

   vtkMRMLLabelStatisticsNode();
  ~vtkMRMLLabelStatisticsNode();
  vtkMRMLLabelStatisticsNode(const vtkMRMLLabelStatisticsNode&);
  void operator=(const vtkMRMLLabelStatisticsNode&);

  char* InputGrayscaleRef;
  char* InputLabelmapRef;
  char* ResultText;
  
 
};

#endif

