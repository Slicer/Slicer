/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLModelStorageNode.cxx,v $
  Date:      $Date: 2006/03/17 15:10:09 $
  Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkBYUReader.h>
#include <vtkCellArray.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPLYWriter.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkSTLReader.h>
#include <vtkSTLWriter.h>
#include <vtkStringArray.h>
#include <vtksys/SystemTools.hxx>
#include <vtkTriangleFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkVersion.h>

// ITK includes
#include <itkDefaultDynamicMeshTraits.h>
#include <itkSpatialObjectReader.h>
#include <itkSpatialObjectWriter.h>

typedef itk::DefaultDynamicMeshTraits< double , 3, 3, double > MeshTrait;
typedef itk::Mesh<double,3,MeshTrait> floatMesh;

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
typedef itk::SpatialObjectReader<3,double,MeshTrait> MeshReaderType;
typedef itk::SpatialObjectWriter<3,double,MeshTrait> MeshWriterType;



// Initialize static member that controls resampling --
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Slicer
// development meeting, to move ijk coordinates to voxel centers."

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLModelStorageNode);

//----------------------------------------------------------------------------
vtkMRMLModelStorageNode::vtkMRMLModelStorageNode()
{
  this->DefaultWriteFileExtension = "vtk";
}

//----------------------------------------------------------------------------
vtkMRMLModelStorageNode::~vtkMRMLModelStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLModelStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLModelNode");
}

