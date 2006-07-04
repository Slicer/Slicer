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
  if (loc != std::string::npos && (loc == 0))
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
      if (hloc == std::string::npos)
        {
        // no hostname specified
        return 0;
        }
      this->Authority = std::string(fname.begin()+loc+2, fname.begin()+hloc);
      loc = hloc+1; // skip the slash
      }

    // now pull off the scene
    hloc = fname.find("/", loc);
    if (hloc == std::string::npos)
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

    // Get spacing, origin and directions from node. The node keeps
    // these in RAS, ITK needs them in LPS.

    vtkMatrix4x4 *ijkToRas = vtkMatrix4x4::New();
    node->GetIJKToRASMatrix(ijkToRas);

    vtkMatrix4x4 *rasToLps = vtkMatrix4x4::New();
    rasToLps->Identity();
    rasToLps->SetElement(0,0,-1.0);
    rasToLps->SetElement(1,1,-1.0);

    vtkMatrix4x4 *ijkToLps = vtkMatrix4x4::New();
    vtkMatrix4x4::Multiply4x4(ijkToRas, rasToLps, ijkToLps);

    // normalize direction vectors and compute the lps spacing
    double spacing[3];
    int row;
    for (row=0; row<3; row++) 
      {
      double len =0;
      int col;
      for (col=0; col<3; col++) 
        {
        len += ijkToLps->GetElement(row, col) * ijkToLps->GetElement(row, col);
        }
      len = sqrt(len);
      spacing[row] = len;
      for (col=0; col<3; col++) 
        {
        ijkToLps->SetElement(row, col,  ijkToLps->GetElement(row, col)/len);
        }
      }

    m_Spacing.resize(3);
    m_Origin.resize(3);
    m_Direction.resize(3);

    int col, i=0;
    for (row=0; row<3; row++) 
      {
      m_Direction[row].resize(3);
      m_Spacing[row] = 0.0;
      for (col=0; col<3; col++) 
        {
        m_Direction[row][col] = ijkToLps->GetElement(row, col);
        m_Spacing[row] += ijkToLps->GetElement(row, col) * spacing[col];
        }
      m_Origin[row] = ijkToLps->GetElement(row,3);
      m_Spacing[row] = fabs(m_Spacing[row]);
      }

    m_Origin[0] *= -1.0; // R -> L
    m_Origin[1] *= -1.0; // A -> P

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
    // Cleanup
    ijkToRas->Delete();
    rasToLps->Delete();
    ijkToLps->Delete();
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
    vtkMatrix4x4* ijkToLps = vtkMatrix4x4::New();
    for (int i = 0; i < 3; i++)
      {
      // Get IJK to RAS direction vector
      for ( unsigned int j=0; j < 3; j++ )
        {
        ijkToLps->SetElement(j, i, m_Spacing[i]*m_Direction[j][i]);
        }
      if (i < 2)
        {
        ijkToLps->SetElement(i, 3, -m_Origin[i]);      
        }
      else
        {
        ijkToLps->SetElement(i, 3, m_Origin[i]);      
        }
      }
    
    vtkMatrix4x4* lpsToRas = vtkMatrix4x4::New();
    lpsToRas->Identity();
    lpsToRas->SetElement(0,0,-1);
    lpsToRas->SetElement(1,1,-1);

    vtkMatrix4x4* ijkToRas = vtkMatrix4x4::New();
    vtkMatrix4x4::Multiply4x4(ijkToLps, lpsToRas, ijkToRas);

    node->SetIJKToRASMatrix(ijkToRas);

    // Fill in dimensions
    // VTK is only 3D, only copy the first 3 dimensions, fill in with
    // reasonable defaults for the rest
    if (this->GetNumberOfDimensions() > 3)
      {
      itkWarningMacro("Dimension of image is too high for VTK (Dimension = "
                    << this->GetNumberOfDimensions() << ")" );
      }
    
    unsigned int i;
    int dim[3];
    double origin[3];
    double spacing[3];

    for (i=0; (i < this->GetNumberOfDimensions()) && (i < 3); ++i)
      {
      dim[i] = this->GetDimensions(i);
      origin[i] = 0.0;
      spacing[i] = 1.0;
      }
    if (this->GetNumberOfDimensions() < 3)
      {
      // VTK is only 3D, fill in remaining dimensions
      for (; i < 3; ++i)
        {
        dim[i] = 1;
        origin[i] = 0.0;
        spacing[i] = 1.0;
        }
      }
    node->GetImageData()->SetDimensions(dim);
    node->GetImageData()->SetOrigin(origin);
    node->GetImageData()->SetSpacing(spacing);

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
    // Cleanup
    ijkToRas->Delete();
    lpsToRas->Delete();
    ijkToLps->Delete();
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
    // Need to create a VTK ImageData to hang off the node if there is
    // not one already there
    vtkImageData *img = 0;
    img = node->GetImageData();
    if (!img)
      {
      img = vtkImageData::New();
      node->SetImageData(img);
      img->Delete();
      }

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
