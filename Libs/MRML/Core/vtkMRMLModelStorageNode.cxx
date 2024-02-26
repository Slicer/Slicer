/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLModelStorageNode.cxx,v $
  Date:      $Date: 2006/03/17 15:10:09 $
  Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkMRMLModelStorageNode.h"

#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLI18N.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkActor.h>
#include <vtkAVSucdReader.h>
#include <vtkBYUReader.h>
#include <vtkCellArray.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkFieldData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkOBJReader.h>
#include <vtkOBJExporter.h>
#include <vtkPointData.h>
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
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkTransformPolyDataFilter.h>
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

typedef itk::DefaultDynamicMeshTraits<double, 3, 3, double> MeshTrait;
typedef itk::Mesh<double, 3, MeshTrait> floatMesh;

/** Hold on to the type information specified by the template parameters. */
typedef floatMesh::Pointer MeshPointer;
typedef MeshTrait::PointType MeshPointType;
typedef MeshTrait::PixelType MeshPixelType;

/** Some convenient typedefs. */
typedef floatMesh::Pointer MeshPointer;
typedef floatMesh::CellTraits CellTraits;
typedef floatMesh::PointsContainerPointer PointsContainerPointer;
typedef floatMesh::PointsContainer PointsContainer;
typedef floatMesh::CellsContainerPointer CellsContainerPointer;
typedef floatMesh::CellsContainer CellsContainer;
typedef floatMesh::PointType PointType;
typedef floatMesh::CellType CellType;
typedef itk::TriangleCell<CellType> TriangleType;

typedef itk::MeshSpatialObject<floatMesh> MeshSpatialObjectType;
typedef itk::SpatialObjectReader<3, double, MeshTrait> MeshReaderType;
typedef itk::SpatialObjectWriter<3, double, MeshTrait> MeshWriterType;

// Initialize static member that controls resampling --
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Slicer
// development meeting, to move ijk coordinates to voxel centers."

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLModelStorageNode);

//----------------------------------------------------------------------------
vtkMRMLModelStorageNode::vtkMRMLModelStorageNode()
{
  this->DefaultWriteFileExtension = "vtk";
  this->CoordinateSystem = vtkMRMLStorageNode::CoordinateSystemLPS;
}

//----------------------------------------------------------------------------
vtkMRMLModelStorageNode::~vtkMRMLModelStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  // If no CoordinateSystem is specified then it means this is a legacy scene,
  // which used RAS coordinate system in file.
  this->CoordinateSystem = vtkMRMLStorageNode::CoordinateSystemRAS;

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(coordinateSystem, CoordinateSystem);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  of << " coordinateSystem=\"" << this->GetCoordinateSystemTypeAsString(this->CoordinateSystem) << "\"";
}

//---------------------------------------------------------------------------
const char* vtkMRMLModelStorageNode::GetCoordinateSystemAsString(int id)
{
  return vtkMRMLStorageNode::GetCoordinateSystemTypeAsString(id);
}

//-----------------------------------------------------------
int vtkMRMLModelStorageNode::GetCoordinateSystemFromString(const char* name)
{
  return vtkMRMLStorageNode::GetCoordinateSystemTypeFromString(name);
}

//----------------------------------------------------------------------------
bool vtkMRMLModelStorageNode::CanReadInReferenceNode(vtkMRMLNode* refNode)
{
  return refNode->IsA("vtkMRMLModelNode");
}

