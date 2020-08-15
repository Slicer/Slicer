#include <map>

#include "vtkTeemNRRDWriter.h"


#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include <vtkVersion.h>

#include <vnl/vnl_math.h>
#include <vnl/vnl_double_3.h>

#include "itkNumberToString.h"


class AttributeMapType: public std::map<std::string, std::string> {};
class AxisInfoMapType : public std::map<unsigned int, std::string> {};

vtkStandardNewMacro(vtkTeemNRRDWriter);

//----------------------------------------------------------------------------
vtkTeemNRRDWriter::vtkTeemNRRDWriter()
{
  this->FileName = nullptr;
  this->BValues = vtkDoubleArray::New();
  this->DiffusionGradients = vtkDoubleArray::New();
  this->IJKToRASMatrix = vtkMatrix4x4::New();
  this->MeasurementFrameMatrix = vtkMatrix4x4::New();
  this->UseCompression = 1;
  // use default CompressionLevel
  this->CompressionLevel = -1;
  this->DiffusionWeightedData = 0;
  this->FileType = VTK_BINARY;
  this->WriteErrorOff();
  this->Attributes = new AttributeMapType;
  this->AxisLabels = new AxisInfoMapType;
  this->AxisUnits = new AxisInfoMapType;
  this->VectorAxisKind = nrrdKindUnknown;
  this->Space = nrrdSpaceRightAnteriorSuperior;
}

//----------------------------------------------------------------------------
vtkTeemNRRDWriter::~vtkTeemNRRDWriter()
{
  this->SetFileName(nullptr);
  this->SetDiffusionGradients(nullptr);
  this->SetBValues(nullptr);
  this->SetIJKToRASMatrix(nullptr);
  this->SetMeasurementFrameMatrix(nullptr);
  delete this->Attributes;
  this->Attributes = nullptr;
  delete this->AxisLabels;
  this->AxisLabels = nullptr;
  delete this->AxisUnits;
  this->AxisUnits = nullptr;
}

//----------------------------------------------------------------------------
vtkImageData* vtkTeemNRRDWriter::GetInput()
{
  return vtkImageData::SafeDownCast(this->Superclass::GetInput());
}

//----------------------------------------------------------------------------
vtkImageData* vtkTeemNRRDWriter::GetInput(int port)
{
  return vtkImageData::SafeDownCast(this->Superclass::GetInput(port));
}

//----------------------------------------------------------------------------
int vtkTeemNRRDWriter::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;

}

//----------------------------------------------------------------------------
// Writes all the data from the input.
void vtkTeemNRRDWriter::vtkImageDataInfoToNrrdInfo(vtkImageData *in, int &kind, size_t &numComp, int &vtkType, void **buffer)
{
  vtkDataArray *array = nullptr;
  this->DiffusionWeightedData = 0;
  if ((array = static_cast<vtkDataArray *> (in->GetPointData()->GetScalars())))
    {
    numComp = array->GetNumberOfComponents();
    vtkType = array->GetDataType();
    (*buffer) = array->GetVoidPointer(0);

    switch (numComp)
      {
      case 1:
        kind = nrrdKindScalar;
        break;
      case 2:
        kind = nrrdKindComplex;
        break;
      case 3:
        kind = nrrdKindRGBColor;
        break;
      case 4:
        kind = nrrdKindRGBAColor;
        break;
      default:
        size_t numGrad = this->DiffusionGradients->GetNumberOfTuples();
        size_t numBValues = this->BValues->GetNumberOfTuples();
        if (numGrad == numBValues && numGrad == numComp && numGrad>6)
          {
          kind = nrrdKindList;
          this->DiffusionWeightedData = 1;
          }
        else
          {
          kind = nrrdKindList;
          }
       }
     }
   else if ((array = static_cast<vtkDataArray *> ( in->GetPointData()->GetVectors())))
     {
     *buffer = array->GetVoidPointer(0);
     vtkType = array->GetDataType();
     kind = nrrdKindVector;
     }
   else if ((array = static_cast<vtkDataArray *> ( in->GetPointData()->GetNormals())))
     {
     *buffer = array->GetVoidPointer(0);
     vtkType = array->GetDataType();
     kind = nrrdKindVector;
     numComp = array->GetNumberOfComponents();
     }
   else if ((array = static_cast<vtkDataArray *> ( in->GetPointData()->GetTensors())))
     {
     *buffer = array->GetVoidPointer(0);
     vtkType = array->GetDataType();
     kind = nrrdKind3DMatrix;
     numComp = array->GetNumberOfComponents();
     }
   else
     {
     *buffer = nullptr;
     vtkType = VTK_VOID;
     kind = nrrdKindUnknown;
     numComp = 0;
     }

  if (this->VectorAxisKind != nrrdKindUnknown)
    {
    kind = this->VectorAxisKind;
    }
}

