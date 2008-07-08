/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkNRRDReader.cxx,v $
  Date:      $Date: 2007/06/12 19:13:58 $
  Version:   $Revision: 1.7.2.1 $

=========================================================================auto=*/
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkNRRDReader.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkNRRDReader.h"

#include <string>

#include "vtkByteSwap.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"


#include "vtkDataArray.h"
#include "vtkBitArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkCharArray.h"
#include "vtkUnsignedShortArray.h"
#include "vtkShortArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkIntArray.h"
#include "vtkUnsignedLongArray.h"
#include "vtkLongArray.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h" 

#include "teem/ten.h"

vtkCxxRevisionMacro(vtkNRRDReader, "$Revision: 1.7.2.1 $");
vtkStandardNewMacro(vtkNRRDReader);

vtkNRRDReader::vtkNRRDReader() 
{
  RasToIjkMatrix = NULL;
  NRRDWorldToRasMatrix = NULL;
  MeasurementFrameMatrix = NULL;
  HeaderKeys = NULL;
  CurrentFileName = NULL;
  nrrd = nrrdNew();
  UseNativeOrigin = false;
  ReadStatus = 0;
}

vtkNRRDReader::~vtkNRRDReader() 
{
  if (RasToIjkMatrix) {
    RasToIjkMatrix->Delete();
    RasToIjkMatrix = NULL;
  }
  
  if (MeasurementFrameMatrix) {
    MeasurementFrameMatrix->Delete();
    MeasurementFrameMatrix = NULL;
  }

  if (NRRDWorldToRasMatrix) {
    NRRDWorldToRasMatrix->Delete();
    NRRDWorldToRasMatrix = NULL;
  }
  
  if (HeaderKeys) {
    delete [] HeaderKeys;
    HeaderKeys = NULL;
  }
  if (CurrentFileName) {
    delete [] CurrentFileName;
    CurrentFileName = NULL;
  }
  nrrdNuke(this->nrrd);
}

vtkMatrix4x4* vtkNRRDReader::GetRasToIjkMatrix()
{
  this->ExecuteInformation();
  return this->RasToIjkMatrix;
}

vtkMatrix4x4* vtkNRRDReader::GetMeasurementFrameMatrix()
{
  this->ExecuteInformation();
  return MeasurementFrameMatrix;
}
  
char* vtkNRRDReader::GetHeaderKeys()
{
  std::string keys;
  for (std::map<std::string,std::string>::iterator i = HeaderKeyValue.begin();
       i != HeaderKeyValue.end(); i++) {
    std::string s = static_cast<std::string> (i->first);
    if (i != HeaderKeyValue.begin()){
      keys = keys + " ";
    }
    keys = keys + s;
  }
  if (HeaderKeys) {
    delete [] HeaderKeys;
  }
  HeaderKeys = NULL;

  if (keys.size() > 0) {
    HeaderKeys = new char[keys.size()+1];
    strcpy(HeaderKeys, keys.c_str());
  }
  return HeaderKeys;
}

char* vtkNRRDReader::GetHeaderValue(char *key)
{
  std::map<std::string,std::string>::iterator i = HeaderKeyValue.find(key);
  if (i != HeaderKeyValue.end()) {
    return (char *)(i->second.c_str());
  }
  else {
    return NULL;
  }
}

int vtkNRRDReader::CanReadFile(const char* filename)
{

  // Check the extension first to avoid opening files that do not
  // look like nrrds.  The file must have an appropriate extension to be
  // recognized.
  std::string fname = filename;
  if(  fname == "" )
    {
    vtkDebugMacro(<<"No filename specified.");
    return false;
    }

  // We'll assume we can read from stdin (don't try to read the header though)
  //if ( fname == "-" )
  //  {
  //  return true;
  //  }

  bool extensionFound = false;
  std::string::size_type nrrdPos = fname.rfind(".nrrd");
  if ((nrrdPos != std::string::npos)
      && (nrrdPos == fname.length() - 5))
    {
    extensionFound = true;
    }

  std::string::size_type nhdrPos = fname.rfind(".nhdr");
  if ((nhdrPos != std::string::npos)
      && (nhdrPos == fname.length() - 5))
    {
    extensionFound = true;
    }

  if( !extensionFound )
    {
    vtkDebugMacro(<<"The filename extension is not recognized");
    return false;
    }

  // We have the correct extension, so now check for the Nrrd magic.
  std::ifstream inputStream;

  inputStream.open( filename, std::ios::in | std::ios::binary );

  if( inputStream.fail() )
    {
    return false;
    }


  char magic[5] = {'\0','\0','\0','\0','\0'};
  inputStream.read(magic,4*sizeof(char));

  if( inputStream.eof() )
    {
    inputStream.close();
    return false;
    }

  if( strcmp(magic,"NRRD")==0 ) 
    {
    inputStream.close();
    return true;
    }

  inputStream.close();
  return false;
}




