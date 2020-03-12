/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

// XXX # Workaround bug in packaging of DCMTK 3.6.0 on Debian.
//     # See http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=637687
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif

#include "itkDCMTKFileReader.h"
#undef HAVE_SSTREAM   // 'twould be nice if people coded without using
// incredibly generic macro names
#include "dcmtk/config/osconfig.h" // make sure OS specific configuration is included first

#define INCLUDE_CSTDIO
#define INCLUDE_CSTRING

#include <dcmtk/dcmdata/dcuid.h>
#include "dcmtk/dcmdata/dcdict.h"             // For DcmDataDictionary
#include "dcmtk/dcmdata/dcsequen.h"        /* for DcmSequenceOfItems */
#include "dcmtk/dcmdata/dcvrcs.h"          /* for DcmCodeString */
#include "dcmtk/dcmdata/dcvrfd.h"          /* for DcmFloatingPointDouble */
#include "dcmtk/dcmdata/dcvrfl.h"          /* for DcmFloatingPointDouble */
#include "dcmtk/dcmdata/dcvrus.h"          /* for DcmUnsignedShort */
#include "dcmtk/dcmdata/dcvris.h"          /* for DcmIntegerString */
#include "dcmtk/dcmdata/dcvrobow.h"        /* for DcmOtherByteOtherWord */
#include "dcmtk/dcmdata/dcvrui.h"          /* for DcmUniqueIdentifier */
#include "dcmtk/dcmdata/dcfilefo.h"        /* for DcmFileFormat */
#include "dcmtk/dcmdata/dcdeftag.h"        /* for DCM_NumberOfFrames */
#include "dcmtk/dcmdata/dcvrlo.h"          /* for DcmLongString */
#include "dcmtk/dcmdata/dcvrtm.h"          /* for DCMTime */
#include "dcmtk/dcmdata/dcvrda.h"          /* for DcmDate */
#include "dcmtk/dcmdata/dcvrpn.h"          /* for DcmPersonName */
// #include "diregist.h"     /* include to support color images */
#include "vnl/vnl_cross.h"