//----------------------------------------------------------------------------
int vtkMRMLModelStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLModelNode *modelNode = dynamic_cast <vtkMRMLModelNode *> (refNode);

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("ReadDataInternal: File name not specified");
    return 0;
    }

  // check that the file exists
  if (vtksys::SystemTools::FileExists(fullName.c_str()) == false)
    {
    vtkErrorMacro("ReadDataInternal: model file '" << fullName.c_str() << "' not found.");
    return 0;
    }

  // compute file prefix
  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if( extension.empty() )
    {
    vtkErrorMacro("ReadData: no file extension specified: " << fullName.c_str());
    return 0;
    }

  vtkDebugMacro("ReadDataInternal: extension = " << extension.c_str());

  int result = 1;
  try
    {
    if ( extension == std::string(".g") || extension == std::string(".byu") )
      {
      vtkNew<vtkBYUReader> reader;
      reader->SetGeometryFileName(fullName.c_str());
      reader->Update();
      modelNode->SetPolyDataConnection(reader->GetOutputPort());
      }
    else if (extension == std::string(".vtk"))
      {
      vtkPolyData* output = 0;
      vtkNew<vtkPolyDataReader> reader;
      reader->SetFileName(fullName.c_str());
      vtkNew<vtkUnstructuredGridReader> unstructuredGridReader;
      vtkNew<vtkDataSetSurfaceFilter> surfaceFilter;

      unstructuredGridReader->SetFileName(fullName.c_str());
      if (reader->IsFilePolyData())
        {
        reader->Update();
        output = reader->GetOutput();
        }
      else if (unstructuredGridReader->IsFileUnstructuredGrid())
        {
        unstructuredGridReader->Update();
        surfaceFilter->SetInputConnection(unstructuredGridReader->GetOutputPort());
        surfaceFilter->Update();
        output = reader->GetOutput();
        }
      else
        {
        vtkErrorMacro("File " << fullName.c_str()
                      << " is not recognized as polydata nor as an unstructured grid.");
        }
      if (output == 0)
        {
        vtkErrorMacro("Unable to read file " << fullName.c_str());
        result = 0;
        }
      else
        {
        modelNode->SetPolyDataConnection(reader->GetOutputPort());
        }
      }
    else if (extension == std::string(".vtp"))
      {
      vtkNew<vtkXMLPolyDataReader> reader;
      reader->SetFileName(fullName.c_str());
      reader->Update();
      modelNode->SetPolyDataConnection(reader->GetOutputPort());
      }
    else if (extension == std::string(".stl"))
      {
      vtkNew<vtkSTLReader> reader;
      reader->SetFileName(fullName.c_str());
      reader->Update();
      modelNode->SetPolyDataConnection(reader->GetOutputPort());
      }
    else if (extension == std::string(".ply"))
      {
      vtkNew<vtkPLYReader> reader;
      reader->SetFileName(fullName.c_str());
      reader->Update();
      modelNode->SetPolyDataConnection(reader->GetOutputPort());
      }
    else if (extension == std::string(".obj"))
      {
      vtkNew<vtkOBJReader> reader;
      reader->SetFileName(fullName.c_str());
      reader->Update();
      modelNode->SetPolyDataConnection(reader->GetOutputPort());
      }
    else if (extension == std::string(".meta"))  // model in meta format
      {
      floatMesh::Pointer surfaceMesh = floatMesh::New();
      MeshReaderType::Pointer readerSH = MeshReaderType::New();
      try
        {
        readerSH->SetFileName(fullName.c_str());
        readerSH->Update();
        MeshReaderType::SceneType::Pointer scene = readerSH->GetScene();
        MeshReaderType::SceneType::ObjectListType * objList =  scene->GetObjects(1,NULL);

        MeshReaderType::SceneType::ObjectListType::iterator it = objList->begin();
        itk::SpatialObject<3> * curObj = *it;
        MeshSpatialObjectType::Pointer  SOMesh = dynamic_cast<MeshSpatialObjectType*> (curObj);
        surfaceMesh = SOMesh->GetMesh();
        }
      catch(itk::ExceptionObject ex)
        {
        std::cout<<ex.GetDescription()<<std::endl;
        result = 0;
        }
      vtkNew<vtkPolyData> vtkMesh;
      // Get the number of points in the mesh
      int numPoints = surfaceMesh->GetNumberOfPoints();
      //int numCells = surfaceMesh->GetNumberOfCells();

      // Create the vtkPoints object and set the number of points
      vtkNew<vtkPoints> vpoints;
      vpoints->SetNumberOfPoints(numPoints);
      // iterate over all the points in the itk mesh filling in
      // the vtkPoints object as we go
      floatMesh::PointsContainer::Pointer points = surfaceMesh->GetPoints();
      for(floatMesh::PointsContainer::Iterator i = points->Begin();
        i != points->End(); ++i)
        {
        // Get the point index from the point container iterator
        int idx = i->Index();
        vpoints->SetPoint(idx, const_cast<double*>(i->Value().GetDataPointer()));
        }
      vtkMesh->SetPoints(vpoints.GetPointer());

      vtkNew<vtkCellArray> cells;
      floatMesh::CellsContainerIterator itCells = surfaceMesh->GetCells()->begin();
      floatMesh::CellsContainerIterator itCellsEnd = surfaceMesh->GetCells()->end();
      for ( ; itCells != itCellsEnd; ++itCells )
        {
        floatMesh::CellTraits::PointIdIterator itPt = itCells->Value()->PointIdsBegin();
        vtkIdType ptIdList[64];
        int nPts = 0;
        for ( ; itPt != itCells->Value()->PointIdsEnd(); ++itPt )
          {
          ptIdList[nPts] = *itPt;
          nPts ++;
          }
        cells->InsertNextCell(nPts, ptIdList);
        }

      vtkMesh->SetPolys(cells.GetPointer());

      modelNode->SetAndObservePolyData(vtkMesh.GetPointer());
      }
    else
      {
      vtkDebugMacro("Cannot read model file '" << fullName.c_str() << "' (extension = " << extension.c_str() << ")");
      return 0;
      }
    }
  catch (...)
    {
    result = 0;
    }

  if (modelNode->GetPolyData() != NULL)
    {
    // is there an active scalar array?
    if (modelNode->GetDisplayNode())
      {
      double *scalarRange =  modelNode->GetPolyData()->GetScalarRange();
      if (scalarRange)
        {
        vtkDebugMacro("ReadDataInternal: setting scalar range " << scalarRange[0] << ", " << scalarRange[1]);
        modelNode->GetDisplayNode()->SetScalarRange(scalarRange);
        }
      }
    //modelNode->GetPolyData()->Modified();
    }
  return result;
}