//----------------------------------------------------------------------------
int vtkMRMLModelStorageNode::ReadDataInternal(vtkMRMLNode* refNode)
{
  if (this->GetWriteState() == SkippedNoData)
  {
    vtkDebugMacro("ReadDataInternal (" << (this->ID ? this->ID : "(unknown)")
                                       << "): empty model file was not saved, ignore loading");
    return 1;
  }

  vtkMRMLModelNode* modelNode = dynamic_cast<vtkMRMLModelNode*>(refNode);
  if (!modelNode)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLModelStorageNode::ReadDataInternal",
                                     "Node for storing reading result (" << (this->ID ? this->ID : "(unknown)")
                                                                         << ") is not a valid model node.");
    return 0;
  }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLModelStorageNode::ReadDataInternal",
                                     "Filename is not specified (" << (this->ID ? this->ID : "(unknown)") << ").");
    return 0;
  }

  // check that the file exists
  if (vtksys::SystemTools::FileExists(fullName.c_str()) == false)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLModelStorageNode::ReadDataInternal",
                                     "Model file '" << fullName.c_str() << "' is not found while trying to read node ("
                                                    << (this->ID ? this->ID : "(unknown)") << ").");
    return 0;
  }

  // compute file prefix
  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if (extension.empty())
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLModelStorageNode::ReadDataInternal",
                                     "Model file '" << fullName.c_str()
                                                    << "' has no file extension while trying to read node ("
                                                    << (this->ID ? this->ID : "(unknown)") << ").");
    return 0;
  }

  vtkDebugMacro("ReadDataInternal (" << (this->ID ? this->ID : "(unknown)") << "): extension = " << extension.c_str());

  int coordinateSystemInFileHeader = -1;
  vtkSmartPointer<vtkPointSet> meshFromFile;
  try
  {
    if (extension == std::string(".g") || extension == std::string(".byu"))
    {
      vtkNew<vtkBYUReader> reader;
      this->GetUserMessages()->SetObservedObject(reader);
      reader->SetGeometryFileName(fullName.c_str());
      reader->Update();
      this->GetUserMessages()->SetObservedObject(nullptr);
      meshFromFile = reader->GetOutput();
    }
    else if (extension == std::string(".vtk"))
    {
      vtkNew<vtkPolyDataReader> reader;
      reader->SetFileName(fullName.c_str());
      vtkNew<vtkUnstructuredGridReader> unstructuredGridReader;
      unstructuredGridReader->SetFileName(fullName.c_str());

      if (reader->IsFilePolyData())
      {
        reader->ReadAllScalarsOn();
        reader->ReadAllVectorsOn();
        reader->ReadAllNormalsOn();
        reader->ReadAllTensorsOn();
        reader->ReadAllColorScalarsOn();
        reader->ReadAllTCoordsOn();
        reader->ReadAllFieldsOn();
        this->GetUserMessages()->SetObservedObject(reader);
        reader->Update();
        meshFromFile = reader->GetOutput();
        this->GetUserMessages()->SetObservedObject(nullptr);
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
        this->GetUserMessages()->SetObservedObject(unstructuredGridReader);
        unstructuredGridReader->Update();
        meshFromFile = unstructuredGridReader->GetOutput();
        this->GetUserMessages()->SetObservedObject(nullptr);
      }
      else
      {
        vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                         "vtkMRMLModelStorageNode::ReadDataInternal",
                                         "Failed to load model from VTK file "
                                           << fullName << " as it does not contain polydata nor unstructured grid."
                                           << " The file might be loadable as a volume.");
      }
      coordinateSystemInFileHeader = vtkMRMLModelStorageNode::GetCoordinateSystemFromFileHeader(reader->GetHeader());
    }
    else if (extension == std::string(".vtp"))
    {
      vtkNew<vtkXMLPolyDataReader> reader;
      this->GetUserMessages()->SetObservedObject(reader);
      reader->SetFileName(fullName.c_str());
      reader->Update();
      meshFromFile = reader->GetOutput();
      this->GetUserMessages()->SetObservedObject(nullptr);
      coordinateSystemInFileHeader = vtkMRMLModelStorageNode::GetCoordinateSystemFromFieldData(meshFromFile);
    }
    else if (extension == std::string(".ucd"))
    {
      vtkNew<vtkAVSucdReader> reader;
      this->GetUserMessages()->SetObservedObject(reader);
      reader->SetFileName(fullName.c_str());
      reader->Update();
      meshFromFile = reader->GetOutput();
      this->GetUserMessages()->SetObservedObject(nullptr);
    }
    else if (extension == std::string(".vtu"))
    {
      vtkNew<vtkXMLUnstructuredGridReader> reader;
      this->GetUserMessages()->SetObservedObject(reader);
      reader->SetFileName(fullName.c_str());
      reader->Update();
      meshFromFile = reader->GetOutput();
      this->GetUserMessages()->SetObservedObject(nullptr);
      coordinateSystemInFileHeader = vtkMRMLModelStorageNode::GetCoordinateSystemFromFieldData(meshFromFile);
    }
    else if (extension == std::string(".stl"))
    {
      vtkNew<vtkSTLReader> reader;
      this->GetUserMessages()->SetObservedObject(reader);
      reader->SetFileName(fullName.c_str());
      reader->Update();
      meshFromFile = reader->GetOutput();
      this->GetUserMessages()->SetObservedObject(nullptr);
      coordinateSystemInFileHeader = vtkMRMLModelStorageNode::GetCoordinateSystemFromFileHeader(reader->GetHeader());
    }
    else if (extension == std::string(".ply"))
    {
      vtkNew<vtkPLYReader> reader;
      this->GetUserMessages()->SetObservedObject(reader);
      reader->SetFileName(fullName.c_str());
      reader->Update();
      meshFromFile = reader->GetOutput();
      this->GetUserMessages()->SetObservedObject(nullptr);
      vtkStringArray* comments = reader->GetComments();
      for (int commentIndex = 0; commentIndex < comments->GetNumberOfValues(); commentIndex++)
      {
        coordinateSystemInFileHeader =
          vtkMRMLModelStorageNode::GetCoordinateSystemFromFileHeader(comments->GetValue(commentIndex).c_str());
        if (coordinateSystemInFileHeader >= 0)
        {
          // found a comment that contains coordinate system information
          break;
        }
      }
    }
    else if (extension == std::string(".obj"))
    {
      vtkNew<vtkOBJReader> reader;
      this->GetUserMessages()->SetObservedObject(reader);
      reader->SetFileName(fullName.c_str());
      reader->Update();
      meshFromFile = reader->GetOutput();
      this->GetUserMessages()->SetObservedObject(nullptr);
      coordinateSystemInFileHeader = vtkMRMLModelStorageNode::GetCoordinateSystemFromFileHeader(reader->GetComment());
    }
    else if (extension == std::string(".meta")) // model in meta format
    {
      floatMesh::Pointer surfaceMesh = floatMesh::New();
      MeshReaderType::Pointer readerSH = MeshReaderType::New();
      try
      {
        readerSH->SetFileName(fullName.c_str());
        readerSH->Update();
        MeshReaderType::GroupType::Pointer group = readerSH->GetGroup();
        MeshReaderType::GroupType::ObjectListType* objList = group->GetChildren(1, nullptr);

        MeshReaderType::GroupType::ObjectListType::iterator it = objList->begin();
        itk::SpatialObject<3>* curObj = *it;
        MeshSpatialObjectType::Pointer SOMesh = dynamic_cast<MeshSpatialObjectType*>(curObj);
        surfaceMesh = SOMesh->GetMesh();
      }
      catch (itk::ExceptionObject& ex)
      {
        vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                         "vtkMRMLModelStorageNode::ReadDataInternal",
                                         "Failed to load model from ITK .meta file " << fullName << ": "
                                                                                     << ex.GetDescription());
        return 0;
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
      for (floatMesh::PointsContainer::Iterator i = points->Begin(); i != points->End(); ++i)
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
      meshFromFile = vtkMesh;
    }
    else
    {
      vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                       "vtkMRMLModelStorageNode::ReadDataInternal",
                                       "Failed to load model: unrecognized file extension '"
                                         << extension << "' of file '" << fullName << "'.");
      return 0;
    }
  }
  catch (...)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLModelStorageNode::ReadDataInternal",
                                     "Failed to load model: unknown exception while trying to load the file '"
                                       << fullName << "'.");
    return 0;
  }

  if (this->GetUserMessages()->GetNumberOfMessagesOfType(vtkCommand::ErrorEvent) > 0)
  {
    // User messages are already logged, no need for logging more
    return 0;
  }

  if (coordinateSystemInFileHeader >= 0)
  {
    // coordinate system specified in the file, use it (regardless oassumingf what was the preferred coordinate system
    // in the node)
    this->CoordinateSystem = coordinateSystemInFileHeader;
  }
  else
  {
    // no coordinate system in the file, use the currently set coordinate system
    vtkInfoMacro("ReadDataInternal (" << (this->ID ? this->ID : "(unknown)") << "): File " << fullName.c_str()
                                      << " does not contain coordinate system information. Assuming "
                                      << vtkMRMLStorageNode::GetCoordinateSystemTypeAsString(this->CoordinateSystem)
                                      << ".");
  }

  vtkSmartPointer<vtkPointSet> meshToSetInNode;
  if (this->CoordinateSystem == vtkMRMLStorageNode::CoordinateSystemRAS)
  {
    // no flip of first two axes
    meshToSetInNode = meshFromFile;
  }
  else
  {
    // transform from RAS to LPS
    if (meshFromFile->IsA("vtkPolyData"))
    {
      meshToSetInNode = vtkSmartPointer<vtkPolyData>::New();
    }
    else
    {
      meshToSetInNode = vtkSmartPointer<vtkUnstructuredGrid>::New();
    }
    vtkMRMLModelStorageNode::ConvertBetweenRASAndLPS(meshFromFile, meshToSetInNode);
  }
  modelNode->SetAndObserveMesh(meshToSetInNode);

  if (modelNode->GetMesh() != nullptr)
  {
    for (int i = 0; i < modelNode->GetNumberOfDisplayNodes(); ++i)
    {
      vtkMRMLDisplayNode* displayNode = modelNode->GetNthDisplayNode(i);
      // is there an active scalar array?
      if (displayNode && displayNode->GetScalarRangeFlag() == vtkMRMLDisplayNode::UseDataScalarRange)
      {
        double* scalarRange = modelNode->GetMesh()->GetScalarRange();
        if (scalarRange)
        {
          vtkDebugMacro("ReadDataInternal (" << (this->ID ? this->ID : "(unknown)") << "): setting scalar range "
                                             << scalarRange[0] << ", " << scalarRange[1]);
          displayNode->SetScalarRange(scalarRange);
        }
      }
    } // For all display nodes
  }
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLModelStorageNode::WriteDataInternal(vtkMRMLNode* refNode)
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(refNode);

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLModelStorageNode::WriteDataInternal",
                                     "Failed to write model: File name not specified.");
    return 0; // failed
  }

  if (modelNode->GetMeshConnection() == nullptr || modelNode->GetMesh()->GetNumberOfPoints() == 0)
  {
    this->SetWriteStateSkippedNoData();
    return 1; // success
  }

  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);

  // We explicitly write the coordinate system into the file header.
  const std::string coordinateSystemTag = "SPACE"; // following NRRD naming convention
  const std::string coordinateSystemStr = vtkMRMLStorageNode::GetCoordinateSystemTypeAsString(this->CoordinateSystem);
  // SPACE=RAS format follows Mimics software's convention, saving extra information into the
  // STL file header: COLOR=rgba,MATERIAL=rgbargbargba
  // (see details at https://en.wikipedia.org/wiki/STL_(file_format)#Color_in_binary_STL)
  const std::string coordinateSytemSpecification = coordinateSystemTag + "=" + coordinateSystemStr;

  vtkSmartPointer<vtkPointSet> meshToWrite;
  if (this->CoordinateSystem == vtkMRMLStorageNode::CoordinateSystemRAS)
  {
    // no flip of first two axes
    meshToWrite = modelNode->GetMesh();
  }
  else
  {
    // transform from RAS to LPS
    if (modelNode->GetMeshType() == vtkMRMLModelNode::PolyDataMeshType)
    {
      meshToWrite = vtkSmartPointer<vtkPolyData>::New();
    }
    else
    {
      meshToWrite = vtkSmartPointer<vtkUnstructuredGrid>::New();
    }
    vtkMRMLModelStorageNode::ConvertBetweenRASAndLPS(modelNode->GetMesh(), meshToWrite);
  }

  bool success = true;

  if (extension == ".vtk"
      && (modelNode->GetMeshType() == vtkMRMLModelNode::PolyDataMeshType
          || modelNode->GetMeshType() == vtkMRMLModelNode::UnstructuredGridMeshType))
  {
    vtkSmartPointer<vtkDataWriter> writer;
    this->GetUserMessages()->SetObservedObject(writer);
    if (modelNode->GetMeshType() == vtkMRMLModelNode::PolyDataMeshType)
    {
      writer = vtkSmartPointer<vtkPolyDataWriter>::New();
      // version 5.1 is not compatible with earlier Slicer versions (VTK < 9) and most other software
      writer->SetFileVersion(42);
      writer->SetInputData(meshToWrite);
    }
    else
    {
      writer = vtkSmartPointer<vtkUnstructuredGridWriter>::New();
      // version 5.1 is not compatible with earlier Slicer versions (VTK < 9) and most other software
      writer->SetFileVersion(42);
      writer->SetInputData(meshToWrite);
    }

    writer->SetFileName(fullName.c_str());
    writer->SetFileType(this->GetUseCompression() ? VTK_BINARY : VTK_ASCII);

    std::string header = std::string("3D Slicer output. ") + coordinateSytemSpecification;
    writer->SetHeader(header.c_str());
    try
    {
      success = success && writer->Write();
    }
    catch (...)
    {
      success = false;
    }
    this->GetUserMessages()->SetObservedObject(nullptr);
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
      inputData->ShallowCopy(meshToWrite);
    }
    else
    {
      writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
      inputData = vtkSmartPointer<vtkPolyData>::New();
      inputData->ShallowCopy(meshToWrite);
    }
    this->GetUserMessages()->SetObservedObject(writer);
    writer->SetInputData(inputData);
    writer->SetFileName(fullName.c_str());
    writer->SetCompressorType(this->GetUseCompression() ? vtkXMLWriter::ZLIB : vtkXMLWriter::NONE);
    writer->SetDataMode(this->GetUseCompression() ? vtkXMLWriter::Appended : vtkXMLWriter::Ascii);

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
      coordinateSystemFieldArray->InsertNextValue(coordinateSystemStr.c_str());
      fieldData->AddArray(coordinateSystemFieldArray.GetPointer());
    }
    else
    {
      vtkWarningToMessageCollectionMacro(
        this->GetUserMessages(),
        "vtkMRMLModelStorageNode::WriteDataInternal",
        "Failed to write model " << (this->ID ? this->ID : "(unknown)")
                                 << ": 'space' field already exists, cannot write coordinate system name into file");
    }

    try
    {
      success = success && writer->Write();
    }
    catch (...)
    {
      success = false;
    }
    this->GetUserMessages()->SetObservedObject(nullptr);
  }
  else if (extension == ".stl")
  {
    vtkNew<vtkTriangleFilter> triangulator;
    vtkNew<vtkSTLWriter> writer;
    this->GetUserMessages()->SetObservedObject(writer);
    writer->SetFileName(fullName.c_str());
    writer->SetFileType(this->GetUseCompression() ? VTK_BINARY : VTK_ASCII);
    triangulator->SetInputData(meshToWrite);
    writer->SetInputConnection(triangulator->GetOutputPort());
    std::string header = std::string("3D Slicer output. ") + coordinateSytemSpecification;
    writer->SetHeader(header.c_str());
    try
    {
      success = success && writer->Write();
    }
    catch (...)
    {
      success = false;
    }
    this->GetUserMessages()->SetObservedObject(nullptr);
  }
  else if (extension == ".ply")
  {
    vtkNew<vtkTriangleFilter> triangulator;
    vtkNew<vtkPLYWriter> writer;
    this->GetUserMessages()->SetObservedObject(writer);
    writer->SetFileName(fullName.c_str());
    writer->SetFileType(this->GetUseCompression() ? VTK_BINARY : VTK_ASCII);
    // VTK's PLY writer can save RGB or RGBA unsigned char color array.
    // If we find such an array then we configure the writer to include that array.
    if (meshToWrite->GetPointData())
    {
      vtkPointData* pointData = meshToWrite->GetPointData();
      for (int arrayIndex = 0; arrayIndex < pointData->GetNumberOfArrays(); arrayIndex++)
      {
        vtkUnsignedCharArray* dataArray = vtkUnsignedCharArray::SafeDownCast(pointData->GetArray(arrayIndex));
        if (!dataArray || !dataArray->GetName())
        {
          continue;
        }
        if (dataArray->GetNumberOfComponents() == 3)
        {
          // Found a suitable RGB array
          writer->SetArrayName(dataArray->GetName());
          writer->EnableAlphaOff();
          break;
        }
        else if (dataArray->GetNumberOfComponents() == 4)
        {
          // Found a suitable RGBA array
          writer->SetArrayName(dataArray->GetName());
          writer->EnableAlphaOn();
          break;
        }
      }
    }
    triangulator->SetInputData(meshToWrite);
    writer->SetInputConnection(triangulator->GetOutputPort());
    std::string header = std::string("3D Slicer output. ") + coordinateSytemSpecification;
    writer->AddComment(coordinateSytemSpecification);
    try
    {
      success = success && writer->Write();
    }
    catch (...)
    {
      success = false;
    }
    this->GetUserMessages()->SetObservedObject(nullptr);
  }
  else if (extension == ".obj")
  {
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(vtkPolyData::SafeDownCast(meshToWrite));
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
    this->GetUserMessages()->SetObservedObject(exporter);
    exporter->SetRenderWindow(renderWindow.GetPointer());
    std::string fullNameWithoutExtension = fullName;
    if (fullNameWithoutExtension.size() > 4)
    {
      fullNameWithoutExtension.erase(fullNameWithoutExtension.size() - 4);
    }
    exporter->SetFilePrefix(fullNameWithoutExtension.c_str());
    std::string header = std::string("3D Slicer output. ") + coordinateSytemSpecification;
    exporter->SetOBJFileComment(header.c_str());
    try
    {
      exporter->Write();
      // exporter's Write() does not provide a return value but UserMessages will capture any errors
      this->ResetFileNameList();
      std::string materialFileName = fullNameWithoutExtension + ".mtl";
      this->AddFileName(materialFileName.c_str());
    }
    catch (...)
    {
      success = false;
    }
    this->GetUserMessages()->SetObservedObject(nullptr);
  }
  else
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLModelStorageNode::WriteDataInternal",
                                     "Failed to write model "
                                       << (this->ID ? this->ID : "(unknown)")
                                       << ": No file extension recognized : " << fullName.c_str());
  }

  // If no errors have been reported then check if the output file exists
  // because some writers might miss reporting file creation errors.
  if (success && this->GetUserMessages()->GetNumberOfMessagesOfType(vtkCommand::ErrorEvent) == 0
      && !vtksys::SystemTools::FileExists(fullName))
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLModelStorageNode::WriteDataInternal",
                                     "Failed to write model " << (this->ID ? this->ID : "(unknown)") << ": Output file "
                                                              << fullName.c_str() << " has not been created.");
    success = false;
  }

  return (success ? 1 : 0);
}

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::InitializeSupportedReadFileTypes()
{
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "VTK Polygon Mesh") + " (.vtk)");
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "VTK XML Polygon Mesh")
                                                + " (.vtp)");
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "VTK Unstructured Grid")
                                                + " (.vtk)");
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "VTK XML Unstructured Grid")
                                                + " (.vtu)");
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "Movie.BYU Mesh") + " (.g)");
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "Movie.BYU Mesh") + " (.byu)");
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "MetaIO Mesh") + " (.meta)");
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "Stereolithography Mesh")
                                                + " (.stl)");
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "Stanford Polygon Mesh")
                                                + " (.ply)");
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "AVS Unstructured Grid")
                                                + " (.ucd)");
  //: File format name
  this->SupportedReadFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "Wavefront Mesh") + " (.obj)");
}

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::InitializeSupportedWriteFileTypes()
{
  vtkMRMLModelNode* modelNode = this->GetAssociatedDataNode();
  if (!modelNode || modelNode->GetMeshType() == vtkMRMLModelNode::PolyDataMeshType)
  {
    //: File format name
    this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "VTK Polygon Mesh")
                                                   + " (.vtk)");
    //: File format name
    this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "VTK XML Polygon Mesh")
                                                   + " (.vtp)");
    // Look at WriteData(), .g and .meta are not being written even though
    // SupportedFileType() says they are supported
    // this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "Movie.BYU Mesh") + " (.g)");
    // this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "Movie.BYU Mesh") + "
    // (.meta)");
    //: File format name
    this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "Stereolithography Mesh")
                                                   + " (.stl)");
    //: File format name
    this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "Stanford Polygon Mesh")
                                                   + " (.ply)");
    //: File format name
    this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "Wavefront Mesh") + " (.obj)");
  }
  if (!modelNode || modelNode->GetMeshType() == vtkMRMLModelNode::UnstructuredGridMeshType)
  {
    //: File format name
    this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "VTK Unstructured Grid")
                                                   + " (.vtk)");
    //: File format name
    this->SupportedWriteFileTypes->InsertNextValue(vtkMRMLTr("vtkMRMLModelStorageNode", "VTK XML Unstructured Grid")
                                                   + " (.vtu)");
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
  for (unsigned int nodeIndex = 0; nodeIndex < numberOfNodes; nodeIndex++)
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

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::ConvertBetweenRASAndLPS(vtkPointSet* inputMesh, vtkPointSet* outputMesh)
{
  if (!inputMesh || !outputMesh)
  {
    vtkGenericWarningMacro("vtkMRMLModelStorageNode::ConvertBetweenRASAndLPS: invalid input or output mesh");
    return;
  }
  vtkNew<vtkTransform> transformRasLps;
  transformRasLps->Scale(-1, -1, 1);
  // vtkTransformPolyDataFilter preserves texture coordinates, while vtkTransformFilter removes them,
  // therefore we must use vtkTransformPolyDataFilter for surface meshes.
  if (inputMesh->IsA("vtkPolyData"))
  {
    vtkNew<vtkTransformPolyDataFilter> transformFilter;
    transformFilter->SetTransform(transformRasLps);
    transformFilter->SetInputData(inputMesh);
    transformFilter->Update();
    outputMesh->ShallowCopy(transformFilter->GetOutput());
  }
  else
  {
    vtkNew<vtkTransformFilter> transformFilter;
    transformFilter->SetTransform(transformRasLps);
    transformFilter->SetInputData(inputMesh);
    transformFilter->TransformAllInputVectorsOn();
    transformFilter->Update();
    outputMesh->ShallowCopy(transformFilter->GetOutput());
  }
}