void vtkNRRDReader::ExecuteInformation()
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

   int i;
   char *key = NULL;
   char *val = NULL;
   char *err;
   NrrdIoState *nio;

   // save the Nrrd struct for the current file and 
   // don't re-execute the read unless the filename changes
   if ( this->CurrentFileName != NULL &&
            !strcmp (this->CurrentFileName, this->GetFileName()) )
   {
       // filename hasn't changed, don't re-execute
       return;
   }

   if ( this->CurrentFileName != NULL )
   {
        delete [] this->CurrentFileName;
   }

   this->CurrentFileName = new char[1 + strlen(this->GetFileName())];
   strcpy (this->CurrentFileName, this->GetFileName());

   nrrdNuke(this->nrrd); // nuke and reallocate to reset the state
   this->nrrd = nrrdNew();


   nio = nrrdIoStateNew();
   // this is the mechanism by which we tell nrrdLoad to read
   // just the header, and none of the data
   nrrdIoStateSet(nio, nrrdIoStateSkipData, 1);
   
   if (nrrdLoad(this->nrrd, this->GetFileName(), nio) != 0) {
     err = biffGetDone(NRRD);
     vtkErrorMacro("Error reading " << this->GetFileName() << ": " << err);
     free(err); // err points to malloc'd data!!
     //     err = NULL;
     nio = nrrdIoStateNix(nio);
     this->ReadStatus = 1;
     return;
   }


   HeaderKeyValue.clear();

   if (this->RasToIjkMatrix) {
     this->RasToIjkMatrix->Delete();
   }
   this->RasToIjkMatrix = vtkMatrix4x4::New();
   vtkMatrix4x4* IjkToRasMatrix = vtkMatrix4x4::New();

   this->RasToIjkMatrix->Identity();
   IjkToRasMatrix->Identity();

   if (MeasurementFrameMatrix) {
      MeasurementFrameMatrix->Delete();
   }
   MeasurementFrameMatrix = vtkMatrix4x4::New();
   MeasurementFrameMatrix->Identity();

   if (NRRDWorldToRasMatrix) {
      NRRDWorldToRasMatrix->Delete();
   }
   NRRDWorldToRasMatrix = vtkMatrix4x4::New();
   NRRDWorldToRasMatrix->Identity();

   if (nrrdTypeBlock == this->nrrd->type)
    {
     vtkErrorMacro("ReadImageInformation: Cannot currently "
                      "handle nrrdTypeBlock");
    nio = nrrdIoStateNix(nio);
    this->ReadStatus = 1;
    return;
    }


   if ( nio->endian == airEndianLittle ) {
     this->SetDataByteOrderToLittleEndian();
   }
   else {
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
   unsigned int domainAxisNum, domainAxisIdx[NRRD_DIM_MAX],
    rangeAxisNum, rangeAxisIdx[NRRD_DIM_MAX];
   domainAxisNum = nrrdDomainAxesGet(nrrd, domainAxisIdx);
   rangeAxisNum = nrrdRangeAxesGet(nrrd, rangeAxisIdx);
   if (this->nrrd->spaceDim && this->nrrd->spaceDim != domainAxisNum)
    {
    vtkErrorMacro("ReadImageInformation: nrrd's # independent axes ("
                      << domainAxisNum << ") doesn't match dimension of space"
                      " in which orientation is defined ("
                      << this->nrrd->spaceDim << "); not currently handled");
    nio = nrrdIoStateNix(nio);
    this->ReadStatus = 1;
    return;              
    }    
    

   // vtkNRRDReader only supports 3 or 4 dimensional image with scalar, vector,
   // normal or tensor data. Other dimensionality is considered a multicomponent scalar field. 

   if ( 3 == this->nrrd->dim && 0 == rangeAxisNum) {
     // Assume that all dimensions are spatial and the last
     //dimension is some vtk object
     
      // we don't have any non-scalar data
      this->SetPointDataType(SCALARS);
      this->SetNumberOfComponents(1);
      
      
   } else if ( 4 == this->nrrd->dim && 1 == rangeAxisNum) {  
   
      unsigned int kind = nrrd->axis[rangeAxisIdx[0]].kind;
      unsigned int size = nrrd->axis[rangeAxisIdx[0]].size;
      // NOTE: it is the NRRD readers responsibility to make sure that
      // the size (# of components) associated with a specific kind is
      // matches the actual size of the axis.
      switch(kind) {
      case nrrdKindDomain:
      case nrrdKindSpace:
      case nrrdKindTime:
    vtkErrorMacro("ReadImageInformation: range axis kind ("
              << airEnumStr(nrrdKind, kind) << ") seems more "
              "like a domain axis than a range axis");
    break;
      case nrrdKindStub:
      case nrrdKindScalar:
      case nrrdKind3Color:
      case nrrdKindRGBColor:
      case nrrdKind4Color:
      case nrrdKindRGBAColor:
      case nrrdKindComplex:
      case nrrdKindHSVColor:
      case nrrdKindXYZColor:  // nod to Luis
      case nrrdKindQuaternion:
      case nrrdKind2DSymMatrix:
      case nrrdKind2DMaskedSymMatrix:
      case nrrdKind2DMatrix:
      case nrrdKind2DMaskedMatrix:
      case nrrdKind2Vector: 
      case nrrdKind4Vector:
      case nrrdKindList:
      case nrrdKindPoint:  // thanks to Luis
        this->SetPointDataType(SCALARS);
        this->SetNumberOfComponents(size);
        break;
      case nrrdKind3Vector:
      case nrrdKind3Gradient:
    this->SetPointDataType(VECTORS);
    this->SetNumberOfComponents(size);
    break;
      case nrrdKindVector:
      case nrrdKindCovariantVector:
    if (3 == size) {
      this->SetPointDataType(VECTORS);
      this->SetNumberOfComponents(3);
    }
    else {
      this->SetPointDataType(SCALARS);
      this->SetNumberOfComponents(size);
    }
    break;
      case nrrdKindNormal:
      case nrrdKind3Normal:
    if (3 == size) {
      this->SetPointDataType(NORMALS);
      this->SetNumberOfComponents(size);
    }
    else {
      this->SetPointDataType(SCALARS);
      this->SetNumberOfComponents(size);
    }
    break;
      case nrrdKind3DMaskedSymMatrix:
    // NOTE: we will crop out the mask in Read() below and expand
    // the 6 values into 9, so NumberOfComponents != size
    this->SetPointDataType(TENSORS);
    this->SetNumberOfComponents(9);
    break;
      case nrrdKind3DSymMatrix:
    // NOTE: another case where NumberOfComponents != size, this
    // time because we will expand the 6 values into 9
    this->SetPointDataType(TENSORS);
    this->SetNumberOfComponents(9);         
    break;
      case nrrdKind3DMatrix:
    // this time we really do have 9 values coming in
    this->SetPointDataType(TENSORS);
    this->SetNumberOfComponents(9);
    break;
      default:
    vtkErrorMacro("ReadImageInformation: nrrdKind " << kind 
              << " not handled!");
    return;
      }
   }      
   else {
     vtkErrorMacro("Error reading " << this->GetFileName() << ": " << "only 3D/4D volumes supported");
     return;
   }

   // Set type information
   this->SetDataType(this->NrrdToVTKScalarType(this->nrrd->type) );
   
   // Set axis information
   int dataExtent[6];
   vtkFloatingPointType spacings[3];
   double spacing;
   vtkFloatingPointType origin[3];

   double spaceDir[NRRD_SPACE_DIM_MAX];
   int spacingStatus;
   for (unsigned int axii=0; axii < domainAxisNum; axii++)
    {
    unsigned int naxi = domainAxisIdx[axii];
    dataExtent[2*axii] = 0;
    dataExtent[2*axii+1] = this->nrrd->axis[naxi].size - 1;  
    
    spacingStatus = nrrdSpacingCalculate(this->nrrd, naxi, &spacing, spaceDir);
    
    switch(spacingStatus) 
      {
      case nrrdSpacingStatusNone:
        spacings[axii]=1.0;
        break;
      case nrrdSpacingStatusScalarNoSpace:
        spacings[axii]=spacing;
        IjkToRasMatrix->SetElement(axii,axii,spacing);
        break;
      case nrrdSpacingStatusDirection:
        if (AIR_EXISTS(spacing))
          {
          // only set info if we have something to set
          //this->SetSpacing(axii, spacing);
          spacings[axii]=spacing;

          switch (this->nrrd->space)
            {
              // on read, convert non-RAS coords into RAS coords, when we can
            case nrrdSpaceRightAnteriorSuperior:
              // no change needed
              NRRDWorldToRasMatrix->Identity();
              break;
            case nrrdSpaceLeftAnteriorSuperior:
              spaceDir[0] *= -1;   // L -> R
          NRRDWorldToRasMatrix->SetElement(0,0,-1);
              break;
            case nrrdSpaceLeftPosteriorSuperior:
              spaceDir[0] *= -1;   // L -> R
              spaceDir[1] *= -1;   // P -> A
              NRRDWorldToRasMatrix->SetElement(0,0,-1);
              NRRDWorldToRasMatrix->SetElement(1,1,-1);
              break;
            default:
              // we're not coming from a space for which the conversion
              // to LPS is well-defined
              break;
            }

          for (int j=0; (unsigned int)j<this->nrrd->spaceDim; j++) 
            {
             IjkToRasMatrix->SetElement(j,axii , spaceDir[j]*spacing);
            }
          }
        break;
          default:
          case nrrdSpacingStatusUnknown:
            vtkErrorMacro("ReadImageInformation: Error interpreting "
                              "nrrd spacing (nrrdSpacingStatusUnknown)");
        break;
          case nrrdSpacingStatusScalarWithSpace:
            vtkErrorMacro("ReadImageInformation: Error interpreting "
                              "nrrd spacing (nrrdSpacingStatusScalarWithSpace)");
        break;
      }
    }

  // Figure out origin
  if (3 == this->nrrd->spaceDim)
    {
    if (AIR_EXISTS(this->nrrd->spaceOrigin[0]))
      {
      // only set info if we have something to set
      for (unsigned int saxi=0; saxi < this->nrrd->spaceDim; saxi++)
        {
        origin[saxi] = nrrd->spaceOrigin[saxi];
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
    int originStatus = nrrdOriginCalculate(nrrd, domainAxisIdx, domainAxisNum,
                                           nrrdCenterCell, spaceOrigin);
    for (unsigned int saxi=0; saxi < domainAxisNum; saxi++) 
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
          vtkErrorMacro("ReadImageInformation: Error interpreting "
                            "nrrd origin status");
          nio = nrrdIoStateNix(nio);
          this->ReadStatus = 1;
          break;
        }
      }
    }
      
  if (this->UseNativeOrigin && AIR_EXISTS(this->nrrd->spaceOrigin[0])) {
       for (int i=0; i < 3; i++) {
           IjkToRasMatrix->SetElement(i, 3, origin[i]);
       }
       vtkMatrix4x4::Invert(IjkToRasMatrix, this->RasToIjkMatrix);
   } else {
       vtkMatrix4x4::Invert(IjkToRasMatrix, this->RasToIjkMatrix);
       for (int i=0; i < 3; i++) {
           this->RasToIjkMatrix->SetElement(i, 3, (dataExtent[2*i+1] - dataExtent[2*i])/2.0);
       }
   }
  
   this->RasToIjkMatrix->SetElement(3,3,1.0);
   IjkToRasMatrix->Delete();

   this->SetDataSpacing(spacings);
   //this->SetDataOrigin(origin);
   this->SetDataExtent(dataExtent);

   // Push extra key/value pair data into std::map
   for (i=0; (unsigned int)i < nrrdKeyValueSize(this->nrrd); i++) {
     nrrdKeyValueIndex(this->nrrd, &key, &val, i);
     HeaderKeyValue[std::string(key)] = std::string(val);
     free(key);  // key and val point to malloc'd data!!
     free(val);
     key = val = NULL;
   }

   if (this->nrrd->space) 
     {
      HeaderKeyValue[std::string("space")] = std::string( airEnumStr(nrrdSpace, nrrd->space) );
     }


   if (AIR_EXISTS(this->nrrd->measurementFrame[0][0])) 
   {
    for (int i=0;i<3;i++)
      {

     switch (this->nrrd->space)
      {
          //WARNING: nrrd->measurementFrame[i][0:2] are the rows fo the measurementFrame matrix
          // on read, convert non-RAS coords into RAS coords, when we can
          case nrrdSpaceRightAnteriorSuperior:
            // no change needed
            MeasurementFrameMatrix->SetElement(0,i,this->nrrd->measurementFrame[i][0]);
            MeasurementFrameMatrix->SetElement(1,i,this->nrrd->measurementFrame[i][1]);
            MeasurementFrameMatrix->SetElement(2,i,this->nrrd->measurementFrame[i][2]);
          break;
          case nrrdSpaceLeftAnteriorSuperior:
           // L -> R
            MeasurementFrameMatrix->SetElement(0,i,-this->nrrd->measurementFrame[i][0]);
            MeasurementFrameMatrix->SetElement(1,i,this->nrrd->measurementFrame[i][1]);
            MeasurementFrameMatrix->SetElement(2,i,this->nrrd->measurementFrame[i][2]); 
            break;
          case nrrdSpaceLeftPosteriorSuperior:
           // L -> R
           // P -> A
            MeasurementFrameMatrix->SetElement(0,i,-this->nrrd->measurementFrame[i][0]);
            MeasurementFrameMatrix->SetElement(1,i,-this->nrrd->measurementFrame[i][1]);
            MeasurementFrameMatrix->SetElement(2,i,this->nrrd->measurementFrame[i][2]);
            break;
          default:
            // we're not coming from a space for which the conversion
            // to LPS is well-defined
            MeasurementFrameMatrix->SetElement(0,i,this->nrrd->measurementFrame[i][0]);
            MeasurementFrameMatrix->SetElement(1,i,this->nrrd->measurementFrame[i][1]);
            MeasurementFrameMatrix->SetElement(2,i,this->nrrd->measurementFrame[i][2]); 
            break;
      }
      }
   }

   this->vtkImageReader2::ExecuteInformation();
   nio = nrrdIoStateNix(nio);
}