int vtkTeemNRRDWriter::VTKToNrrdPixelType( const int vtkPixelType )
  {
  switch( vtkPixelType )
    {
    default:
    case VTK_VOID:
      return nrrdTypeDefault;
      break;
    case VTK_CHAR:
      return nrrdTypeChar;
      break;
    case VTK_UNSIGNED_CHAR:
      return nrrdTypeUChar;
      break;
    case VTK_SHORT:
      return nrrdTypeShort;
      break;
    case VTK_UNSIGNED_SHORT:
      return nrrdTypeUShort;
      break;
      //    case nrrdTypeLLong:
      //      return LONG ;
      //      break;
      //    case nrrdTypeULong:
      //      return ULONG;
      //      break;
    case VTK_INT:
      return nrrdTypeInt;
      break;
    case VTK_UNSIGNED_INT:
      return nrrdTypeUInt;
      break;
    case VTK_FLOAT:
      return nrrdTypeFloat;
      break;
    case VTK_DOUBLE:
      return nrrdTypeDouble;
      break;
    }
  }

void* vtkTeemNRRDWriter::MakeNRRD()
  {
  // Fill in image information.
  if (this->Space != nrrdSpaceRightAnteriorSuperior && this->Space != nrrdSpaceRightAnteriorSuperiorTime)
    {
    if (this->GetInput()->GetPointData()->GetTensors())
      {
      vtkErrorMacro("Write: Can only NRRD with tensors in RAS space");
      return nullptr;
      }
    if (this->MeasurementFrameMatrix)
      {
      vtkErrorMacro("Write: Can only NRRD with a measurement frame in RAS space");
      return nullptr;
      }
    }

  // Find Pixel type from data and select a buffer.
  int kind[NRRD_DIM_MAX] = { nrrdKindUnknown };
  size_t size[NRRD_DIM_MAX] = { 0 };
  int vtkType = VTK_VOID;
  void* buffer = nullptr;
  this->vtkImageDataInfoToNrrdInfo(this->GetInput(), kind[0], size[0], vtkType, &buffer);

  double spaceDir[NRRD_DIM_MAX][NRRD_SPACE_DIM_MAX] = { 0.0 };
  unsigned int baseDim = 0;
  const unsigned int spaceDim = 3; // VTK is always 3D volumes.
  if (size[0] > 1)
    {
    // the range axis has no space direction
    for (unsigned int saxi=0; saxi < spaceDim; saxi++)
      {
      spaceDir[0][saxi] = AIR_NAN;
      }
    baseDim = 1;
    }
  else
    {
    baseDim = 0;
    }
  unsigned int nrrdDim = baseDim + spaceDim;

  vtkNew<vtkMatrix4x4> rasToSpaceMatrix;
  switch (this->Space)
    {
    case nrrdSpaceRightAnteriorSuperior:
    case nrrdSpaceRightAnteriorSuperiorTime:
      break;
    case nrrdSpaceLeftPosteriorSuperior:
    case nrrdSpaceLeftPosteriorSuperiorTime:
      rasToSpaceMatrix->SetElement(0, 0, -1);
      rasToSpaceMatrix->SetElement(1, 1, -1);
      break;
    default:
      vtkErrorMacro("Write: Unsupported space " << this->Space << " for " << this->GetFileName());
      return nullptr;
    }
  vtkNew<vtkMatrix4x4> ijkToSpaceMatrix;
  vtkMatrix4x4::Multiply4x4(rasToSpaceMatrix, this->IJKToRASMatrix, ijkToSpaceMatrix);

  double origin[NRRD_DIM_MAX] = { 0.0 };
  for (unsigned int axi=0; axi < spaceDim; axi++)
    {
    size[axi+baseDim] = this->GetInput()->GetDimensions()[axi];
    kind[axi+baseDim] = nrrdKindDomain;
    origin[axi] = ijkToSpaceMatrix->GetElement((int) axi,3);

    for (unsigned int saxi=0; saxi < spaceDim; saxi++)
      {
      spaceDir[axi+baseDim][saxi] = ijkToSpaceMatrix->GetElement(saxi,axi);
      }
    }

  Nrrd* nrrd = nrrdNew();
  if (nrrdWrap_nva(nrrd, const_cast<void *> (buffer),
                   this->VTKToNrrdPixelType( vtkType ),
                   nrrdDim, size)
      || nrrdSpaceDimensionSet(nrrd, spaceDim)
      || nrrdSpaceOriginSet(nrrd, origin))
    {
    char *err = biffGetDone(NRRD); // would be nice to free(err)
    vtkErrorMacro("Write: Error wrapping nrrd for "
                      << this->GetFileName() << ":\n" << err);
    // Free the nrrd struct but don't touch nrrd->data
    nrrd = nrrdNix(nrrd);
    return nullptr;
    }
  nrrdAxisInfoSet_nva(nrrd, nrrdAxisInfoKind, kind);
  nrrdAxisInfoSet_nva(nrrd, nrrdAxisInfoSpaceDirection, spaceDir);
  nrrd->space = this->Space;

  if (!this->AxisLabels->empty())
    {
    const char* labels[NRRD_DIM_MAX] = { nullptr };
    for (unsigned int axi = 0; axi < NRRD_DIM_MAX; axi++)
      {
      if (this->AxisLabels->find(axi) != this->AxisLabels->end())
        {
        labels[axi] = (*this->AxisLabels)[axi].c_str();
        }
      }
    nrrdAxisInfoSet_nva(nrrd, nrrdAxisInfoLabel, labels);
    }

  if (!this->AxisUnits->empty())
    {
    const char* units[NRRD_DIM_MAX] = { nullptr };
    for (unsigned int axi = 0; axi < NRRD_DIM_MAX; axi++)
      {
      if (this->AxisUnits->find(axi) != this->AxisUnits->end())
        {
        units[axi] = (*this->AxisUnits)[axi].c_str();
        }
      }
    nrrdAxisInfoSet_nva(nrrd, nrrdAxisInfoUnits, units);
    }

  // Write out attributes, diffusion information and the measurement frame.
  //

  // 0. Write out any attributes. Write non-specific attributes first. This allows
  // variables like the diffusion gradients and B-values that are saved as
  // attributes on disk to overwrite the attributes later. (This
  // should ensure the version written to disk has the best information.)
  AttributeMapType::iterator ait;
  for (ait = this->Attributes->begin(); ait != this->Attributes->end(); ++ait)
    {
    // Don't set `space` as k-v. it is handled above, and needs to be a nrrd *field*.
    if (ait->first == "space") { continue; }

    nrrdKeyValueAdd(nrrd, (*ait).first.c_str(), (*ait).second.c_str());
    }

  // 1. Measurement Frame
  if (this->MeasurementFrameMatrix)
    {
    for (unsigned int saxi=0; saxi < nrrd->spaceDim; saxi++)
      {
      for (unsigned int saxj=0; saxj < nrrd->spaceDim; saxj++)
        {
        // Note the transpose: each entry in the nrrd measurementFrame
        // is a column of the matrix
        nrrd->measurementFrame[saxi][saxj] = this->MeasurementFrameMatrix->GetElement(saxj,saxi);
        }
      }
    }

  // use double-conversion library (via ITK) for better
  // float64 string representation.
  itk::NumberToString<double> DoubleConvert;

  // 2. Take care about diffusion data
  if (this->DiffusionWeightedData)
    {
    unsigned int numGrad = this->DiffusionGradients->GetNumberOfTuples();
    unsigned int numBValues = this->BValues->GetNumberOfTuples();

    if (kind[0] == nrrdKindList && numGrad == size[0] && numBValues == size[0])
      {
      // This is diffusion Data
      vnl_double_3 grad;
      double bVal, factor;
      double maxbVal = this->BValues->GetRange()[1];

      std::string modality_key("modality");
      std::string modality_value("DWMRI");
      nrrdKeyValueAdd(nrrd, modality_key.c_str(), modality_value.c_str());

      std::string bval_key("DWMRI_b-value");
      std::stringstream bval_value;
      bval_value << DoubleConvert(maxbVal);
      nrrdKeyValueAdd(nrrd, bval_key.c_str(), bval_value.str().c_str());

      for (unsigned int ig =0; ig < numGrad; ig++)
        {
        // key
        std::stringstream key_stream;
        key_stream << "DWMRI_gradient_" << setfill('0') << setw(4) << ig;

        // gradient value
        grad.copy_in(this->DiffusionGradients->GetTuple3(ig));

        bVal = this->BValues->GetValue(ig);
        // per NA-MIC DWI convention
        factor = sqrt(bVal/maxbVal);
        std::stringstream value_stream;
        value_stream << std::setprecision(17) <<
                        DoubleConvert(grad[0] * factor) << "   " <<
                        DoubleConvert(grad[1] * factor) << "   " <<
                        DoubleConvert(grad[2] * factor);

        nrrdKeyValueAdd(nrrd, key_stream.str().c_str(), value_stream.str().c_str());
        }
      }
    }
  return nrrd;
  }