//----------------------------------------------------------------------------
int vtkMRMLModelStorageNode::GetCoordinateSystemFromFileHeader(const char* header)
{
  if (!header)
  {
    return -1;
  }
  std::string headerStr = header;
  if (headerStr.find("SPACE=RAS") != std::string::npos)
  {
    return vtkMRMLStorageNode::CoordinateSystemRAS;
  }
  else if (headerStr.find("SPACE=LPS") != std::string::npos)
  {
    return vtkMRMLStorageNode::CoordinateSystemLPS;
  }
  return -1;
}

//----------------------------------------------------------------------------
int vtkMRMLModelStorageNode::GetCoordinateSystemFromFieldData(vtkPointSet* mesh)
{
  if (!mesh)
  {
    return -1;
  }
  vtkFieldData* fieldData = mesh->GetFieldData();
  if (!fieldData)
  {
    return -1;
  }
  const std::string coordinateSystemTag = "SPACE"; // following NRRD naming convention
  vtkStringArray* coordinateSystemArray =
    vtkStringArray::SafeDownCast(fieldData->GetAbstractArray(coordinateSystemTag.c_str()));
  if (!coordinateSystemArray || coordinateSystemArray->GetNumberOfValues() < 1)
  {
    return -1;
  }
  std::string headerStr = coordinateSystemArray->GetValue(0);
  // We retrieved the value, now delete it, as it would interfere with writing of the file
  // if a coordinate system array is already specified in the mesh.
  fieldData->RemoveArray(coordinateSystemTag.c_str());
  if (headerStr.find("RAS") != std::string::npos)
  {
    return vtkMRMLStorageNode::CoordinateSystemRAS;
  }
  else if (headerStr.find("LPS") != std::string::npos)
  {
    return vtkMRMLStorageNode::CoordinateSystemLPS;
  }
  return -1;
}
