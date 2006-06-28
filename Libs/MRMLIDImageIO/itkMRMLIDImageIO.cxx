/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: itkMRMLIDImageIO.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.18 $

=========================================================================auto=*/


#include "itkMRMLIDImageIO.h"

#include "vtkMRMLNode.h"
#include "vtkMRMLVolumeNode.h"

#include "vtkPointData.h"
#include "vtkDataArray.h"

#include <string>

namespace itk {

MRMLIDImageIO
::MRMLIDImageIO()
{
  this->Scheme = "";
  this->Authority = "";
  this->SceneID = "";
  this->NodeID = "";
}

MRMLIDImageIO
::~MRMLIDImageIO()
{
}

bool
MRMLIDImageIO
::IsAVolumeNode(const char* filename)
{
  vtkMRMLVolumeNode *node;

  node = this->FileNameToVolumeNodePtr(filename);

  if (node)
    {
    return true;
    }

  return false;
}

vtkMRMLVolumeNode *
MRMLIDImageIO
::FileNameToVolumeNodePtr(const char* filename)
{
  // if this is a MRML node, then filename will be encoded 
  // with a "slicer" scheme.  There are two possibilities:
  //
  // slicer:<scene id>/<node id>                  - local slicer
  // slicer://<hostname>/<scene id>/<node id>     - remote slicer
  //
  // What about multiple instances of slicer on a machine? (ports?)
  //
  std::string fname = filename;
  std::string::size_type loc, hloc;

  this->Authority = "";
  this->SceneID = "";
  this->NodeID = "";
  
  // check that the filename starts with the slicer3 scheme
  loc = fname.find("slicer:");
  if (loc != fname.size() && (loc == 0))
    {
    this->Scheme = std::string(fname.begin(),
                               fname.begin() + std::string("slicer").size());
    loc = this->Scheme.size() + 1; // skip the colon
    
    // now check whether we have a local or remote resource
    if (std::string(fname.begin()+loc,
                    fname.begin()+loc+2) == "//")
      {
      // remote access, pull out a hostname
      hloc = fname.find("/", loc+2);
      if (hloc == fname.size())
        {
        // no hostname specified
        return 0;
        }
      this->Authority = std::string(fname.begin()+loc+2, fname.begin()+hloc);
      loc = hloc+1; // skip the slash
      }

    // now pull off the scene
    hloc = fname.find("/", loc);
    if (hloc == fname.size())
      {
      // no scene specified
      return 0;
      }
    if (hloc >= loc)
      {
      this->SceneID = std::string(fname.begin()+loc, fname.begin()+hloc);
      }
    else
      {
      this->SceneID = "";
      }
    loc = hloc+1;   // skip the slash
    
    // now pull off the node
    this->NodeID = std::string(fname.begin()+loc, fname.end());
    
    // so far so good.  now see if we can cast down to a MRMLVolumeNode
    //
    // Scenes and nodes should be specified by tags not by pointer
    std::string ptrAsString = this->NodeID;
    vtkObjectBase *ptr = 0;
    
    sscanf(ptrAsString.c_str(), "%p", &ptr);

    vtkMRMLVolumeNode* vptr = vtkMRMLVolumeNode::SafeDownCast(ptr);
    if (vptr != 0)
      {
      // we are indeed referencing a volume node
      return vptr;
      }
    }
  
  return 0;
}


bool
MRMLIDImageIO
::CanReadFile(const char* filename)
{
  return this->IsAVolumeNode(filename);
}

void
MRMLIDImageIO
::ReadImageInformation()
{
  vtkMRMLVolumeNode *node;

  node = this->FileNameToVolumeNodePtr( m_FileName.c_str() );
  if (node)
    {
    // VTK is only 3D
    this->SetNumberOfDimensions(3);

    // Spacing
    m_Spacing.resize(3);
    m_Spacing[0] = node->GetSpacing()[0];
    m_Spacing[1] = node->GetSpacing()[1];
    m_Spacing[2] = node->GetSpacing()[2];

    // Origin
    m_Origin.resize(3);
    m_Origin[0] = node->GetOrigin()[0];
    m_Origin[1] = node->GetOrigin()[1];
    m_Origin[2] = node->GetOrigin()[2];

    // Directions 
    m_Direction.resize(3);

    m_Direction[0].resize(3);
    node->GetIToRASDirection( &(m_Direction[0][0]) );
    m_Direction[1].resize(3);
    node->GetJToRASDirection( &(m_Direction[1][0]) );
    m_Direction[2].resize(3);
    node->GetKToRASDirection( &(m_Direction[2][0]) );

    // Dimensions
    this->SetNumberOfDimensions(3);
    this->SetDimensions(0, node->GetImageData()->GetDimensions()[0]);
    this->SetDimensions(1, node->GetImageData()->GetDimensions()[1]);
    this->SetDimensions(2, node->GetImageData()->GetDimensions()[2]);
    
    // Number of components, PixelType
    this->SetNumberOfComponents(node->GetImageData()
                                ->GetNumberOfScalarComponents());

    // PixelType
    if (this->GetNumberOfComponents() == 1)
      {
      this->SetPixelType(SCALAR);
      }
    else
      {
      // what should the mapping be for multi-component scalars?
      this->SetPixelType(VECTOR);
      }
      
    // ComponentType
    switch (node->GetImageData()->GetScalarType())
      {
      case VTK_FLOAT: this->SetComponentType(FLOAT); break;
      case VTK_DOUBLE: this->SetComponentType(DOUBLE); break;
      case VTK_INT: this->SetComponentType(INT); break;
      case VTK_UNSIGNED_INT: this->SetComponentType(UINT); break;
      case VTK_SHORT: this->SetComponentType(SHORT); break;
      case VTK_UNSIGNED_SHORT: this->SetComponentType(USHORT); break;
      case VTK_LONG: this->SetComponentType(LONG); break;
      case VTK_UNSIGNED_LONG: this->SetComponentType(ULONG); break;
      case VTK_CHAR: this->SetComponentType(CHAR); break;
      case VTK_UNSIGNED_CHAR: this->SetComponentType(UCHAR); break;
      default: itkWarningMacro("Unknown scalar type.");
        this->SetComponentType(UNKNOWNCOMPONENTTYPE);
        break;
      }
    }    
}

void
MRMLIDImageIO
::Read(void *buffer)
{
  vtkMRMLVolumeNode *node;

  node = this->FileNameToVolumeNodePtr( m_FileName.c_str() );
  if (node)
    {
    // buffer is preallocated, memcpy the data
    // (just looking at the scalars)
    memcpy(buffer, node->GetImageData()->GetScalarPointer(), 
           this->GetImageSizeInBytes());
    }
}

bool
MRMLIDImageIO
::CanWriteFile(const char* filename)
{
  return this->IsAVolumeNode(filename);
}

void
MRMLIDImageIO
::WriteImageInformation()
{
  vtkMRMLVolumeNode *node;

  node = this->FileNameToVolumeNodePtr( m_FileName.c_str() );
  if (node)
    {
    // Directions
    node->SetIToRASDirection(m_Direction[0][0],
                             m_Direction[0][1],
                             m_Direction[0][2]);
    node->SetJToRASDirection(m_Direction[1][0],
                             m_Direction[1][1],
                             m_Direction[1][2]);
    node->SetKToRASDirection(m_Direction[2][0],
                             m_Direction[2][1],
                             m_Direction[2][2]);

    // Fill in dimensions, spacing, origin
    // VTK is only 3D, only copy the first 3 dimensions, fill in with
    // reasonable defaults for the rest
    if (this->GetNumberOfDimensions() > 3)
      {
      itkWarningMacro("Dimension of image is too high for VTK (Dimension = "
                    << this->GetNumberOfDimensions() << ")" );
      }
    
    unsigned int i;
    int dim[3];
    double spacing[3];
    double origin[3];
    for (i=0; (i < this->GetNumberOfDimensions()) && (i < 3); ++i)
      {
      dim[i] = this->GetDimensions(i);
      spacing[i] = m_Spacing[i];
      origin[i] = m_Origin[i];
      }
    if (this->GetNumberOfDimensions() < 3)
      {
      // VTK is only 3D, fill in remaining dimensions
      for (; i < 3; ++i)
        {
        dim[i] = 1;
        spacing[i] = 1.0;
        origin[i] = 0.0;
        }
      }
    node->GetImageData()->SetDimensions(dim);
    node->SetSpacing( spacing );

//     // Center the data
//     origin[0] =  -spacing[0]*dim[0]/2.0;
//     origin[1] =  -spacing[1]*dim[1]/2.0;
//     origin[2] =  -spacing[2]*dim[2]/2.0;

    node->SetOrigin( origin );
    
    // Number of components, PixelType
    node->GetImageData()
      ->SetNumberOfScalarComponents(this->GetNumberOfComponents());

    // ComponentType
    switch (this->GetComponentType())
      {
      case FLOAT: node->GetImageData()->SetScalarTypeToFloat(); break;
      case DOUBLE: node->GetImageData()->SetScalarTypeToDouble(); break;
      case INT: node->GetImageData()->SetScalarTypeToInt(); break;
      case UINT: node->GetImageData()->SetScalarTypeToUnsignedInt(); break;
      case SHORT: node->GetImageData()->SetScalarTypeToShort(); break;
      case USHORT: node->GetImageData()->SetScalarTypeToUnsignedShort(); break;
      case LONG: node->GetImageData()->SetScalarTypeToLong(); break;
      case ULONG: node->GetImageData()->SetScalarTypeToUnsignedLong(); break;
      case CHAR: node->GetImageData()->SetScalarTypeToChar(); break;
      case UCHAR: node->GetImageData()->SetScalarTypeToUnsignedChar(); break;
      default:
        // What should we do?
        itkWarningMacro("Unknown scalar type.");
        node->GetImageData()->SetScalarTypeToShort();
        break;
      }
    }    
}

void
MRMLIDImageIO
::Write(const void *buffer)
{
  vtkMRMLVolumeNode *node;

  node = this->FileNameToVolumeNodePtr( m_FileName.c_str() );
  if (node)
    {
    // Need to create a VTK ImageData to hang off the node
    vtkImageData *img = vtkImageData::New();
    node->SetImageData(img);
    img->Delete();

    // Configure the information on the node/image data
    this->WriteImageInformation();

    // allocate the data, copy the data (just looking at the scalars)
    node->GetImageData()->AllocateScalars();

    memcpy(node->GetImageData()->GetScalarPointer(), buffer,
          node->GetImageData()->GetPointData()->GetScalars()->GetNumberOfComponents() *
          node->GetImageData()->GetPointData()->GetScalars()->GetNumberOfTuples() *
          node->GetImageData()->GetPointData()->GetScalars()->GetDataTypeSize()
      );
    }
}


void
MRMLIDImageIO
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Scheme: " << this->Scheme << std::endl;
  os << indent << "Authority: " << this->Authority << std::endl;
  os << indent << "SceneID: " << this->SceneID << std::endl;
  os << indent << "NodeID: " << this->NodeID << std::endl;
  
}


} // end namespace itk
