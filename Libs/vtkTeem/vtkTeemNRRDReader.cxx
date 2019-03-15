/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTeemNRRDReader.cxx,v $
  Date:      $Date: 2007/06/12 19:13:58 $
  Version:   $Revision: 1.7.2.1 $

=========================================================================auto=*/
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkTeemNRRDReader.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// vtkTeem includes
#include "vtkTeemNRRDReader.h"

// VTK includes
#include "vtkBitArray.h"
#include "vtkCharArray.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include "vtkIntArray.h"
#include "vtkLongArray.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkShortArray.h"
#include <vtkStreamingDemandDrivenPipeline.h>
#include "vtkUnsignedCharArray.h"
#include "vtkUnsignedShortArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkUnsignedLongArray.h"
#include <vtksys/SystemTools.hxx>

// Teem includes
#include "teem/ten.h"

vtkStandardNewMacro(vtkTeemNRRDReader);

//----------------------------------------------------------------------------
vtkTeemNRRDReader::vtkTeemNRRDReader()
{
  this->RasToIjkMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->NRRDWorldToRasMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->MeasurementFrameMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  this->nrrd = nrrdNew();
  this->UseNativeOrigin = true;
  this->ReadStatus = 0;
  this->PointDataType = -1;
  this->DataType = -1;
  this->NumberOfComponents = -1;
}