//----------------------------------------------------------------------------
// Writes all the data from the input.
void vtkTeemNRRDWriter::WriteData()
{
  this->WriteErrorOff();
  if (this->GetFileName() == nullptr)
    {
    vtkErrorMacro("FileName has not been set. Cannot save file");
    this->WriteErrorOn();
    return;
    }

  Nrrd* nrrd = (Nrrd*)this->MakeNRRD();
  if (nrrd == nullptr)
    {
    vtkErrorMacro("Failed to initialize NRRD image writing for " << this->GetFileName());
    this->WriteErrorOn();
    return;
    }

  NrrdIoState *nio = nrrdIoStateNew();

  // set encoding for data: compressed (raw), (uncompressed) raw, or ascii
  if ( this->GetUseCompression() && nrrdEncodingGzip->available() )
    {
    // this is necessarily gzip-compressed *raw* data
    nio->encoding = nrrdEncodingGzip;
    nio->zlibLevel = this->CompressionLevel;
    }
  else
    {
    int fileType = this->GetFileType();
    switch ( fileType )
      {
      default:
      case VTK_BINARY:
        nio->encoding = nrrdEncodingRaw;
        break;
      case VTK_ASCII:
        nio->encoding = nrrdEncodingAscii;
        break;
      }
    }

  // set endianness as unknown of output
  nio->endian = airEndianUnknown;

  // Write the nrrd to file.
  if (nrrdSave(this->GetFileName(), nrrd, nio))
    {
    char *err = biffGetDone(NRRD); // would be nice to free(err)
    vtkErrorMacro("Write: Error writing "
                      << this->GetFileName() << ":\n" << err);
    this->WriteErrorOn();
    }
  // Free the nrrd struct but don't touch nrrd->data
  nrrd = nrrdNix(nrrd);
  nio = nrrdIoStateNix(nio);
}

void vtkTeemNRRDWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "RAS to IJK Matrix: ";
     this->IJKToRASMatrix->PrintSelf(os,indent);
  os << indent << "Measurement frame: ";
     this->MeasurementFrameMatrix->PrintSelf(os,indent);
}

void vtkTeemNRRDWriter::SetAttribute(const std::string& name, const std::string& value)
{
  if (!this->Attributes)
    {
    return;
    }

  (*this->Attributes)[name] = value;
}

void vtkTeemNRRDWriter::SetAxisLabel(unsigned int axis, const char* label)
{
  if (!this->AxisLabels)
    {
    return;
    }
  if (label)
    {
    (*this->AxisLabels)[axis] = label;
    }
  else
    {
    this->AxisLabels->erase(axis);
    }
}

void vtkTeemNRRDWriter::SetAxisUnit(unsigned int axis, const char* unit)
{
  if (!this->AxisUnits)
    {
    return;
    }
  if (unit)
    {
    (*this->AxisUnits)[axis] = unit;
    }
  else
    {
    this->AxisLabels->erase(axis);
    }
}

void vtkTeemNRRDWriter::SetVectorAxisKind(int kind)
{
  this->VectorAxisKind = kind;
}