vtkImageData *vtkNRRDReader::AllocateOutputData(vtkDataObject *out) {

 vtkImageData *res = vtkImageData::SafeDownCast(out);
  if (!res)
    {
    vtkWarningMacro("Call to AllocateOutputData with non vtkImageData output");
    return NULL;
    }

  // I would like to eliminate this method which requires extra "information"
  // That is not computed in the graphics pipeline.
  // Until I can eliminate the method, I will reexecute the ExecuteInformation
  // before the execute.
  this->ExecuteInformation();

  res->SetExtent(res->GetUpdateExtent());
  this->AllocatePointData(res);

  return res;

}

void vtkNRRDReader::AllocatePointData(vtkImageData *out) {

 vtkDataArray *pd = NULL;
 int Extent[6]; 
 out->GetExtent(Extent);

  // if the scalar type has not been set then we have a problem
  if (this->DataType == VTK_VOID)
    {
    vtkErrorMacro("Attempt to allocate scalars before scalar type was set!.");
    return;
    }

  // if we currently have scalars then just adjust the size
  switch (this->PointDataType) {
    case SCALARS:
       pd = out->GetPointData()->GetScalars();
       break;
    case VECTORS:
       pd = out->GetPointData()->GetVectors();
       break;
    case NORMALS:
       pd = out->GetPointData()->GetNormals();
       break;
    case TENSORS:
       pd = out->GetPointData()->GetTensors();
       break;
    default:
       vtkErrorMacro("Unknown PointData Type.");
       return;
   }             

  if (pd && pd->GetDataType() == this->DataType
      && pd->GetReferenceCount() == 1) 
    {
    pd->SetNumberOfComponents(this->GetNumberOfComponents());
    pd->SetNumberOfTuples((Extent[1] - Extent[0] + 1)*
                               (Extent[3] - Extent[2] + 1)*
                               (Extent[5] - Extent[4] + 1));
    // Since the execute method will be modifying the scalars
    // directly.
    pd->Modified();
    return;
    }
  
  // allocate the new scalars
  switch (this->DataType)
    {
    case VTK_BIT:
      pd = vtkBitArray::New();
      break;
    case VTK_UNSIGNED_CHAR:
      pd = vtkUnsignedCharArray::New();
      break;
    case VTK_CHAR:
      pd = vtkCharArray::New();
      break;
    case VTK_UNSIGNED_SHORT: 
      pd = vtkUnsignedShortArray::New();
      break;
    case VTK_SHORT:
      pd = vtkShortArray::New();
      break;
    case VTK_UNSIGNED_INT:
      pd = vtkUnsignedIntArray::New();
      break;
    case VTK_INT:
      pd = vtkIntArray::New();
      break;
    case VTK_UNSIGNED_LONG:
      pd = vtkUnsignedLongArray::New();
      break;
    case VTK_LONG:
      pd = vtkLongArray::New();
      break;
    case VTK_DOUBLE:
      pd = vtkDoubleArray::New();
      break;
    case VTK_FLOAT:
      pd = vtkFloatArray::New();
      break;
    default:
      vtkErrorMacro("Could not allocate data type.");
      return;
    }
  out->SetScalarType(this->DataType);
  pd->SetNumberOfComponents(this->GetNumberOfComponents());

  // allocate enough memors
  pd->SetNumberOfTuples((Extent[1] - Extent[0] + 1)*
                      (Extent[3] - Extent[2] + 1)*
                      (Extent[5] - Extent[4] + 1));

    switch (this->PointDataType) {
    case SCALARS:
       out->GetPointData()->SetScalars(pd);
       out->SetNumberOfScalarComponents(this->GetNumberOfComponents());
       break;
    case VECTORS:
       out->GetPointData()->SetVectors(pd);
       break;
    case NORMALS:
       out->GetPointData()->SetNormals(pd);
       break;
    case TENSORS:
       out->GetPointData()->SetTensors(pd);
       break;
    default:
       vtkErrorMacro("Unknown PointData Type.");
       return;
     }
  pd->Delete();
}