//----------------------------------------------------------------------------
vtkTeemNRRDReader::~vtkTeemNRRDReader()
{
  nrrdNuke(this->nrrd);
  this->nrrd = nullptr;
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkTeemNRRDReader::GetRasToIjkMatrix()
{
  this->ExecuteInformation();
  return this->RasToIjkMatrix;
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkTeemNRRDReader::GetMeasurementFrameMatrix()
{
  this->ExecuteInformation();
  return this->MeasurementFrameMatrix;
}

//----------------------------------------------------------------------------
const char* vtkTeemNRRDReader::GetHeaderKeys()
{
  this->HeaderKeys.clear();
  for (std::map<std::string,std::string>::iterator i = HeaderKeyValue.begin(); i != HeaderKeyValue.end(); i++)
  {
    if (!this->HeaderKeys.empty())
    {
      this->HeaderKeys += " ";
    }
    this->HeaderKeys += (i->first);
  }
  return this->HeaderKeys.c_str();
}

//----------------------------------------------------------------------------
const std::map<std::string,std::string> vtkTeemNRRDReader::GetHeaderKeysMap()
{
  return this->HeaderKeyValue;
}

//----------------------------------------------------------------------------
const std::vector<std::string> vtkTeemNRRDReader::GetHeaderKeysVector()
{
  std::vector<std::string> keys;
  for (std::map<std::string,std::string>::iterator i = HeaderKeyValue.begin(); i != HeaderKeyValue.end(); i++)
    {
    keys.push_back( i->first );
    }
  return keys;
}

//----------------------------------------------------------------------------
const char* vtkTeemNRRDReader::GetHeaderValue(const char *key)
{
  std::map<std::string,std::string>::iterator i = HeaderKeyValue.find(key);
  if (i != HeaderKeyValue.end())
  {
    return (i->second.c_str());
  }
  else
  {
    return nullptr;
  }
}

//----------------------------------------------------------------------------
const char* vtkTeemNRRDReader::GetAxisLabel(unsigned int axis)
{
  if (this->AxisLabels.find(axis) == this->AxisLabels.end())
    {
    return nullptr;
    }
  return this->AxisLabels[axis].c_str();
}

//----------------------------------------------------------------------------
const char* vtkTeemNRRDReader::GetAxisUnit(unsigned int axis)
{
  if (this->AxisUnits.find(axis) == this->AxisUnits.end())
    {
    return nullptr;
    }
  return this->AxisUnits[axis].c_str();
}

//----------------------------------------------------------------------------
int vtkTeemNRRDReader::CanReadFile(const char* filename)
{
  // Check the extension first to avoid opening files that do not
  // look like nrrds.  The file must have an appropriate extension to be
  // recognized.
  if (!filename)
    {
    vtkDebugMacro(<<"No filename specified.");
    return false;
    }

  // We'll assume we can read from stdin (don't try to read the header though)
  //if ( fname == "-" )
  //  {
  //  return true;
  //  }

  std::string extension = vtksys::SystemTools::LowerCase( vtksys::SystemTools::GetFilenameLastExtension(filename) );
  if (extension != ".nrrd" && extension != ".nhdr")
    {
    vtkDebugMacro("The filename extension is not recognized");
    return false;
    }

  // We have the correct extension, so now check for the Nrrd magic.
  std::ifstream inputStream;
  inputStream.open(filename, std::ios::in | std::ios::binary);
  if (inputStream.fail())
    {
    return false;
    }

  char magic[5] = {'\0','\0','\0','\0','\0'};
  inputStream.read(magic,4*sizeof(char));

  if (inputStream.eof())
    {
    inputStream.close();
    return false;
    }

  if (strcmp(magic,"NRRD")!=0)
    {
    inputStream.close();
    return false;
    }

  inputStream.close();

  // Look into the file to avoid claiming the file is readable when it's actually not
  Nrrd* nrrdTemp = nrrdNew();
  NrrdIoState *nio = nrrdIoStateNew();
  // Tell nrrdLoad to read just the header, and none of the data
  nrrdIoStateSet(nio, nrrdIoStateSkipData, 1);

  bool supported = true;
  if (nrrdLoad(nrrdTemp, filename, nio) != 0)
    {
    supported = false;
    }
  if (nrrdTypeBlock == nrrdTemp->type)
    {
    supported = false;
    }
  int pointDataType = -1;
  int numOfComponents = -1;
  if (!vtkTeemNRRDReader::GetPointType(nrrdTemp, pointDataType, numOfComponents))
    {
    supported = false;
    }

  nrrdNuke(nrrdTemp);
  nrrdIoStateNix(nio);
  return supported;
}

//----------------------------------------------------------------------------
bool vtkTeemNRRDReader::GetPointType(Nrrd* nrrdTemp, int& pointDataType, int &numOfComponents)
{
  // vtkTeemNRRDReader only supports 3 or 4 dimensional image with scalar, vector,
  // normal or tensor data. Other dimensionality is considered a multicomponent scalar field.

  if (nrrdTemp->dim < 3 || nrrdTemp->dim > 4)
    {
    // Only 3D/4D volumes supported.
    return false;
    }

  unsigned int rangeAxisIdx[NRRD_DIM_MAX];
  unsigned int rangeAxisNum = nrrdRangeAxesGet(nrrdTemp, rangeAxisIdx);

  // 3D volumes
  if (nrrdTemp->dim  == 3)
    {
    if (rangeAxisNum != 0)
      {
      // 3D volumes supported only with 3 spatial dimensions
      return false;
      }
    // Assume that all dimensions are spatial.
    // We don't have any non-scalar data.
    pointDataType = vtkDataSetAttributes::SCALARS;
    numOfComponents = 1;
    return true;
    }

  // 4D volumes
  if (rangeAxisNum != 1)
    {
    // 4D volumes supported only with 1 range axis
    return false;
    }

  unsigned int rangeAxisKind = nrrdTemp->axis[rangeAxisIdx[0]].kind;
  int sizeAlongRangeAxis = static_cast<int>(nrrdTemp->axis[rangeAxisIdx[0]].size);
  // NOTE: it is the NRRD readers responsibility to make sure that
  // the size (# of components) associated with a specific kind is
  // matches the actual size of the axis.
  if (rangeAxisKind == nrrdKindDomain
    || rangeAxisKind == nrrdKindSpace
    || rangeAxisKind == nrrdKindTime)
    {
    // Range axis kind seems more like a domain axis than a range axis.
    return false;
    }

  if (rangeAxisKind == nrrdKindStub || rangeAxisKind == nrrdKindScalar || rangeAxisKind == nrrdKind3Color
    || rangeAxisKind == nrrdKindRGBColor || rangeAxisKind == nrrdKind4Color || rangeAxisKind == nrrdKindRGBAColor
    || rangeAxisKind == nrrdKindComplex || rangeAxisKind == nrrdKindHSVColor || rangeAxisKind == nrrdKindXYZColor
    || rangeAxisKind == nrrdKindQuaternion || rangeAxisKind == nrrdKind2DSymMatrix || rangeAxisKind == nrrdKind2DMaskedSymMatrix
    || rangeAxisKind == nrrdKind2DMatrix || rangeAxisKind == nrrdKind2DMaskedMatrix || rangeAxisKind == nrrdKind2Vector
    || rangeAxisKind == nrrdKind4Vector || rangeAxisKind == nrrdKindList || rangeAxisKind == nrrdKindPoint
    || rangeAxisKind == nrrdKind3Vector || rangeAxisKind == nrrdKind3Gradient || rangeAxisKind == nrrdKindVector
    || rangeAxisKind == nrrdKindCovariantVector)
    {
    pointDataType = vtkDataSetAttributes::SCALARS;
    numOfComponents = sizeAlongRangeAxis;
    return true;
    }

  if (rangeAxisKind == nrrdKindNormal || rangeAxisKind == nrrdKind3Normal)
  {
    if (sizeAlongRangeAxis == 3)
    {
      pointDataType = vtkDataSetAttributes::NORMALS;
    }
    else
    {
      pointDataType = vtkDataSetAttributes::SCALARS;
    }
    numOfComponents = sizeAlongRangeAxis;
    return true;
  }

  if (rangeAxisKind == nrrdKind3DMaskedSymMatrix || rangeAxisKind == nrrdKind3DSymMatrix
    || rangeAxisKind == nrrdKind3DMatrix)
    {
    // NOTE: in case of nrrdKind3DMaskedSymMatrix and nrrdKind3DSymMatrix we will crop
    // out the mask in Read() below and expand the 6 values into 9,
    // so this->NumberOfComponents != sizeAlongRangeAxis.
    pointDataType = vtkDataSetAttributes::TENSORS;
    numOfComponents = 9;
    return true;
    }

  // unsupported rangeAxisKind
  return false;
}

//----------------------------------------------------------------------------
void vtkTeemNRRDReader::ExecuteInformation()
{
  // This method determines the following and sets the appropriate value in
  // the parent IO class:
  //
  // binary/ascii file type
  // endianness
  // pixel type
  // pixel component type
  // number of pixel components
  // number of image dimensions
  // image spacing
  // image origin
  // meta data dictionary information

  // save the Nrrd struct for the current file and
  // don't re-execute the read unless the filename changes
  if (this->CurrentFileName.compare(this->GetFileName()) == 0)
    {
    // filename hasn't changed, don't re-execute
    return;
    }
  this->CurrentFileName = this->GetFileName();

  nrrdNuke(this->nrrd); // nuke and reallocate to reset the state
  this->nrrd = nrrdNew();

  NrrdIoState *nio = nrrdIoStateNew();

  // Tell nrrdLoad to read just the header, and none of the data
  nrrdIoStateSet(nio, nrrdIoStateSkipData, 1);

  if (nrrdLoad(this->nrrd, this->GetFileName(), nio) != 0)
    {
    char *err = biffGetDone(NRRD);
    vtkErrorMacro("Error reading " << this->GetFileName() << ": " << err);
    free(err); // err points to malloc'd data!!
    err = nullptr;
    nio = nrrdIoStateNix(nio);
    this->ReadStatus = 1;
    return;
    }

  HeaderKeyValue.clear();

  this->RasToIjkMatrix->Identity();
  this->MeasurementFrameMatrix->Identity();
  this->NRRDWorldToRasMatrix->Identity();

  if (nrrdTypeBlock == this->nrrd->type)
    {
    vtkErrorMacro("ReadImageInformation: Cannot currently handle nrrdTypeBlock");
    nio = nrrdIoStateNix(nio);
    this->ReadStatus = 1;
    return;
    }

  if (nio->endian == airEndianLittle)
    {
    this->SetDataByteOrderToLittleEndian();
    }
  else
    {
    this->SetDataByteOrderToBigEndian();
    }

  //if ( nio->encoding == nrrdEncodingAscii )
  //  {
  //  this->SetFileTypeToASCII();
  //  }
  //else
  //  {
  //  this->SetFileTypeToBinary();
  //  }

  // Set the number of image dimensions and bail if needed
  unsigned int domainAxisIdx[NRRD_DIM_MAX];
  unsigned int domainAxisNum = nrrdDomainAxesGet(this->nrrd, domainAxisIdx);
  if (this->nrrd->spaceDim && this->nrrd->spaceDim != domainAxisNum)
    {
    vtkErrorMacro("ReadImageInformation: this->nrrd's # independent axes ("
      << domainAxisNum << ") doesn't match dimension of space"
      " in which orientation is defined ("
      << this->nrrd->spaceDim << "); not currently handled");
    nio = nrrdIoStateNix(nio);
    this->ReadStatus = 1;
    return;
    }

  int pointDataType = -1;
  int numOfComponents = -1;
  if (!vtkTeemNRRDReader::GetPointType(this->nrrd, pointDataType, numOfComponents))
    {
    vtkErrorMacro("ReadImageInformation: only 3 spatial dimension and 1 optional range axis is supported");
    nio = nrrdIoStateNix(nio);
    this->ReadStatus = 1;
    return;
    }
  this->SetPointDataType(pointDataType);
  this->SetNumberOfComponents(numOfComponents);

  // Set type information
  this->SetDataType(this->NrrdToVTKScalarType(this->nrrd->type));
  this->SetDataScalarType(this->NrrdToVTKScalarType(this->nrrd->type));

  // Set axis information
  int dataExtent[6] = { 0 };
  double spacing[3] = { 1.0 };
  double origin[3] = { 0 };
  vtkNew<vtkMatrix4x4> ijkToRasMatrix;
  if (domainAxisNum > 3)
  {
    vtkErrorMacro("ReadImageInformation: only up to 3 domain axes are supported");
    nio = nrrdIoStateNix(nio);
    this->ReadStatus = 1;
    return;
  }
  for (unsigned int axii = 0; axii < domainAxisNum; axii++)
    {
    unsigned int naxi = domainAxisIdx[axii];
    dataExtent[2 * axii] = 0;
    dataExtent[2 * axii + 1] = static_cast<int>(this->nrrd->axis[naxi].size) - 1;

    double spaceDir[NRRD_SPACE_DIM_MAX];
    double axisSpacing = 1.0;
    int spacingStatus = nrrdSpacingCalculate(this->nrrd, naxi, &axisSpacing, spaceDir);
    switch (spacingStatus)
      {
      case nrrdSpacingStatusNone:
        spacing[axii] = 1.0;
        break;
      case nrrdSpacingStatusScalarNoSpace:
        spacing[axii] = axisSpacing;
        ijkToRasMatrix->SetElement(axii, axii, axisSpacing);
        break;
      case nrrdSpacingStatusDirection:
        if (AIR_EXISTS(axisSpacing))
          {
          // only set info if we have something to set
          //this->SetSpacing(axii, axisSpacing);
          spacing[axii] = axisSpacing;

          switch (this->nrrd->space)
            {
              // on read, convert non-RAS coords into RAS coords, when we can
            case nrrdSpaceRightAnteriorSuperior:
              // no change needed
              this->NRRDWorldToRasMatrix->Identity();
              break;
            case nrrdSpaceLeftAnteriorSuperior:
              spaceDir[0] *= -1;   // L -> R
              this->NRRDWorldToRasMatrix->SetElement(0, 0, -1);
              break;
            case nrrdSpaceLeftPosteriorSuperior:
              spaceDir[0] *= -1;   // L -> R
              spaceDir[1] *= -1;   // P -> A
              this->NRRDWorldToRasMatrix->SetElement(0, 0, -1);
              this->NRRDWorldToRasMatrix->SetElement(1, 1, -1);
              break;
            default:
              // we're not coming from a space for which the conversion
              // to LPS is well-defined
              break;
            }

          for (int j = 0; (unsigned int)j < this->nrrd->spaceDim; j++)
            {
            ijkToRasMatrix->SetElement(j, axii, spaceDir[j] * axisSpacing);
            }
          }
        break;
      default:
      case nrrdSpacingStatusUnknown:
        vtkErrorMacro("ReadImageInformation: Error interpreting nrrd spacing (nrrdSpacingStatusUnknown)");
        break;
      case nrrdSpacingStatusScalarWithSpace:
        vtkErrorMacro("ReadImageInformation: Error interpreting nrrd spacing (nrrdSpacingStatusScalarWithSpace)");
        break;
      }
    }

  // Figure out origin
  if (this->nrrd->spaceDim == 3)
    {
    if (AIR_EXISTS(this->nrrd->spaceOrigin[0]))
      {
      // only set info if we have something to set
      for (unsigned int saxi = 0; saxi < this->nrrd->spaceDim; saxi++)
        {
        origin[saxi] = this->nrrd->spaceOrigin[saxi];
        }
      switch (this->nrrd->space)
        {
          // convert non-RAS coords into RAS coords, when we can
        case nrrdSpaceRightAnteriorSuperior:
          // no change needed
          break;
        case nrrdSpaceLeftAnteriorSuperior:
          origin[0] *= -1;   // L -> R
          break;
        case nrrdSpaceLeftPosteriorSuperior:
          origin[0] *= -1;   // L -> R
          origin[1] *= -1;   // P -> A
          break;
        default:
          // we're not coming from a space for which the conversion
          // to LPS is well-defined
          break;
        }
      }
    }
  else
    {
    double spaceOrigin[NRRD_DIM_MAX];
    int originStatus = nrrdOriginCalculate(this->nrrd, domainAxisIdx, domainAxisNum, nrrdCenterCell, spaceOrigin);
    for (unsigned int saxi = 0; saxi < domainAxisNum; saxi++)
      {
      switch (originStatus)
        {
        case nrrdOriginStatusNoMin:
        case nrrdOriginStatusNoMaxOrSpacing:
          // only set info if we have something to set
          // this->SetOrigin(saxi, 0.0);
          origin[saxi] = 0.0;
          break;
        case nrrdOriginStatusOkay:
          origin[saxi] = spaceOrigin[saxi];
          break;
        default:
        case nrrdOriginStatusUnknown:
        case nrrdOriginStatusDirection:
          vtkErrorMacro("ReadImageInformation: Error interpreting nrrd origin status");
          nio = nrrdIoStateNix(nio);
          this->ReadStatus = 1;
          break;
        }
      }
    }

  if (this->UseNativeOrigin && AIR_EXISTS(this->nrrd->spaceOrigin[0]))
    {
    for (int i = 0; i < 3; i++)
      {
      ijkToRasMatrix->SetElement(i, 3, origin[i]);
      }
    vtkMatrix4x4::Invert(ijkToRasMatrix.GetPointer(), this->RasToIjkMatrix);
    }
  else
    {
    vtkMatrix4x4::Invert(ijkToRasMatrix.GetPointer(), this->RasToIjkMatrix);
    for (int i = 0; i < 3; i++)
      {
      this->RasToIjkMatrix->SetElement(i, 3, (dataExtent[2 * i + 1] - dataExtent[2 * i]) / 2.0);
      }
    }

  this->RasToIjkMatrix->SetElement(3, 3, 1.0);

  this->SetDataSpacing(spacing);
  //this->SetDataOrigin(origin);
  this->SetDataExtent(dataExtent);

  // Push extra key/value pair data into std::map
  for (unsigned int i = 0; i < nrrdKeyValueSize(this->nrrd); i++)
    {
    char *key = nullptr;
    char *val = nullptr;
    nrrdKeyValueIndex(this->nrrd, &key, &val, i);
    HeaderKeyValue[std::string(key)] = std::string(val);
    free(key);  // key and val point to malloc'd data!!
    free(val);
    }

  const char* labels[NRRD_DIM_MAX] = { nullptr };
  nrrdAxisInfoGet_nva(nrrd, nrrdAxisInfoLabel, labels);
  this->AxisLabels.clear();
  for (unsigned int axi = 0; axi < NRRD_DIM_MAX; axi++)
    {
    if (labels[axi] != nullptr)
      {
      this->AxisLabels[axi] = labels[axi];
      }
    }

  const char* units[NRRD_DIM_MAX] = { nullptr };
  nrrdAxisInfoGet_nva(nrrd, nrrdAxisInfoUnits, units);
  this->AxisUnits.clear();
  for (unsigned int axi = 0; axi < NRRD_DIM_MAX; axi++)
    {
    if (units[axi] != nullptr)
      {
      this->AxisUnits[axi] = units[axi];
      }
    }

  if (this->nrrd->space)
    {
    HeaderKeyValue[std::string("space")] = std::string(airEnumStr(nrrdSpace, this->nrrd->space));
    }

  if (AIR_EXISTS(this->nrrd->measurementFrame[0][0]))
    {
    for (int i = 0; i < 3; i++)
      {
      switch (this->nrrd->space)
        {
        // WARNING: this->nrrd->measurementFrame[i][0:2] are the rows fo the measurementFrame matrix
        // on read, convert non-RAS coords into RAS coords, when we can
        case nrrdSpaceRightAnteriorSuperior:
          // no change needed
          this->MeasurementFrameMatrix->SetElement(0, i, this->nrrd->measurementFrame[i][0]);
          this->MeasurementFrameMatrix->SetElement(1, i, this->nrrd->measurementFrame[i][1]);
          this->MeasurementFrameMatrix->SetElement(2, i, this->nrrd->measurementFrame[i][2]);
          break;
        case nrrdSpaceLeftAnteriorSuperior:
          // L -> R
          this->MeasurementFrameMatrix->SetElement(0, i, -this->nrrd->measurementFrame[i][0]);
          this->MeasurementFrameMatrix->SetElement(1, i, this->nrrd->measurementFrame[i][1]);
          this->MeasurementFrameMatrix->SetElement(2, i, this->nrrd->measurementFrame[i][2]);
          break;
        case nrrdSpaceLeftPosteriorSuperior:
          // L -> R
          // P -> A
          this->MeasurementFrameMatrix->SetElement(0, i, -this->nrrd->measurementFrame[i][0]);
          this->MeasurementFrameMatrix->SetElement(1, i, -this->nrrd->measurementFrame[i][1]);
          this->MeasurementFrameMatrix->SetElement(2, i, this->nrrd->measurementFrame[i][2]);
          break;
        default:
          // we're not coming from a space for which the conversion
          // to LPS is well-defined
          this->MeasurementFrameMatrix->SetElement(0, i, this->nrrd->measurementFrame[i][0]);
          this->MeasurementFrameMatrix->SetElement(1, i, this->nrrd->measurementFrame[i][1]);
          this->MeasurementFrameMatrix->SetElement(2, i, this->nrrd->measurementFrame[i][2]);
          break;
        }
      }
    }

  this->vtkImageReader2::ExecuteInformation();
  nio = nrrdIoStateNix(nio);
}

//----------------------------------------------------------------------------
vtkImageData *vtkTeemNRRDReader::AllocateOutputData(vtkDataObject *out, vtkInformation* outInfo)
{
  vtkImageData *res = vtkImageData::SafeDownCast(out);
  if (!res)
    {
    vtkWarningMacro("Call to AllocateOutputData with non vtkImageData output");
    return nullptr;
    }

  // I would like to eliminate this method which requires extra "information"
  // That is not computed in the graphics pipeline.
  // Until I can eliminate the method, I will reexecute the ExecuteInformation
  // before the execute.
  this->ExecuteInformation();

  res->SetExtent(this->GetUpdateExtent());
  this->AllocatePointData(res, outInfo);

  return res;

}

//----------------------------------------------------------------------------
void vtkTeemNRRDReader::AllocatePointData(vtkImageData *out, vtkInformation* outInfo)
{
  // if the scalar type has not been set then we have a problem
  if (this->DataType == VTK_VOID)
    {
    vtkErrorMacro("Attempt to allocate scalars before scalar type was set!.");
    return;
    }

  // if we currently have scalars then just adjust the size
  vtkSmartPointer<vtkDataArray> pd;
  switch (this->PointDataType)
    {
    case vtkDataSetAttributes::SCALARS:
      pd = out->GetPointData()->GetScalars();
      break;
    case vtkDataSetAttributes::VECTORS:
      pd = out->GetPointData()->GetVectors();
      break;
    case vtkDataSetAttributes::NORMALS:
      pd = out->GetPointData()->GetNormals();
      break;
    case vtkDataSetAttributes::TENSORS:
      pd = out->GetPointData()->GetTensors();
      break;
    default:
      vtkErrorMacro("Unknown PointData Type.");
      return;
    }

  int extent[6] = { 0, -1, 0, -1, 0, -1 };
  out->GetExtent(extent);

  if (pd && pd->GetDataType() == this->DataType
    && pd->GetReferenceCount() == 1)
    {
    pd->SetNumberOfComponents(this->GetNumberOfComponents());
    pd->SetNumberOfTuples(vtkIdType(extent[1] - extent[0] + 1)*
      vtkIdType(extent[3] - extent[2] + 1)*
      vtkIdType(extent[5] - extent[4] + 1));
    // Since the execute method will be modifying the scalars
    // directly.
    pd->Modified();
    return;
    }

  // allocate the new scalars
  switch (this->DataType)
    {
    case VTK_BIT:
      pd = vtkSmartPointer<vtkBitArray>::New();
      break;
    case VTK_UNSIGNED_CHAR:
      pd = vtkSmartPointer<vtkUnsignedCharArray>::New();
      break;
    case VTK_CHAR:
      pd = vtkSmartPointer<vtkCharArray>::New();
      break;
    case VTK_UNSIGNED_SHORT:
      pd = vtkSmartPointer<vtkUnsignedShortArray>::New();
      break;
    case VTK_SHORT:
      pd = vtkSmartPointer<vtkShortArray>::New();
      break;
    case VTK_UNSIGNED_INT:
      pd = vtkSmartPointer<vtkUnsignedIntArray>::New();
      break;
    case VTK_INT:
      pd = vtkSmartPointer<vtkIntArray>::New();
      break;
    case VTK_UNSIGNED_LONG:
      pd = vtkSmartPointer<vtkUnsignedLongArray>::New();
      break;
    case VTK_LONG:
      pd = vtkSmartPointer<vtkLongArray>::New();
      break;
    case VTK_DOUBLE:
      pd = vtkSmartPointer<vtkDoubleArray>::New();
      break;
    case VTK_FLOAT:
      pd = vtkSmartPointer<vtkFloatArray>::New();
      break;
    default:
      vtkErrorMacro("Could not allocate data type.");
      return;
    }
  vtkDataObject::SetPointDataActiveScalarInfo(outInfo, this->DataType, this->GetNumberOfComponents());
  pd->SetNumberOfComponents(this->GetNumberOfComponents());

  // allocate enough memory
  pd->SetNumberOfTuples(vtkIdType(extent[1] - extent[0] + 1)*
    vtkIdType(extent[3] - extent[2] + 1)*
    vtkIdType(extent[5] - extent[4] + 1));

  switch (this->PointDataType)
    {
    case vtkDataSetAttributes::SCALARS:
      out->GetPointData()->SetScalars(pd);
      vtkDataObject::SetPointDataActiveScalarInfo(outInfo, this->DataType, this->GetNumberOfComponents());
      break;
    case vtkDataSetAttributes::VECTORS:
      out->GetPointData()->SetVectors(pd);
      break;
    case vtkDataSetAttributes::NORMALS:
      out->GetPointData()->SetNormals(pd);
      break;
    case vtkDataSetAttributes::TENSORS:
      out->GetPointData()->SetTensors(pd);
      break;
    default:
      vtkErrorMacro("Unknown PointData Type.");
      return;
    }
}

//----------------------------------------------------------------------------
int vtkTeemNRRDReader::tenSpaceDirectionReduce(Nrrd *nout, const Nrrd *nin, double SD[9])
{
  char me[] = "tenSpaceDirectionReduce";
  char err[BUFSIZ];

  if (!(nout && nin))
    {
    sprintf(err, "%s: got nullptr pointer", me);
    biffAdd(TEN, err);
    return 1;
    }
  if (tenTensorCheck(nin, nin->type, AIR_TRUE, AIR_TRUE))
    {
    sprintf(err, "%s: ", me);
    biffAdd(TEN, err);
    return 1;
    }
  if (nin->spaceDim != 3)
    {
    sprintf(err, "%s: input this->nrrd needs 3-D (not %u-D) space dimension",
            me, nin->spaceDim);
    biffAdd(TEN, err);
    return 1;
    }
  if (!ELL_3M_EXISTS(SD))
    {
    sprintf(err, "%s: 3x3 space direction doesn't exist", me);
    biffAdd(TEN, err);
    return 1;
    }

  double SDT[9];
  //ELL_3M_INV(SD, SDINV, det);
  ELL_3M_TRANSPOSE(SDT, SD);

  if (nout != nin)
    {
    if (nrrdCopy(nout, nin))
      {
      sprintf(err, "%s: trouble with initial copy", me);
      biffAdd(TEN, err);
      return 1;
      }
    }

  double tenMeasr[9];
  double tenSlice[9];
  size_t nn = nrrdElementNumber(nout) / nout->axis[0].size;
  switch (this->DataType)
    {
    case VTK_FLOAT:
      {
      float *tdata = (float*)(nout->data);
      for (size_t ii=0; ii<nn; ii++)
        {
        TEN_T2M(tenMeasr, tdata);
        ell_3m_mul_d(tenSlice, SD, tenMeasr);
        ell_3m_mul_d(tenSlice, tenSlice, SDT);
        TEN_M2T_TT(tdata, float, tenSlice);
        tdata += 7;
        }
      }
      break;
    case VTK_DOUBLE:
      {
      double *tdata = (double*)(nout->data);
      for (size_t ii = 0; ii<nn; ii++)
        {
        TEN_T2M(tenMeasr, tdata);
        ell_3m_mul_d(tenSlice, SD, tenMeasr);
        ell_3m_mul_d(tenSlice, tenSlice, SDT);
        TEN_M2T_TT(tdata, double, tenSlice);
        tdata += 7;
        }
      }
      break;
    default:
      return 1;
    }

  return 0;
}


//----------------------------------------------------------------------------
// This function reads a data from a file.  The datas extent/axes
// are assumed to be the same as the file extent/order.
void vtkTeemNRRDReader::ExecuteDataWithInformation(vtkDataObject *output, vtkInformation* outInfo)
{
  if (this->GetOutputInformation(0))
    {
    this->GetOutputInformation(0)->Set(
      vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
      this->GetOutputInformation(0)->Get(
        vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()), 6);
    }

  vtkImageData *imageData = this->AllocateOutputData(output, outInfo);

  if (this->GetFileName() == nullptr)
    {
    vtkErrorMacro(<< "Either a FileName or FilePrefix must be specified.");
    return;
    }

  // Read in the this->nrrd.  Yes, this means that the header is being read
  // twice: once by ExecuteInformation, and once here
  if ( nrrdLoad(this->nrrd, this->GetFileName(), nullptr) != 0 )
    {
    char *err =  biffGetDone(NRRD); // would be nice to free(err)
    vtkErrorMacro("Read: Error reading " << this->GetFileName() << ":\n" << err);
    return;
    }

  if (this->nrrd->data == nullptr)
    {
    vtkErrorMacro(<< "data is null.");
    return;
    }

  void *ptr = nullptr;
  switch(this->PointDataType)
    {
    case vtkDataSetAttributes::SCALARS:
      imageData->GetPointData()->GetScalars()->SetName("NRRDImage");
      //get pointer
      ptr = imageData->GetPointData()->GetScalars()->GetVoidPointer(0);
      break;
    case vtkDataSetAttributes::VECTORS:
      imageData->GetPointData()->GetVectors()->SetName("NRRDImage");
      //get pointer
      ptr = imageData->GetPointData()->GetVectors()->GetVoidPointer(0);
      break;
    case vtkDataSetAttributes::NORMALS:
      imageData->GetPointData()->GetNormals()->SetName("NRRDImage");
      ptr = imageData->GetPointData()->GetNormals()->GetVoidPointer(0);
      break;
    case vtkDataSetAttributes::TENSORS:
      imageData->GetPointData()->GetTensors()->SetName("NRRDImage");
      ptr = imageData->GetPointData()->GetTensors()->GetVoidPointer(0);
      break;
    }
  this->ComputeDataIncrements();

  unsigned int rangeAxisIdx[NRRD_DIM_MAX] = { 0 };
  unsigned int rangeAxisNum = nrrdRangeAxesGet(this->nrrd, rangeAxisIdx);
  if (rangeAxisNum > 1)
    {
    vtkErrorMacro("Read: handling more than one non-scalar axis not currently handled");
    return;
    }
  if (rangeAxisNum == 1 && rangeAxisIdx[0] != 0)
    {
    // the range (dependent variable) is not on the fastest axis,
    // so we have to permute axes to put it there, since that is
    // how we set things up in ReadImageInformation() above
    Nrrd *ntmp = nrrdNew();
    unsigned int axmap[NRRD_DIM_MAX] = { 0 };
    axmap[0] = rangeAxisIdx[0];
    for (unsigned int axi = 1; axi < this->nrrd->dim; axi++)
      {
      axmap[axi] = axi - (axi <= rangeAxisIdx[0]);
      }
    // The memory size of the input and output of nrrdAxesPermute is
    // the same; the existing this->nrrd->data is re-used.
    if (nrrdCopy(ntmp, this->nrrd)
      || nrrdAxesPermute(this->nrrd, ntmp, axmap))
      {
      char *err = biffGetDone(NRRD); // would be nice to free(err)
      vtkErrorMacro("Read: Error permuting independent axis in " << this->GetFileName() << ":\n" << err);
      return;
      }

    nrrdNuke(ntmp);
    }

  // "the famous y-flip": we always flip along the second domain axis
  //Nrrd *nflip = nrrdNew();
  //nrrdCopy(nflip, this->nrrd);
  //unsigned int domainAxisNum, domainAxisIdx[NRRD_DIM_MAX];
  //domainAxisNum = nrrdDomainAxesGet(this->nrrd, domainAxisIdx);
  //nrrdFlip(this->nrrd, nflip, domainAxisIdx[1]);
  //nrrdNuke(nflip);

  // Now we have to get the data back into the given VTK data pointer
  // In any case, the logic here has the luxury of assuming that the
  // *single* non-scalar axis is the *first* (fastest) axis.

  if (nrrdKind3DMaskedSymMatrix == this->nrrd->axis[0].kind
    || nrrdKind3DSymMatrix == this->nrrd->axis[0].kind)
    {
    if (nrrdKind3DSymMatrix == this->nrrd->axis[0].kind)
      {
      // we pad on a constant value 1 mask, then
      Nrrd *ntmp = nrrdNew();
      ptrdiff_t minIdx[4] = { -1, 0, 0, 0 };
      ptrdiff_t maxIdx[4] =
      {
        static_cast<ptrdiff_t>(this->nrrd->axis[0].size - 1),
        static_cast<ptrdiff_t>(this->nrrd->axis[1].size - 1),
        static_cast<ptrdiff_t>(this->nrrd->axis[2].size - 1),
        static_cast<ptrdiff_t>(this->nrrd->axis[3].size - 1)
      };
      if (nrrdCopy(ntmp, this->nrrd)
        || nrrdPad_nva(this->nrrd, ntmp, minIdx, maxIdx, nrrdBoundaryPad, 1.0))
        {
        char *err = biffGetDone(NRRD); // would be nice to free(err)
        vtkErrorMacro("Read: Error padding on conf mask in " << this->GetFileName() << ":\n" << err);
        return;
        }
      }

    vtkDebugMacro("Kind: Masked Sym Matrix");
    // Call tendExpand(nout,nin,scale,threshold)
    // Set up threshold to -1 to avoid this
    Nrrd *ntmp = nrrdNew();
    int errorCode = 0;
    const char *key = NRRD;
    errorCode |= nrrdCopy(ntmp, this->nrrd);
    if (!errorCode)
      {
      key = TEN;
      }
    if (!errorCode && AIR_EXISTS(ntmp->measurementFrame[0][0]))
      {
      // scan order-specific logic to tweak measurement frame goes here

      // Build a rotation matrix that brings us from this->RasToIjkMatrix
      vtkNew<vtkMatrix4x4> rasToIjkRotationMatrix;
      double col[3];
      for (int jjj = 0; jjj < 3; jjj++)
        {
        for (int iii = 0; iii < 3; iii++)
          {
          col[iii] = this->RasToIjkMatrix->GetElement(iii, jjj);
          }
        vtkMath::Normalize(col);
        for (int iii = 0; iii < 3; iii++)
          {
          rasToIjkRotationMatrix->SetElement(iii, jjj, col[iii]);
          }
        }

      double nrrdWorldToIjk[9] = { 0 };
      vtkNew<vtkMatrix4x4> nrrdWorldToIjkMatrix;
      nrrdWorldToIjkMatrix->Multiply4x4(rasToIjkRotationMatrix.GetPointer(), this->NRRDWorldToRasMatrix, nrrdWorldToIjkMatrix.GetPointer());
      for (int iii = 0; iii < 3; iii++)
        {
        for (int jjj = 0; jjj < 3; jjj++)
          {
          nrrdWorldToIjk[iii * 3 + jjj] = nrrdWorldToIjkMatrix->GetElement(iii, jjj);
          }
        }

      errorCode |= tenMeasurementFrameReduce(ntmp, ntmp);
      errorCode |= this->tenSpaceDirectionReduce(ntmp, ntmp, nrrdWorldToIjk);
      // Tensor has been reduced-> Tensor components are described in VTK Space. The measurement frame has to
      // be updated to reflect this change:
      rasToIjkRotationMatrix->Invert();

      this->MeasurementFrameMatrix->DeepCopy(rasToIjkRotationMatrix.GetPointer());
      }
    if (!errorCode)
      {
      errorCode |= tenExpand(this->nrrd, ntmp, 1, -1);
      }
    if (errorCode)
      {
      char *err = biffGetDone(key); // would be nice to free(err)
      vtkErrorMacro("Read: Error copying, crapping or cropping:\n" << err);
      return;
      }
    nrrdNuke(ntmp);
    }
  else if (nrrdKind3DMatrix == this->nrrd->axis[0].kind)
    {
    // alas, there is no Teem/ten function for reducing the
    // measurement frame of a non-symmetric matrix, but it would
    // be called here if it existed.
    }

  if (ptr)
    {
    memcpy(ptr, this->nrrd->data, nrrdElementSize(this->nrrd)*nrrdElementNumber(this->nrrd));
    }

  // release the memory while keeping the struct
  nrrdEmpty(this->nrrd);
}

//----------------------------------------------------------------------------
void vtkTeemNRRDReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