//----------------------------------------------------------------------------
int vtkMRMLModelStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(refNode);

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLModelNode: File name not specified");
    return 0;
    }

  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);

  int result = 1;
  if (extension == ".vtk")
    {
    vtkNew<vtkPolyDataWriter> writer;
    writer->SetFileName(fullName.c_str());
    writer->SetFileType(this->GetUseCompression() ? VTK_BINARY : VTK_ASCII );
    writer->SetInputConnection( modelNode->GetPolyDataConnection() );
    try
      {
      writer->Write();
      }
    catch (...)
      {
      result = 0;
      }
    }
  else if (extension == ".vtp")
    {
    vtkNew<vtkXMLPolyDataWriter> writer;
    writer->SetFileName(fullName.c_str());
    writer->SetCompressorType(
      this->GetUseCompression() ? vtkXMLWriter::ZLIB : vtkXMLWriter::NONE);
    writer->SetDataMode(
      this->GetUseCompression() ? vtkXMLWriter::Appended : vtkXMLWriter::Ascii);
    writer->SetInputConnection( modelNode->GetPolyDataConnection() );
    try
      {
      writer->Write();
      }
    catch (...)
      {
      result = 0;
      }
    }
  else if (extension == ".stl")
    {
    vtkNew<vtkTriangleFilter> triangulator;
    vtkNew<vtkSTLWriter> writer;
    writer->SetFileName(fullName.c_str());
    writer->SetFileType(this->GetUseCompression() ? VTK_BINARY : VTK_ASCII );
    triangulator->SetInputConnection( modelNode->GetPolyDataConnection() );
    writer->SetInputConnection( triangulator->GetOutputPort() );
    try
      {
      writer->Write();
      }
    catch (...)
      {
      result = 0;
      }
    }
  else if (extension == ".ply")
    {
    vtkNew<vtkTriangleFilter> triangulator;
    vtkNew<vtkPLYWriter> writer;
    writer->SetFileName(fullName.c_str());
    writer->SetFileType(this->GetUseCompression() ? VTK_BINARY : VTK_ASCII );
    triangulator->SetInputConnection( modelNode->GetPolyDataConnection() );
    writer->SetInputConnection( triangulator->GetOutputPort() );
    try
      {
      writer->Write();
      }
    catch (...)
      {
      result = 0;
      }
    }
  else
    {
    result = 0;
    vtkErrorMacro( << "No file extension recognized: " << fullName.c_str() );
    }

  return result;
}

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Poly Data (.vtk)");
  this->SupportedReadFileTypes->InsertNextValue("Poly Data (.vtp)");
  this->SupportedReadFileTypes->InsertNextValue("vtkXMLPolyDataReader (.g)");
  this->SupportedReadFileTypes->InsertNextValue("BYU (.byu)");
  this->SupportedReadFileTypes->InsertNextValue("vtkXMLPolyDataReader (.meta)");
  this->SupportedReadFileTypes->InsertNextValue("STL (.stl)");
  this->SupportedReadFileTypes->InsertNextValue("PLY (.ply)");
  this->SupportedReadFileTypes->InsertNextValue("Wavefront OBJ (.obj)");
}

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::InitializeSupportedWriteFileTypes()
{
  // Look at WriteData(), .g and .meta are not being written even though
  // SupportedFileType() says they are supported
  this->SupportedWriteFileTypes->InsertNextValue("Poly Data (.vtk)");
  this->SupportedWriteFileTypes->InsertNextValue("XML Poly Data (.vtp)");
  //
  //this->SupportedWriteFileTypes->InsertNextValue("vtkXMLPolyDataReader (.g)");
  //this->SupportedWriteFileTypes->InsertNextValue("vtkXMLPolyDataReader (.meta)");
  this->SupportedWriteFileTypes->InsertNextValue("STL (.stl)");
  this->SupportedWriteFileTypes->InsertNextValue("PLY (.ply)");
}