int
tenSpaceDirectionReduce(Nrrd *nout, const Nrrd *nin, double SD[9]) {
  char me[]="tenSpaceDirectionReduce", err[BIFF_STRLEN];
  double SDT[9], tenMeasr[9], tenSlice[9];
  float *tdata;
  size_t ii, nn;
  //unsigned int si, sj;
  //double det;
  
  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_TRUE, AIR_TRUE)) {
    sprintf(err, "%s: ", me);
    biffAdd(TEN, err); return 1;
  }
  if (3 != nin->spaceDim) {
    sprintf(err, "%s: input nrrd needs 3-D (not %u-D) space dimension",
            me, nin->spaceDim);
    biffAdd(TEN, err); return 1;
  }
  if (!ELL_3M_EXISTS(SD)) {
    sprintf(err, "%s: 3x3 space direction doesn't exist", me);
    biffAdd(TEN, err); return 1;
  }

  //ELL_3M_INV(SD, SDINV, det);
  ELL_3M_TRANSPOSE(SDT, SD);

  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: trouble with initial copy", me);
      biffAdd(TEN, err); return 1;
    }
  }
  nn = nrrdElementNumber(nout)/nout->axis[0].size;
  tdata = (float*)(nout->data);
  for (ii=0; ii<nn; ii++) {
    TEN_T2M(tenMeasr, tdata);
    ell_3m_mul_d(tenSlice, SD, tenMeasr);
    ell_3m_mul_d(tenSlice, tenSlice, SDT);
    TEN_M2T_TT(tdata, float, tenSlice);
    tdata += 7;
  }
  return 0;
}


