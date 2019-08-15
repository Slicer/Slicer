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
#include <vtkActor.h>
#include <vtkBYUReader.h>
#include <vtkCellArray.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkFieldData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkOBJReader.h>
#include <vtkOBJExporter.h>
#include <vtkPolyDataMapper.h>
#include <vtkPLYReader.h>
#include <vtkPLYWriter.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSTLReader.h>
#include <vtkSTLWriter.h>
#include <vtkStringArray.h>
#include <vtksys/SystemTools.hxx>
#include <vtkTriangleFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridWriter.h>
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
= default;

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write coordinateSystem explicitly in the scene file. In the future
  // we will support saving in LPS coordinate system as well, similarly
  // to markups nodes.
  of << " coordinateSystem=\"RAS\"";
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
      vtkNew<vtkPolyDataReader> reader;
      reader->SetFileName(fullName.c_str());
      vtkNew<vtkUnstructuredGridReader> unstructuredGridReader;
      unstructuredGridReader->SetFileName(fullName.c_str());

      if (reader->IsFilePolyData())
        {
        reader->Update();
        reader->ReadAllScalarsOn();
        reader->ReadAllVectorsOn();
        reader->ReadAllNormalsOn();
        reader->ReadAllTensorsOn();
        reader->ReadAllColorScalarsOn();
        reader->ReadAllTCoordsOn();
        reader->ReadAllFieldsOn();
        modelNode->SetPolyDataConnection(reader->GetOutputPort());
        }
      else if (unstructuredGridReader->IsFileUnstructuredGrid())
        {
        unstructuredGridReader->ReadAllScalarsOn();
        unstructuredGridReader->ReadAllVectorsOn();
        unstructuredGridReader->ReadAllNormalsOn();
        unstructuredGridReader->ReadAllTensorsOn();
        unstructuredGridReader->ReadAllColorScalarsOn();
        unstructuredGridReader->ReadAllTCoordsOn();
        unstructuredGridReader->ReadAllFieldsOn();
        unstructuredGridReader->Update();
        modelNode->SetUnstructuredGridConnection(unstructuredGridReader->GetOutputPort());
        }
      else
        {
        vtkErrorMacro("File " << fullName.c_str()
                      << " is not recognized as polydata nor as an unstructured grid.");
        }
      }
    else if (extension == std::string(".vtp"))
      {
      vtkNew<vtkXMLPolyDataReader> reader;
      reader->SetFileName(fullName.c_str());
      reader->Update();
      modelNode->SetPolyDataConnection(reader->GetOutputPort());
      }
    else if (extension == std::string(".vtu"))
      {
      vtkNew<vtkXMLUnstructuredGridReader> reader;
      reader->SetFileName(fullName.c_str());
      reader->Update();
      modelNode->SetUnstructuredGridConnection(reader->GetOutputPort());
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
        MeshReaderType::GroupType::Pointer group = readerSH->GetGroup();
        MeshReaderType::GroupType::ObjectListType * objList =  group->GetChildren(1,nullptr);

        MeshReaderType::GroupType::ObjectListType::iterator it = objList->begin();
        itk::SpatialObject<3> * curObj = *it;
        MeshSpatialObjectType::Pointer  SOMesh = dynamic_cast<MeshSpatialObjectType*> (curObj);
        surfaceMesh = SOMesh->GetMesh();
        }
      catch(itk::ExceptionObject &ex)
        {
        std::cout<<ex.GetDescription()<<std::endl;
        result = 0;
        }
      vtkNew<vtkPolyData> vtkMesh;
      // Get the number of points in the mesh
      int numPoints = surfaceMesh->GetNumberOfPoints();

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
      for (; itCells != itCellsEnd; ++itCells)
        {
        floatMesh::CellTraits::PointIdIterator itPt = itCells->Value()->PointIdsBegin();
        vtkIdType ptIdList[64];
        int nPts = 0;
        for (; itPt != itCells->Value()->PointIdsEnd(); ++itPt)
          {
          ptIdList[nPts] = *itPt;
          nPts++;
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
    vtkErrorMacro("ReadData: unknown exception while trying to read file: " << fullName.c_str());
    result = 0;
    }

  if (modelNode->GetMesh() != nullptr)
    {
    // is there an active scalar array?
    if (modelNode->GetDisplayNode()
      && modelNode->GetDisplayNode()->GetScalarRangeFlag() == vtkMRMLDisplayNode::UseDataScalarRange)
      {
      double *scalarRange = modelNode->GetMesh()->GetScalarRange();
      if (scalarRange)
        {
        vtkDebugMacro("ReadDataInternal: setting scalar range " << scalarRange[0] << ", " << scalarRange[1]);
        modelNode->GetDisplayNode()->SetScalarRange(scalarRange);
        }
      }
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

  // We explicitly write the coordinate system into the file header.
  // For now, if space is not defined in a file then we assume that it is in the RAS
  // space (for backward compatibility) but in the future we will switch to LPS by default
  // to be consistent with image coordinate system (that is most often LPS).
  const std::string coordinateSystemTag = "SPACE"; // following NRRD naming convention
  const std::string coordinateSystemValue = "RAS";
  // SPACE=RAS format follows Mimics software's convention, saving extra information into the
  // STL file header: COLOR=rgba,MATERIAL=rgbargbargba
  // (see details at https://en.wikipedia.org/wiki/STL_(file_format)#Color_in_binary_STL)
  const std::string coordinateSytemSpecification = coordinateSystemTag + "=" + coordinateSystemValue;

  int result = 1;
  if (extension == ".vtk" &&
    (modelNode->GetMeshType() == vtkMRMLModelNode::PolyDataMeshType
    || modelNode->GetMeshType() == vtkMRMLModelNode::UnstructuredGridMeshType))
    {
    vtkSmartPointer<vtkDataWriter> writer;
    if (modelNode->GetMeshType() == vtkMRMLModelNode::PolyDataMeshType)
      {
      writer = vtkSmartPointer<vtkPolyDataWriter>::New();
      writer->SetInputConnection(modelNode->GetPolyDataConnection());
      }
    else
      {
      writer = vtkSmartPointer<vtkUnstructuredGridWriter>::New();
      writer->SetInputConnection(modelNode->GetMeshConnection());
      }

    writer->SetFileName(fullName.c_str());
    writer->SetFileType(this->GetUseCompression() ? VTK_BINARY : VTK_ASCII );

    std::string header = std::string("3D Slicer output. ") + coordinateSytemSpecification;
    writer->SetHeader(header.c_str());
    try
      {
      writer->Write();
      }
    catch (...)
      {
      result = 0;
      }
    }
  else if (extension == ".vtu" || extension == ".vtp")
    {
    vtkSmartPointer<vtkXMLUnstructuredDataWriter> writer;
    // We make a shallow copy of the input data and store in inputData
    // so that we can add a custom field for storing coordinate system.
    vtkSmartPointer<vtkDataObject> inputData;
    if (extension == ".vtu")
      {
      writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
      inputData = vtkSmartPointer<vtkUnstructuredGrid>::New();
      inputData->ShallowCopy(modelNode->GetUnstructuredGrid());
      }
    else
      {
      writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
      inputData = vtkSmartPointer<vtkPolyData>::New();
      inputData->ShallowCopy(modelNode->GetPolyData());
      }
    writer->SetInputData(inputData);
    writer->SetFileName(fullName.c_str());
    writer->SetCompressorType(
      this->GetUseCompression() ? vtkXMLWriter::ZLIB : vtkXMLWriter::NONE);
    writer->SetDataMode(
      this->GetUseCompression() ? vtkXMLWriter::Appended : vtkXMLWriter::Ascii);

    // Write coordinate system space (RAS) to field data
    // In the future (when Slicer switches to VTK8) array metadata may be used instead of separate field data.
    vtkFieldData* fieldData = inputData->GetFieldData();
    if (!fieldData)
      {
      vtkNew<vtkFieldData> newFieldData;
      inputData->SetFieldData(newFieldData.GetPointer());
      fieldData = newFieldData.GetPointer();
      }
    if (!fieldData->GetAbstractArray(coordinateSystemTag.c_str()))
      {
      vtkNew<vtkStringArray> coordinateSystemFieldArray;
      coordinateSystemFieldArray->SetName(coordinateSystemTag.c_str());
      coordinateSystemFieldArray->InsertNextValue(coordinateSystemValue.c_str());
      fieldData->AddArray(coordinateSystemFieldArray.GetPointer());
      }
    else
      {
      vtkWarningMacro("vtkMRMLModelStorageNode::WriteDataInternal 'space' field already exists, cannot write coordinate system name into file");
      }

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
    std::string header = std::string("3D Slicer output. ") + coordinateSytemSpecification;
    writer->SetHeader(header.c_str());
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
    std::string header = std::string("3D Slicer output. ") + coordinateSytemSpecification;
    writer->AddComment(coordinateSytemSpecification);
    try
      {
      writer->Write();
      }
    catch (...)
      {
      result = 0;
      }
    }
  else if (extension == ".obj")
    {
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(modelNode->GetPolyDataConnection());
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper.GetPointer());
    vtkMRMLDisplayNode* displayNode = modelNode->GetDisplayNode();
    if (displayNode)
      {
      double color[3] = { 0.5, 0.5, 0.5 };
      displayNode->GetColor(color);
      // OBJ exporter sets the same color for ambient, diffuse, specular
      // so we scale it by 1/3 to avoid having too bright material.
      double colorScale = 1.0 / 3.0;
      actor->GetProperty()->SetColor(color[0] * colorScale, color[1] * colorScale, color[2] * colorScale);
      actor->GetProperty()->SetSpecularPower(3.0);
      actor->GetProperty()->SetOpacity(displayNode->GetOpacity());
      }
    vtkNew<vtkRenderer> renderer;
    renderer->AddActor(actor.GetPointer());
    vtkNew<vtkRenderWindow> renderWindow;
    renderWindow->AddRenderer(renderer.GetPointer());
    vtkNew<vtkOBJExporter> exporter;
    exporter->SetRenderWindow(renderWindow.GetPointer());
    std::string fullNameWithoutExtension = fullName;
    if (fullNameWithoutExtension.size() > 4)
      {
      fullNameWithoutExtension.erase(fullNameWithoutExtension.size() - 4);
      }
    exporter->SetFilePrefix(fullNameWithoutExtension.c_str());
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
    std::string header = std::string("3D Slicer output. ") + coordinateSytemSpecification;
    exporter->SetOBJFileComment(header.c_str());
#endif
    try
      {
      exporter->Write();
      this->ResetFileNameList();
      std::string materialFileName = fullNameWithoutExtension + ".mtl";
      this->AddFileName(materialFileName.c_str());
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
  this->SupportedReadFileTypes->InsertNextValue("XML Poly Data (.vtp)");
  this->SupportedReadFileTypes->InsertNextValue("Unstructured Grid (.vtk)");
  this->SupportedReadFileTypes->InsertNextValue("XML Unstructured Grid (.vtu)");
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
  vtkMRMLModelNode* modelNode = this->GetAssociatedDataNode();
  if (!modelNode || modelNode->GetMeshType() == vtkMRMLModelNode::PolyDataMeshType)
    {
    this->SupportedWriteFileTypes->InsertNextValue("Poly Data (.vtk)");
    this->SupportedWriteFileTypes->InsertNextValue("XML Poly Data (.vtp)");
    // Look at WriteData(), .g and .meta are not being written even though
    // SupportedFileType() says they are supported
    //this->SupportedWriteFileTypes->InsertNextValue("vtkXMLPolyDataReader (.g)");
    //this->SupportedWriteFileTypes->InsertNextValue("vtkXMLPolyDataReader (.meta)");
    this->SupportedWriteFileTypes->InsertNextValue("STL (.stl)");
    this->SupportedWriteFileTypes->InsertNextValue("PLY (.ply)");
    this->SupportedWriteFileTypes->InsertNextValue("Wavefront OBJ (.obj)");
    }
  if (!modelNode || modelNode->GetMeshType() == vtkMRMLModelNode::UnstructuredGridMeshType)
    {
    this->SupportedWriteFileTypes->InsertNextValue("Unstructured Grid (.vtk)");
    this->SupportedWriteFileTypes->InsertNextValue("XML Unstructured Grid (.vtu)");
    }
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLModelStorageNode::GetAssociatedDataNode()
{
  if (!this->GetScene())
    {
    return nullptr;
    }

  std::vector<vtkMRMLNode*> nodes;
  unsigned int numberOfNodes = this->GetScene()->GetNodesByClass("vtkMRMLModelNode", nodes);
  for (unsigned int nodeIndex=0; nodeIndex<numberOfNodes; nodeIndex++)
    {
    vtkMRMLModelNode* node = vtkMRMLModelNode::SafeDownCast(nodes[nodeIndex]);
    if (node)
      {
      const char* storageNodeID = node->GetStorageNodeID();
      if (storageNodeID && !strcmp(storageNodeID, this->ID))
        {
        return vtkMRMLModelNode::SafeDownCast(node);
        }
      }
    }

  return nullptr;
}