namespace itk
{

void
DCMTKSequence
::SetDcmSequenceOfItems(DcmSequenceOfItems *seq)
{
  this->m_DcmSequenceOfItems = seq;
}
int
DCMTKSequence
::card()
{
  return this->m_DcmSequenceOfItems->card();
}

int
DCMTKSequence
::GetSequence(unsigned long index,
              DCMTKSequence &target,
              bool throwException)
{
  DcmItem *item = this->m_DcmSequenceOfItems->getItem(index);
  DcmSequenceOfItems *sequence =
    dynamic_cast<DcmSequenceOfItems *>(item);
  if(sequence == nullptr)
    {
    DCMTKException(<< "Can't find DCMTKSequence at index " << index);
    }
  target.SetDcmSequenceOfItems(sequence);
  return EXIT_SUCCESS;
}
int
DCMTKSequence
::GetStack(unsigned short group,
             unsigned short element,
             DcmStack *resultStack, bool throwException)
{
  DcmTagKey tagkey(group,element);
  if(this->m_DcmSequenceOfItems->search(tagkey,*resultStack) != EC_Normal)
    {
    DCMTKException(<< "Can't find tag " << std::hex << group << " "
                   << element << std::dec);
    }
  return EXIT_SUCCESS;
}

int
DCMTKSequence
::GetElementCS(unsigned short group,
               unsigned short element,
               std::string &target,
               bool throwException)
{
  DcmStack resultStack;
  this->GetStack(group,element,&resultStack);
  DcmCodeString *codeStringElement = dynamic_cast<DcmCodeString *>(resultStack.top());
  if(codeStringElement == nullptr)
    {
    DCMTKException(<< "Can't get CodeString Element at tag "
                   << std::hex << group << " "
                   << element << std::dec);
    }
  OFString ofString;
  if(codeStringElement->getOFStringArray(ofString) != EC_Normal)
    {
    DCMTKException(<< "Can't get OFString Value at tag "
                   << std::hex << group << " "
                   << element << std::dec);
    }
  target = "";
  for(unsigned j = 0; j < ofString.length(); ++j)
    {
    target += ofString[j];
    }
  return EXIT_SUCCESS;
}

int
DCMTKSequence::
GetElementFD(unsigned short group,
             unsigned short element,
             double * &target,
             bool throwException)
{
  DcmStack resultStack;
  this->GetStack(group,element,&resultStack);
  DcmFloatingPointDouble *fdItem = dynamic_cast<DcmFloatingPointDouble *>(resultStack.top());
  if(fdItem == nullptr)
    {
    DCMTKException(<< "Can't get CodeString Element at tag "
                   << std::hex << group << " "
                   << element << std::dec);
    }
  if(fdItem->getFloat64Array(target) != EC_Normal)
    {
    DCMTKException(<< "Can't get floatarray Value at tag "
                   << std::hex << group << " "
                   << element << std::dec);
    }
  return EXIT_SUCCESS;
}

int
DCMTKSequence
::GetElementFD(unsigned short group,
                   unsigned short element,
                   double &target,
               bool throwException)
{
  double *array;
  this->GetElementFD(group,element,array,throwException);
  target = array[0];
  return EXIT_SUCCESS;
}

int
DCMTKSequence
::GetElementDS(unsigned short group,
                   unsigned short element,
                   std::string &target,
               bool throwException)
{
  DcmStack resultStack;
  this->GetStack(group,element,&resultStack);
  DcmDecimalString *decimalStringElement = dynamic_cast<DcmDecimalString *>(resultStack.top());
  if(decimalStringElement == nullptr)
    {
    DCMTKException(<< "Can't get DecimalString Element at tag "
                   << std::hex << group << " "
                   << element << std::dec);
    }
  OFString ofString;
  if(decimalStringElement->getOFStringArray(ofString) != EC_Normal)
    {
    DCMTKException(<< "Can't get DecimalString Value at tag "
                   << std::hex << group << " "
                   << element << std::dec);
    }
  target = "";
  for(unsigned j = 0; j < ofString.length(); ++j)
    {
    target += ofString[j];
    }
  return EXIT_SUCCESS;
}

int
DCMTKSequence
::GetElementSQ(unsigned short group,
                   unsigned short element,
                   DCMTKSequence &target,
               bool throwException)
{
  DcmTagKey tagkey(group,element);
  DcmStack resultStack;
  this->GetStack(group,element,&resultStack);

  DcmSequenceOfItems *seqElement = dynamic_cast<DcmSequenceOfItems *>(resultStack.top());
  if(seqElement == nullptr)
    {
    DCMTKException(<< "Can't get  at tag "
                   << std::hex << group << " "
                   << element << std::dec);
    }
  target.SetDcmSequenceOfItems(seqElement);
  return EXIT_SUCCESS;
}

int
DCMTKSequence
::GetElementTM(unsigned short group,
             unsigned short element,
             std::string &target,
             bool throwException)
{
  DcmTagKey tagkey(group,element);
  DcmStack resultStack;
  this->GetStack(group,element,&resultStack);

  DcmTime *dcmTime = dynamic_cast<DcmTime *>(resultStack.top());
  if(dcmTime == nullptr)
    {
    DCMTKException(<< "Can't get  at tag "
                   << std::hex << group << " "
                   << element << std::dec);
    }
  char *cs;
  dcmTime->getString(cs);
  target = cs;
  return EXIT_SUCCESS;
}

DCMTKFileReader
::~DCMTKFileReader()
{

  delete m_DFile;
}

void
DCMTKFileReader
::SetFileName(const std::string &fileName)
{
  this->m_FileName = fileName;
}

const std::string &
DCMTKFileReader
::GetFileName() const
{
  return this->m_FileName;
}

void
DCMTKFileReader
::LoadFile()
{
  if(this->m_FileName == "")
    {
    itkGenericExceptionMacro(<< "No filename given" );
    }
  if(this->m_DFile != nullptr)
    {
    delete this->m_DFile;
    }
  this->m_DFile = new DcmFileFormat();
  OFCondition cond = this->m_DFile->loadFile(this->m_FileName.c_str());
                                             // /* transfer syntax, autodetect */
                                             // EXS_Unknown,
                                             // /* group length */
                                             // EGL_noChange,
                                             // /* Max read length */
                                             // 1024, // should be big
                                             //       // enough for
                                             //       // header stuff but
                                             //       // prevent reading
                                             //       // image data.
                                             // /* file read mode */
                                             // ERM_fileOnly);
  if(cond != EC_Normal)
    {
    itkGenericExceptionMacro(<< cond.text() << ": reading file " << this->m_FileName);
    }
  this->m_Dataset = this->m_DFile->getDataset();
  this->m_Xfer = this->m_Dataset->getOriginalXfer();
  if(this->m_Dataset->findAndGetSint32(DCM_NumberOfFrames,this->m_FrameCount).bad())
    {
    this->m_FrameCount = 1;
    }
  int fnum;
  this->GetElementIS(0x0020,0x0013,fnum);
  this->m_FileNumber = fnum;
}

int
DCMTKFileReader
::GetElementLO(unsigned short group,
                 unsigned short element,
                 std::string &target,
                 bool throwException)
{
  DcmTagKey tagkey(group,element);
  DcmElement *el;
  if(this->m_Dataset->findAndGetElement(tagkey,el) != EC_Normal)
    {
    DCMTKException(<< "Can't find tag " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  DcmLongString *loItem = dynamic_cast<DcmLongString *>(el);
  if(loItem == nullptr)
    {
    DCMTKException(<< "Can't find DecimalString element " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  OFString ofString;
  if(loItem->getOFStringArray(ofString) != EC_Normal)
    {
    DCMTKException(<< "Can't get string from element " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  target = "";
  for(unsigned i = 0; i < ofString.size(); i++)
    {
    target += ofString[i];
    }
  return EXIT_SUCCESS;
}

int
DCMTKFileReader
::GetElementLO(unsigned short group,
                 unsigned short element,
                 std::vector<std::string> &target,
                 bool throwException)
{
  DcmTagKey tagkey(group,element);
  DcmElement *el;
  if(this->m_Dataset->findAndGetElement(tagkey,el) != EC_Normal)
    {
    DCMTKException(<< "Can't find tag " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  DcmLongString *loItem = dynamic_cast<DcmLongString *>(el);
  if(loItem == nullptr)
    {
    DCMTKException(<< "Can't find DecimalString element " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  target.clear();
  OFString ofString;
  for(unsigned long i = 0; loItem->getOFString(ofString,i) == EC_Normal; ++i)
    {
    std::string targetStr = "";
    for(unsigned j = 0; j < ofString.size(); j++)
      {
      targetStr += ofString[j];
      }
    target.push_back(targetStr);
    }
  return EXIT_SUCCESS;
}

/** Get a DecimalString Item as a single string
 */
int
DCMTKFileReader
::GetElementDS(unsigned short group,
                  unsigned short element,
                  std::string &target,
                  bool throwException)
{
  DcmTagKey tagkey(group,element);
  DcmElement *el;
  if(this->m_Dataset->findAndGetElement(tagkey,el) != EC_Normal)
    {
    DCMTKException(<< "Can't find tag " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  DcmDecimalString *dsItem = dynamic_cast<DcmDecimalString *>(el);
  if(dsItem == nullptr)
    {
    DCMTKException(<< "Can't find DecimalString element " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  OFString ofString;
  if(dsItem->getOFStringArray(ofString) != EC_Normal)
    {
    DCMTKException(<< "Can't get DecimalString Value at tag "
                   << std::hex << group << " "
                   << element << std::dec);
    }
  target = "";
  for(unsigned j = 0; j < ofString.length(); ++j)
    {
    target += ofString[j];
    }
  return EXIT_SUCCESS;
}

int
DCMTKFileReader
::GetElementFD(unsigned short group,
                  unsigned short element,
                  double &target,
                  bool throwException)
{
  DcmTagKey tagkey(group,element);
  DcmElement *el;
  if(this->m_Dataset->findAndGetElement(tagkey,el) != EC_Normal)
    {
    DCMTKException(<< "Can't find tag " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  DcmFloatingPointDouble *fdItem = dynamic_cast<DcmFloatingPointDouble *>(el);
  if(fdItem == nullptr)
    {
    DCMTKException(<< "Can't find DecimalString element " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  if(fdItem->getFloat64(target) != EC_Normal)
    {
    DCMTKException(<< "Can't extract Array from DecimalString " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  return EXIT_SUCCESS;
}
int
DCMTKFileReader
::GetElementFD(unsigned short group,
                  unsigned short element,
                  double * &target,
                  bool throwException)
{
  DcmTagKey tagkey(group,element);
  DcmElement *el;
  if(this->m_Dataset->findAndGetElement(tagkey,el) != EC_Normal)
    {
    DCMTKException(<< "Can't find tag " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  DcmFloatingPointDouble *fdItem = dynamic_cast<DcmFloatingPointDouble *>(el);
  if(fdItem == nullptr)
    {
    DCMTKException(<< "Can't find DecimalString element " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  if(fdItem->getFloat64Array(target) != EC_Normal)
    {
    DCMTKException(<< "Can't extract Array from DecimalString " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  return EXIT_SUCCESS;
}
int
DCMTKFileReader
::GetElementFL(unsigned short group,
                  unsigned short element,
                  float &target,
                  bool throwException)
{
  DcmTagKey tagkey(group,element);
  DcmElement *el;
  if(this->m_Dataset->findAndGetElement(tagkey,el) != EC_Normal)
    {
    DCMTKException(<< "Can't find tag " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  DcmFloatingPointSingle *flItem = dynamic_cast<DcmFloatingPointSingle *>(el);
  if(flItem == nullptr)
    {
    DCMTKException(<< "Can't find DecimalString element " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  if(flItem->getFloat32(target) != EC_Normal)
    {
    DCMTKException(<< "Can't extract Array from DecimalString " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  return EXIT_SUCCESS;
}
int
DCMTKFileReader
::GetElementFLorOB(unsigned short group,
                      unsigned short element,
                      float &target,
                      bool throwException)
{
  if(this->GetElementFL(group,element,target,false) == EXIT_SUCCESS)
    {
    return EXIT_SUCCESS;
    }
  std::string val;
  if(this->GetElementOB(group,element,val) != EXIT_SUCCESS)
    {
    DCMTKException(<< "Can't find DecimalString element " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  const char *data = val.c_str();
  const float *fptr = reinterpret_cast<const float *>(data);
  target = *fptr;
  switch(this->GetTransferSyntax())
    {
    case EXS_LittleEndianImplicit:
    case EXS_LittleEndianExplicit:
      itk::ByteSwapper<float>::SwapFromSystemToLittleEndian(&target);
      break;
    case EXS_BigEndianImplicit:
    case EXS_BigEndianExplicit:
      itk::ByteSwapper<float>::SwapFromSystemToBigEndian(&target);
      break;
    default:
      break;
    }
  return EXIT_SUCCESS;
}

int
DCMTKFileReader
::GetElementUS(unsigned short group,
                  unsigned short element,
                  unsigned short &target,
                  bool throwException)
{
  DcmTagKey tagkey(group,element);
  DcmElement *el;
  if(this->m_Dataset->findAndGetElement(tagkey,el) != EC_Normal)
    {
    DCMTKException(<< "Can't find tag " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  DcmUnsignedShort *usItem = dynamic_cast<DcmUnsignedShort *>(el);
  if(usItem == nullptr)
    {
    DCMTKException(<< "Can't find DecimalString element " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  if(usItem->getUint16(target) != EC_Normal)
    {
    DCMTKException(<< "Can't extract Array from DecimalString " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  return EXIT_SUCCESS;
}
int
DCMTKFileReader
::GetElementUS(unsigned short group,
                  unsigned short element,
                  unsigned short *&target,
                  bool throwException)
{
  DcmTagKey tagkey(group,element);
  DcmElement *el;
  if(this->m_Dataset->findAndGetElement(tagkey,el) != EC_Normal)
    {
    DCMTKException(<< "Can't find tag " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  DcmUnsignedShort *usItem = dynamic_cast<DcmUnsignedShort *>(el);
  if(usItem == nullptr)
    {
    DCMTKException(<< "Can't find DecimalString element " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  if(usItem->getUint16Array(target) != EC_Normal)
    {
    DCMTKException(<< "Can't extract Array from DecimalString " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  return EXIT_SUCCESS;
}
/** Get a DecimalString Item as a single string
   */
int
DCMTKFileReader
::GetElementCS(unsigned short group,
                  unsigned short element,
                  std::string &target,
                  bool throwException)
{
  DcmTagKey tagkey(group,element);
  DcmElement *el;
  if(this->m_Dataset->findAndGetElement(tagkey,el) != EC_Normal)
    {
    DCMTKException(<< "Can't find tag " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  DcmCodeString *csItem = dynamic_cast<DcmCodeString *>(el);
  if(csItem == nullptr)
    {
    DCMTKException(<< "Can't find DecimalString element " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  OFString ofString;
  if(csItem->getOFStringArray(ofString) != EC_Normal)
    {
    DCMTKException(<< "Can't get DecimalString Value at tag "
                   << std::hex << group << " "
                   << element << std::dec);
    }
  target = "";
  for(unsigned j = 0; j < ofString.length(); ++j)
    {
    target += ofString[j];
    }
  return EXIT_SUCCESS;
}

int
DCMTKFileReader
::GetElementPN(unsigned short group,
                  unsigned short element,
                  std::string &target,
                  bool throwException)
{
  DcmTagKey tagkey(group,element);
  DcmElement *el;
  if(this->m_Dataset->findAndGetElement(tagkey,el) != EC_Normal)
    {
    DCMTKException(<< "Can't find tag " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  DcmPersonName *pnItem = dynamic_cast<DcmPersonName *>(el);
  if(pnItem == nullptr)
    {
    DCMTKException(<< "Can't find DecimalString element " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  OFString ofString;
  if(pnItem->getOFStringArray(ofString) != EC_Normal)
    {
    DCMTKException(<< "Can't get DecimalString Value at tag "
                   << std::hex << group << " "
                   << element << std::dec);
    }
  target = "";
  for(unsigned j = 0; j < ofString.length(); ++j)
    {
    target += ofString[j];
    }
  return EXIT_SUCCESS;
}

/** get an IS (Integer String Item
   */
int
DCMTKFileReader
::GetElementIS(unsigned short group,
                  unsigned short element,
                  ::itk::int32_t  &target,
                  bool throwException)
{
  DcmTagKey tagkey(group,element);
  DcmElement *el;
  if(this->m_Dataset->findAndGetElement(tagkey,el) != EC_Normal)
    {
    DCMTKException(<< "Can't find tag " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  DcmIntegerString *isItem = dynamic_cast<DcmIntegerString *>(el);
  if(isItem == nullptr)
    {
    DCMTKException(<< "Can't find DecimalString element " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  Sint32 _target; // MSVC seems to have type conversion problems with
                  // using int32_t as a an argument to getSint32
  if(isItem->getSint32(_target) != EC_Normal)
    {
    DCMTKException(<< "Can't get DecimalString Value at tag "
                   << std::hex << group << " "
                   << element << std::dec);
    }
  target = static_cast< ::itk::int32_t>(_target);
  return EXIT_SUCCESS;
}

int
DCMTKFileReader
::GetElementISorOB(unsigned short group,
                      unsigned short element,
                      ::itk::int32_t  &target,
                      bool throwException)
{
  if(this->GetElementIS(group,element,target,false) == EXIT_SUCCESS)
    {
    return EXIT_SUCCESS;
    }
  std::string val;
  if(this->GetElementOB(group,element,val,throwException) != EXIT_SUCCESS)
    {
    return EXIT_FAILURE;
    }
  const char *data = val.c_str();
  const int *iptr = reinterpret_cast<const int *>(data);
  target = *iptr;
  switch(this->GetTransferSyntax())
    {
    case EXS_LittleEndianImplicit:
    case EXS_LittleEndianExplicit:
      itk::ByteSwapper<int>::SwapFromSystemToLittleEndian(&target);
      break;
    case EXS_BigEndianImplicit:
    case EXS_BigEndianExplicit:
      itk::ByteSwapper<int>::SwapFromSystemToBigEndian(&target);
      break;
    default:                // no idea what to do
      break;
    }

  return EXIT_SUCCESS;
}

/** get an OB OtherByte Item
   */
int
DCMTKFileReader
::GetElementOB(unsigned short group,
                  unsigned short element,
                  std::string &target,
                  bool throwException)
{
  DcmTagKey tagkey(group,element);
  DcmElement *el;
  if(this->m_Dataset->findAndGetElement(tagkey,el) != EC_Normal)
    {
    DCMTKException(<< "Can't find tag " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  DcmOtherByteOtherWord *obItem = dynamic_cast<DcmOtherByteOtherWord *>(el);
  if(obItem == nullptr)
    {
    DCMTKException(<< "Can't find DecimalString element " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  OFString ofString;
  if(obItem->getOFStringArray(ofString) != EC_Normal)
    {
    DCMTKException(<< "Can't get OFString Value at tag "
                   << std::hex << group << " "
                   << element << std::dec);
    }
  target = Self::ConvertFromOB(ofString);
  return EXIT_SUCCESS;
}

int
DCMTKFileReader
::GetElementSQ(unsigned short group,
                 unsigned short entry,
                 DCMTKSequence &sequence,
                 bool throwException)
{
  DcmSequenceOfItems *seq;
  DcmTagKey tagKey(group,entry);

  if(this->m_Dataset->findAndGetSequence(tagKey,seq) != EC_Normal)
    {
    DCMTKException(<< "Can't find sequence "
                   << std::hex << group << " "
                   << std::hex << entry)
      }
  sequence.SetDcmSequenceOfItems(seq);
  return EXIT_SUCCESS;
}

int
DCMTKFileReader
::GetElementUI(unsigned short group,
                 unsigned short entry,
                 std::string &target,
                 bool throwException)
{
  DcmTagKey tagKey(group,entry);
  DcmElement *el;
  if(this->m_Dataset->findAndGetElement(tagKey,el) != EC_Normal)
    {
    DCMTKException(<< "Can't find tag " << std::hex
                   << group << " " << std::hex
                   << entry << std::dec);
    }
  DcmUniqueIdentifier *uiItem = dynamic_cast<DcmUniqueIdentifier *>(el);
  if(uiItem == nullptr)
    {
    DCMTKException(<< "Can't convert data item " << group
                   << "," << entry);
    }
  OFString ofString;
  if(uiItem->getOFStringArray(ofString,false) != EC_Normal)
    {
    DCMTKException(<< "Can't get UID Value at tag "
                   << std::hex << group << " " << std::hex
                   << entry << std::dec);
    }
  target = "";
  for(unsigned int j = 0; j < ofString.length(); ++j)
    {
    target.push_back(ofString[j]);
    }
  return EXIT_SUCCESS;
}

int DCMTKFileReader::
GetElementDA(unsigned short group,
             unsigned short element,
             std::string &target,
             bool throwException)
{
  DcmTagKey tagkey(group,element);
  DcmElement *el;
  if(this->m_Dataset->findAndGetElement(tagkey,el) != EC_Normal)
    {
    DCMTKException(<< "Can't find tag " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  DcmDate *dcmDate = dynamic_cast<DcmDate *>(el);
  if(dcmDate == nullptr)
    {
    DCMTKException(<< "Can't get  at tag "
                   << std::hex << group << " "
                   << element << std::dec);
    }
  char *cs;
  dcmDate->getString(cs);
  target = cs;
  return EXIT_SUCCESS;
}

int
DCMTKFileReader
::GetElementTM(unsigned short group,
             unsigned short element,
             std::string &target,
             bool throwException)
{

  DcmTagKey tagkey(group,element);
  DcmElement *el;
  if(this->m_Dataset->findAndGetElement(tagkey,el) != EC_Normal)
    {
    DCMTKException(<< "Can't find tag " << std::hex
                   << group << " " << std::hex
                   << element << std::dec);
    }
  DcmTime *dcmTime = dynamic_cast<DcmTime *>(el);
  if(dcmTime == nullptr)
    {
    DCMTKException(<< "Can't get  at tag "
                   << std::hex << group << " "
                   << element << std::dec);
    }
  char *cs;
  dcmTime->getString(cs);
  target = cs;
  return EXIT_SUCCESS;
}

int
DCMTKFileReader
::GetDirCosines(vnl_vector<double> &dir1,
                  vnl_vector<double> &dir2,
                  vnl_vector<double> &dir3)
{
  double dircos[6];
  DCMTKSequence planeSeq;
  if(this->GetElementDS(0x0020,0x0037,6,dircos,false) != EXIT_SUCCESS)
    {
    if(this->GetElementSQ(0x0020,0x9116,planeSeq,false) == EXIT_SUCCESS)
      {
      if(planeSeq.GetElementDS(0x0020,0x0037,6,dircos,false) != EXIT_SUCCESS)
        {
        return EXIT_FAILURE;
        }
      }
    }
  dir1[0] = dircos[0]; dir1[1] = dircos[1]; dir1[2] = dircos[2];
  dir2[0] = dircos[3]; dir2[1] = dircos[4]; dir2[2] = dircos[5];
  dir3 = vnl_cross_3d(dir1,dir2);
  return EXIT_SUCCESS;
}

int
DCMTKFileReader
::GetSlopeIntercept(double &slope, double &intercept)
{
  if(this->GetElementDS<double>(0x0028,1053,1,&slope,false) != EXIT_SUCCESS)
    {
    slope = 1.0;
    }
  if(this->GetElementDS<double>(0x0028,1052,1,&intercept,false) != EXIT_SUCCESS)
    {
    intercept = 0.0;
    }
  return EXIT_SUCCESS;
}

ImageIOBase::IOPixelType
DCMTKFileReader
::GetImagePixelType()
{
  unsigned short SamplesPerPixel;
  if(this->GetElementUS(0x0028,0x0100,SamplesPerPixel,false) != EXIT_SUCCESS)
    {
    return ImageIOBase::UNKNOWNPIXELTYPE;
    }
  ImageIOBase::IOPixelType pixelType;
  switch(SamplesPerPixel)
    {
    case 8:
    case 16:
      pixelType = ImageIOBase::SCALAR;
      break;
    case 24:
      pixelType = ImageIOBase::RGB;
      break;
    default:
      pixelType = ImageIOBase::VECTOR;
    }
  return pixelType;
}

ImageIOBase::IOComponentType
DCMTKFileReader
::GetImageDataType()
{
  unsigned short IsSigned;
  unsigned short BitsAllocated;
  unsigned short BitsStored;
  unsigned short HighBit;
  ImageIOBase::IOComponentType type =
    ImageIOBase::UNKNOWNCOMPONENTTYPE;

  if(this->GetElementUS(0x0028,0x0100,BitsAllocated,false) != EXIT_SUCCESS ||
     this->GetElementUS(0x0028,0x0101,BitsStored,false) != EXIT_SUCCESS ||
     this->GetElementUS(0x0028,0x0102,HighBit,false) != EXIT_SUCCESS ||
     this->GetElementUS(0x0028,0x0103,IsSigned,false) != EXIT_SUCCESS)
    {
    return type;
    }
  double slope, intercept;
  this->GetSlopeIntercept(slope,intercept);

  switch( BitsAllocated )
    {
    case 1:
    case 8:
    case 24: // RGB?
      if(IsSigned)
        {
        type = ImageIOBase::CHAR;
        }
      else
        {
        type = ImageIOBase::UCHAR;
        }
    break;
  case 12:
  case 16:
    if(IsSigned)
      {
      type = ImageIOBase::SHORT;
      }
    else
      {
      type = ImageIOBase::USHORT;
      }
    break;
  case 32:
  case 64: // Don't know what this actually means
    if(IsSigned)
      {
      type = ImageIOBase::LONG;
      }
    else
      {
      type = ImageIOBase::ULONG;
      }
    break;
  case 0:
  default:
    break;
    //assert(0);
    }

  return type;
}


int
DCMTKFileReader
::GetDimensions(unsigned short &rows, unsigned short &columns)
{
  if(this->GetElementUS(0x0028,0x0010,rows,false) != EXIT_SUCCESS ||
     this->GetElementUS(0x0028,0x0011,columns,false) != EXIT_SUCCESS)
    {
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

int
DCMTKFileReader
::GetSpacing(double *spacing)
{
  double _spacing[3];
  //
  // There are several tags that can have spacing, and we're going
  // from most to least desirable, starting with PixelSpacing, which
  // is guaranteed to be in patient space.
  // Imager Pixel spacing is inter-pixel spacing at the sensor front plane
  // Pixel spacing
  if((this->GetElementDS<double>(0x0028,0x0030,2,_spacing,false) != EXIT_SUCCESS &&
      // imager pixel spacing
      this->GetElementDS<double>(0x0018, 0x1164, 2, &_spacing[0],false) != EXIT_SUCCESS &&
      // Nominal Scanned PixelSpacing
      this->GetElementDS<double>(0x0018, 0x2010, 2, &_spacing[0],false) != EXIT_SUCCESS) ||
     // slice thickness
     this->GetElementDS<double>(0x0018,0x0050,1,&_spacing[2],false) != EXIT_SUCCESS)
    {
    // that method failed, go looking for the spacing sequence
    DCMTKSequence spacingSequence;
    DCMTKSequence subSequence;
    DCMTKSequence subsubSequence;
    // first, shared function groups sequence, then
    // per-frame groups sequence
    if((this->GetElementSQ(0x5200,0x9229,spacingSequence,false) == EXIT_SUCCESS ||
        this->GetElementSQ(0X5200,0X9230,spacingSequence,false) == EXIT_SUCCESS) &&
       spacingSequence.GetSequence(0,subSequence,false) == EXIT_SUCCESS &&
       subSequence.GetElementSQ(0x0028,0x9110,subsubSequence,false) == EXIT_SUCCESS)
      {
      if(subsubSequence.GetElementDS<double>(0x0028,0x0030,2,_spacing,false) != EXIT_SUCCESS)
        {
        // Pixel Spacing
        _spacing[0] = _spacing[1] = 1.0;
        }
      if(subsubSequence.GetElementDS<double>(0x0018,0x0050,1,&_spacing[2],false) != EXIT_SUCCESS)
        {
        // Slice Thickness
        _spacing[2] = 1.0;
        }
      }
    else
      {
      // punt if no info found.
      _spacing[0] = _spacing[1] = _spacing[2] = 1.0;
      }
    }
  //
  // spacing is row spacing\column spacing
  // but a slice is width-first, i.e. columns increase fastest.
  //
  spacing[0] = _spacing[1];
  spacing[1] = _spacing[0];
  spacing[2] = _spacing[2];
  return EXIT_SUCCESS;
}

int
DCMTKFileReader
::GetOrigin(double *origin)
{
  DCMTKSequence originSequence;
  DCMTKSequence subSequence;
  DCMTKSequence subsubSequence;

  if((this->GetElementSQ(0x5200,0x9229,originSequence,false) == EXIT_SUCCESS ||
      this->GetElementSQ(0X5200,0X9239,originSequence,false) == EXIT_SUCCESS) &&
     originSequence.GetSequence(0,subSequence,false) == EXIT_SUCCESS &&
     subSequence.GetElementSQ(0x0028,0x9113,subsubSequence,false) == EXIT_SUCCESS)
    {
    subsubSequence.GetElementDS<double>(0x0020,0x0032,3,origin,true);
    return EXIT_SUCCESS;
    }
  this->GetElementDS<double>(0x0020,0x0032,3,origin,true);
  return EXIT_SUCCESS;
}

int
DCMTKFileReader
::GetFrameCount() const
{
  return this->m_FrameCount;
}

E_TransferSyntax
DCMTKFileReader
::GetTransferSyntax() const
{
  return m_Xfer;
}

long
DCMTKFileReader
::GetFileNumber() const
{
  return m_FileNumber;
}

void
DCMTKFileReader
::AddDictEntry(DcmDictEntry *entry)
{
  DcmDataDictionary &dict = dcmDataDict.wrlock();
  dict.addEntry(entry);
#if OFFIS_DCMTK_VERSION_NUMBER < 364
  dcmDataDict.unlock();
#else
  dcmDataDict.rdunlock();
#endif
}

unsigned
DCMTKFileReader
::ascii2hex(char c)
{
  switch(c)
    {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a':
    case 'A': return 10;
    case 'b':
    case 'B': return 11;
    case 'c':
    case 'C': return 12;
    case 'd':
    case 'D': return 13;
    case 'e':
    case 'E': return 14;
    case 'f':
    case 'F': return 15;
    }
  return 255; // should never happen
}

std::string
DCMTKFileReader
::ConvertFromOB(OFString &toConvert)
{
  // string format is nn\nn\nn...
  std::string rval;
  for(size_t pos = 0; pos < toConvert.size(); pos += 3)
    {
    unsigned char convert[2];
    convert[0] = Self::ascii2hex(toConvert[pos]);
    convert[1] = Self::ascii2hex(toConvert[pos+1]);
    unsigned char conv = convert[0] << 4;
    conv += convert[1];
    rval.push_back(static_cast<unsigned char>(conv));
    }
  return rval;
}

bool CompareDCMTKFileReaders(DCMTKFileReader *a, DCMTKFileReader *b)
{
  return a->GetFileNumber() < b->GetFileNumber();
}

}