//----------------------------------------------------------------------------
// This function reads a data from a file.  The datas extent/axes
// are assumed to be the same as the file extent/order.
void vtkNRRDReader::ExecuteData(vtkDataObject *output)
{

  output->SetUpdateExtentToWholeExtent();
  vtkImageData *data = this->AllocateOutputData(output);

  if (this->GetFileName() == NULL)
    {
    vtkErrorMacro(<< "Either a FileName or FilePrefix must be specified.");
    return;
    }


  // Read in the nrrd.  Yes, this means that the header is being read
  // twice: once by ExecuteInformation, and once here
  if ( nrrdLoad(this->nrrd, this->GetFileName(), NULL) != 0 )
    {
    char *err =  biffGetDone(NRRD); // would be nice to free(err)
    vtkErrorMacro("Read: Error reading " 
                      << this->GetFileName() << ":\n" << err);
     return;
    }


  if (this->nrrd->data == NULL)
    {
    vtkErrorMacro(<< "data is null.");
    return;
    }
  void *ptr = NULL;
  switch(PointDataType) {
    case SCALARS:
      data->GetPointData()->GetScalars()->SetName("NRRDImage");
      //get pointer
      ptr = data->GetPointData()->GetScalars()->GetVoidPointer(0);
      break;
    case VECTORS:
      data->GetPointData()->GetVectors()->SetName("NRRDImage");
      //get pointer
      ptr = data->GetPointData()->GetVectors()->GetVoidPointer(0);
      break;
    case NORMALS:
      data->GetPointData()->GetNormals()->SetName("NRRDImage");
      ptr = data->GetPointData()->GetNormals()->GetVoidPointer(0); 
      break;
    case TENSORS:
      data->GetPointData()->GetTensors()->SetName("NRRDImage");
      ptr = data->GetPointData()->GetTensors()->GetVoidPointer(0); 
      break;
   }
  this->ComputeDataIncrements();

  int dims[3];
  data->GetDimensions(dims);

  unsigned int rangeAxisNum, rangeAxisIdx[NRRD_DIM_MAX];
  rangeAxisNum = nrrdRangeAxesGet(this->nrrd, rangeAxisIdx);

  if ( rangeAxisNum > 1)
    {
    vtkErrorMacro("Read: handling more than one non-scalar axis "
                      "not currently handled");
    return;
    }
  if (1 == rangeAxisNum && 0 != rangeAxisIdx[0])
    {
    // the range (dependent variable) is not on the fastest axis,
    // so we have to permute axes to put it there, since that is
    // how we set things up in ReadImageInformation() above
    Nrrd *ntmp = nrrdNew();
    unsigned int axmap[NRRD_DIM_MAX];
    axmap[0] = rangeAxisIdx[0];
    for (unsigned int axi=1; axi<this->nrrd->dim; axi++)
      {
      axmap[axi] = axi - (axi <= rangeAxisIdx[0]);
      }
    // The memory size of the input and output of nrrdAxesPermute is
    // the same; the existing nrrd->data is re-used.
    if (nrrdCopy(ntmp, this->nrrd)
        || nrrdAxesPermute(this->nrrd, ntmp, axmap))
      {
      char *err =  biffGetDone(NRRD); // would be nice to free(err)
      vtkErrorMacro("Read: Error permuting independent axis in " 
                        << this->GetFileName() << ":\n" << err);
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
     || nrrdKind3DSymMatrix == this->nrrd->axis[0].kind) {

       if (nrrdKind3DSymMatrix == this->nrrd->axis[0].kind) {
         // we pad on a constant value 1 mask, then 
         ptrdiff_t minIdx[4], maxIdx[4];
         Nrrd *ntmp = nrrdNew();
         minIdx[0] = -1;
         minIdx[1] = minIdx[2] = minIdx[3] = 0;
         maxIdx[0] = static_cast<ptrdiff_t>(this->nrrd->axis[0].size - 1);
         maxIdx[1] = static_cast<ptrdiff_t>(this->nrrd->axis[1].size - 1);
         maxIdx[2] = static_cast<ptrdiff_t>(this->nrrd->axis[2].size - 1);
         maxIdx[3] = static_cast<ptrdiff_t>(this->nrrd->axis[3].size - 1);
         if (nrrdCopy(ntmp, this->nrrd)
             || nrrdPad_nva(this->nrrd, ntmp, minIdx, maxIdx,
                    nrrdBoundaryPad, 1.0)) {
           char *err =  biffGetDone(NRRD); // would be nice to free(err)
           vtkErrorMacro("Read: Error padding on conf mask in " 
                 << this->GetFileName() << ":\n" << err);
           return;
         }
       }

       const char *key;
       int E;
       cout<<"Kind: Masked Sym Matrix"<<endl;
       // Call tendExpand(nout,nin,scale,threshold)
       // Set up threshold to -1 to avoid this 
       Nrrd *ntmp = nrrdNew();
       E = 0;
       if (!E) key = NRRD;
       if (!E) E |= nrrdCopy(ntmp, this->nrrd);
       if (!E) key = TEN;
       if (!E && AIR_EXISTS(ntmp->measurementFrame[0][0])) {
         // scan order-specific logic to tweak measurement frame goes here
         
         double NRRDWorldToIjk[9];
         vtkMatrix4x4 *NRRDWorldToIjkMatrix = vtkMatrix4x4::New();
     vtkMatrix4x4 *RasToIjkRotationMatrix = vtkMatrix4x4::New();

         // Build a rotation matrix that brings us from RasToIjkMatrix
         int iii, jjj;
         double col[3];
         RasToIjkRotationMatrix->Identity();
         for (jjj = 0; jjj < 3; jjj++)
           {
            for (iii = 0; iii < 3; iii++)
              {
              col[iii]=this->RasToIjkMatrix->GetElement(iii,jjj);
              }
            vtkMath::Normalize(col);
            for (iii = 0; iii < 3; iii++)
              {
              RasToIjkRotationMatrix->SetElement(iii,jjj,col[iii]);
              }
         }

         NRRDWorldToIjkMatrix->Multiply4x4(RasToIjkRotationMatrix,this->NRRDWorldToRasMatrix,NRRDWorldToIjkMatrix);
        
         for (iii = 0; iii < 3; iii++) {
            for (jjj = 0; jjj < 3; jjj++) {
                NRRDWorldToIjk[iii*3+jjj] = NRRDWorldToIjkMatrix->GetElement(iii,jjj);
            }
         }

         E |= tenMeasurementFrameReduce(ntmp, ntmp);
         E |= tenSpaceDirectionReduce(ntmp, ntmp, NRRDWorldToIjk);
    // Tensor has been reduced-> Tensor components are described in VTK Space. The measurement frame has to
        // be updated to reflect this change: 
        RasToIjkRotationMatrix->Invert();

        this->MeasurementFrameMatrix->DeepCopy(RasToIjkRotationMatrix);
        NRRDWorldToIjkMatrix->Delete();
        RasToIjkRotationMatrix->Delete();
       }
       if (!E) E |= tenExpand(this->nrrd, ntmp, 1, -1);
       if (E) {
         char *err =  biffGetDone(key); // would be nice to free(err)
         vtkErrorMacro("Read: Error copying, crapping or cropping:\n"
                   << err);
         return;
       }
       nrrdNuke(ntmp);
     }
     else if (nrrdKind3DMatrix == this->nrrd->axis[0].kind) {
       // alas, there is no Teem/ten function for reducing the
       // measurement frame of a non-symmetric matrix, but it would
       // be called here if it existed.
     }
     memcpy(ptr, this->nrrd->data,
        nrrdElementSize(nrrd)*nrrdElementNumber(nrrd));

     // release the memory while keeping the struct
     nrrdEmpty(nrrd);
}


//----------------------------------------------------------------------------
void vtkNRRDReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

