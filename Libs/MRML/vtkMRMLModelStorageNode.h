/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLModelStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLModelStorageNode - MRML node for model storage on disk
// .SECTION Description
// Storage nodes has methods to read/write vtkPolyData to/from disk

#ifndef __vtkMRMLModelStorageNode_h
#define __vtkMRMLModelStorageNode_h

#include "vtkMRML.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLModelNode.h" 

#include "itkMesh.h"
#include "itkTriangleCell.h"
#include "itkQuadrilateralCell.h"
#include "itkDefaultDynamicMeshTraits.h"
#include "itkMeshSpatialObject.h"
#include "itkSpatialObjectReader.h"
#include "itkSpatialObjectWriter.h"

typedef itk::DefaultDynamicMeshTraits< vtkFloatingPointType , 3, 3, double > MeshTrait;
typedef itk::Mesh<vtkFloatingPointType,3,MeshTrait> floatMesh;

/** Hold on to the type information specified by the template parameters. */
typedef  floatMesh::Pointer             MeshPointer;
typedef  MeshTrait::PointType           MeshPointType;
typedef  MeshTrait::PixelType           MeshPixelType;  

/** Some convenient typedefs. */
typedef  floatMesh::Pointer              MeshPointer;
typedef  floatMesh::CellTraits           CellTraits;
typedef  floatMesh::PointsContainerPointer PointsContainerPointer;
typedef  floatMesh::PointsContainer      PointsContainer;
typedef  floatMesh::CellsContainerPointer CellsContainerPointer;
typedef  floatMesh::CellsContainer       CellsContainer;
typedef  floatMesh::PointType            PointType;
typedef  floatMesh::CellType             CellType;
typedef  itk::TriangleCell<CellType>   TriangleType;

typedef itk::MeshSpatialObject<floatMesh> MeshSpatialObjectType;
typedef itk::SpatialObjectReader<3,vtkFloatingPointType,MeshTrait> MeshReaderType;
typedef itk::SpatialObjectWriter<3,vtkFloatingPointType,MeshTrait> MeshWriterType;

class vtkImageData;

class VTK_MRML_EXPORT vtkMRMLModelStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLModelStorageNode *New();
  vtkTypeMacro(vtkMRMLModelStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Set dependencies between this node and the parent node
  // when parsing XML file
  virtual void ProcessParentNode(vtkMRMLNode *parentNode);

  // Description:
  // Read data and set it in the referenced node
  // NOTE: Subclasses should implement this method
  virtual int ReadData(vtkMRMLNode *refNode);

  // Description:
  // Write data from a  referenced node
  // NOTE: Subclasses should implement this method
  virtual int WriteData(vtkMRMLNode *refNode);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "ModelStorage";};

  // Description:
  // Check to see if this storage node can handle the file type in the input
  // string. If input string is null, check URI, then check FileName. 
  // Subclasses should implement this method.
  virtual int SupportedFileType(const char *fileName);

protected:
  vtkMRMLModelStorageNode();
  ~vtkMRMLModelStorageNode();
  vtkMRMLModelStorageNode(const vtkMRMLModelStorageNode&);
  void operator=(const vtkMRMLModelStorageNode&);

};

#endif

