/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "itkDCMTKFileReader.h"
#undef HAVE_SSTREAM // 'twould be nice if people coded without using
// incredibly generic macro names
#include "dcmtk/config/osconfig.h" // make sure OS specific configuration is included first
#define INCLUDE_CSTDIO
#define INCLUDE_CSTRING

#include "dcmtk/dcmdata/dcdict.h"    // For DcmDataDictionary
#include "dcmtk/dcmdata/dcuid.h"     /* Included for OFFIS_DCMTK_VERSION_NUMBER */
#include "dcmtk/dcmdata/dcsequen.h"  /* for DcmSequenceOfItems */
#include "dcmtk/dcmdata/dcvrcs.h"    /* for DcmCodeString */
#include "dcmtk/dcmdata/dcvrfd.h"    /* for DcmFloatingPointDouble */
#include "dcmtk/dcmdata/dcvrfl.h"    /* for DcmFloatingPointDouble */
#include "dcmtk/dcmdata/dcvrsl.h"    /* for DCMSignedLong */
#include "dcmtk/dcmdata/dcvrus.h"    /* for DcmUnsignedShort */
#include "dcmtk/dcmdata/dcvris.h"    /* for DcmIntegerString */
#include "dcmtk/dcmdata/dcvrobow.h"  /* for DcmOtherByteOtherWord */
#include "dcmtk/dcmdata/dcelem.h"    /* for DcmElement */
#include "dcmtk/dcmdata/dcvrui.h"    /* for DcmUniqueIdentifier */
#include "dcmtk/dcmdata/dcfilefo.h"  /* for DcmFileFormat */
#include "dcmtk/dcmdata/dcmetinf.h"  /* for DcmMetaInfo */
#include "dcmtk/dcmdata/dcdeftag.h"  /* for DCM_NumberOfFrames */
#include "dcmtk/dcmdata/dcvrlo.h"    /* for DcmLongString */
#include "dcmtk/dcmdata/dcvrtm.h"    /* for DCMTime */
#include "dcmtk/dcmdata/dcvrda.h"    /* for DcmDate */
#include "dcmtk/dcmdata/dcvrpn.h"    /* for DcmPersonName */
#include "dcmtk/dcmimgle/dcmimage.h" /* fore DicomImage */
#include "dcmtk/dcmimage/diregist.h" /* include to support color images */

#include "vnl/vnl_cross.h"
#include "itkIntTypes.h"
#include "itkMetaDataObject.h"
#include "itksys/Base64.h"
#include <algorithm>
#include <memory>

namespace itk
{

void DCMTKItem::SetDcmItem(DcmItem* item)
{
  this->m_DcmItem = item;
}
int DCMTKItem::GetElementSQ(const unsigned short group, const unsigned short entry, DCMTKSequence& sequence, const bool throwException) const
{
  DcmSequenceOfItems* seq;
  DcmTagKey tagKey(group, entry);
  //  this->m_DcmItem->print(std::cerr);
  if (this->m_DcmItem->findAndGetSequence(tagKey, seq) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't find sequence " << std::hex << group << ' ' << std::hex << entry)
  }
  sequence.SetDcmSequenceOfItems(seq);
  return EXIT_SUCCESS;
}

int DCMTKSequence::GetStack(const unsigned short group, const unsigned short element, DcmStack& resultStack, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  if (this->m_DcmSequenceOfItems->search(tagkey, resultStack) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't find tag " << std::hex << group << ' ' << element << std::dec);
  }
  return EXIT_SUCCESS;
}

void DCMTKSequence::SetDcmSequenceOfItems(DcmSequenceOfItems* seq)
{
  this->m_DcmSequenceOfItems = seq;
}

int DCMTKSequence::card() const
{
  return this->m_DcmSequenceOfItems->card();
}

int DCMTKSequence::GetSequence(unsigned long index, DCMTKSequence& target, const bool throwException) const
{
  DcmItem* item = this->m_DcmSequenceOfItems->getItem(index);
  auto* sequence = dynamic_cast<DcmSequenceOfItems*>(item);
  if (sequence == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't find DCMTKSequence at index " << index);
  }
  target.SetDcmSequenceOfItems(sequence);
  return EXIT_SUCCESS;
}

int DCMTKSequence::GetElementCS(const unsigned short group, const unsigned short element, std::string& target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmStack resultStack;
  this->GetStack(group, element, resultStack, throwException);
  auto* codeStringElement = dynamic_cast<DcmCodeString*>(resultStack.top());
  if (codeStringElement == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find CodeString Element " << std::hex << group << ' ' << std::hex << element << std::dec);
  }

  OFString ofString;
  if (codeStringElement->getOFStringArray(ofString) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't get OFString Value at tag " << std::hex << group << ' ' << element << std::dec);
  }
  target = "";
  for (auto j : ofString)
  {
    target += j;
  }
  return EXIT_SUCCESS;
}

int DCMTKSequence::GetElementOB(const unsigned short group, const unsigned short element, std::string& target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmStack resultStack;
  this->GetStack(group, element, resultStack, throwException);
  auto* obItem = dynamic_cast<DcmOtherByteOtherWord*>(resultStack.top());

  if (obItem == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  Uint8* bytes;
  obItem->getUint8Array(bytes);
  Uint32 length = obItem->getLength();
  std::string val;
  for (unsigned int i = 0; i < length; ++i)
  {
    val += bytes[i];
  }
  target = val;
  return EXIT_SUCCESS;
}

int DCMTKSequence::GetElementCSorOB(const unsigned short group, const unsigned short element, std::string& target, const bool throwException) const
{
  if (this->GetElementCS(group, element, target, false) == EXIT_SUCCESS)
  {
    return EXIT_SUCCESS;
  }
  if (this->GetElementOB(group, element, target, throwException) != EXIT_SUCCESS)
  {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int DCMTKSequence::GetElementFD(const unsigned short group, const unsigned short element, int count, double* target, const bool throwException) const
{
  DcmStack resultStack;
  this->GetStack(group, element, resultStack);
  auto* fdItem = dynamic_cast<DcmFloatingPointDouble*>(resultStack.top());
  if (fdItem == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't get CodeString Element at tag " << std::hex << group << ' ' << element << std::dec);
  }
  double* tmp;
  std::stringstream ss;
  ss << count;
  OFString vm(ss.str().c_str());
  if (fdItem->checkValue(vm).bad())
  {
    DCMTKExceptionOrErrorReturn(<< "Value multiplicity doesn't match " << count << " at  tag " << std::hex << group << ' ' << element << std::dec);
  }
  if (fdItem->getFloat64Array(tmp) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't get floatarray Value at tag " << std::hex << group << ' ' << element << std::dec);
  }
  for (int i = 0; i < count; ++i)
  {
    target[i] = tmp[i];
  }
  return EXIT_SUCCESS;
}

int DCMTKSequence::GetElementFD(const unsigned short group, const unsigned short element, double& target, const bool throwException) const
{
  this->GetElementFD(group, element, 1, &target, throwException);
  return EXIT_SUCCESS;
}

int DCMTKSequence::GetElementDS(const unsigned short group, const unsigned short element, std::string& target, const bool throwException) const
{
  DcmStack resultStack;
  this->GetStack(group, element, resultStack);
  auto* decimalStringElement = dynamic_cast<DcmDecimalString*>(resultStack.top());
  if (decimalStringElement == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't get DecimalString Element at tag " << std::hex << group << ' ' << element << std::dec);
  }
  OFString ofString;
  if (decimalStringElement->getOFStringArray(ofString) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't get DecimalString Value at tag " << std::hex << group << ' ' << element << std::dec);
  }
  target = "";
  for (auto j : ofString)
  {
    target += j;
  }
  return EXIT_SUCCESS;
}

int DCMTKSequence::GetElementSQ(const unsigned short group, const unsigned short element, DCMTKSequence& target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmStack resultStack;
  this->GetStack(group, element, resultStack);

  auto* seqElement = dynamic_cast<DcmSequenceOfItems*>(resultStack.top());
  if (seqElement == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't get  at tag " << std::hex << group << ' ' << element << std::dec);
  }
  target.SetDcmSequenceOfItems(seqElement);
  return EXIT_SUCCESS;
}

int DCMTKSequence::GetElementItem(unsigned short index, DCMTKItem& target, const bool throwException) const
{
  DcmItem* itemElement = this->m_DcmSequenceOfItems->getItem(index);
  if (itemElement == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't get item " << index);
  }
  target.SetDcmItem(itemElement);
  return EXIT_SUCCESS;
}

int DCMTKSequence::GetElementTM(const unsigned short group, const unsigned short element, std::string& target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmStack resultStack;
  this->GetStack(group, element, resultStack);

  auto* dcmTime = dynamic_cast<DcmTime*>(resultStack.top());
  if (dcmTime == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't get  at tag " << std::hex << group << ' ' << element << std::dec);
  }
  char* cs;
  dcmTime->getString(cs);
  target = cs;
  return EXIT_SUCCESS;
}

DCMTKFileReader::~DCMTKFileReader()
{
  delete m_DFile;
}

void DCMTKFileReader::SetFileName(const std::string& fileName)
{
  this->m_FileName = fileName;
}

const std::string& DCMTKFileReader::GetFileName() const
{
  return this->m_FileName;
}

bool DCMTKFileReader::CanReadFile(const std::string& filename)
{
  auto* MInfo = new DcmMetaInfo();
  if (!MInfo)
  {
    return false;
  }
  OFCondition loadCondition = MInfo->loadFile(filename.c_str(), EXS_Unknown, EGL_noChange, 65536);
  delete MInfo;
  if (loadCondition == EC_Normal || loadCondition == EC_FileMetaInfoHeaderMissing)
  {
    return true;
  }
  return false;
}

bool DCMTKFileReader::IsImageFile(const std::string& filename)
{
  if (!DCMTKFileReader::CanReadFile(filename))
  {
    return false;
  }

  bool rval = false;
  auto* image = new DicomImage(filename.c_str());
  if (image && image->getStatus() == EIS_Normal && image->getInterData())
  {
    rval = true;
  }
  delete image;
  return rval;
}

void DCMTKFileReader::LoadFile()
{
  if (this->m_FileName.empty())
  {
    itkGenericExceptionMacro("No filename given");
  }
  delete this->m_DFile;
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
  if (cond != EC_Normal)
  {
    itkGenericExceptionMacro(<< cond.text() << ": reading file " << this->m_FileName);
  }
  this->m_Dataset = this->m_DFile->getDataset();
  this->m_Xfer = this->m_Dataset->getOriginalXfer();
  if (this->m_Dataset->findAndGetSint32(DCM_NumberOfFrames, this->m_FrameCount).bad())
  {
    this->m_FrameCount = 1;
  }
  int fnum(0);
  if (this->GetElementIS(0x0020, 0x0013, fnum, false) == EXIT_SUCCESS)
  {
    this->m_FileNumber = fnum;
  }
}

int DCMTKFileReader::GetElementLO(const unsigned short group, const unsigned short element, std::string& target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmElement* el;
  if (this->m_Dataset->findAndGetElement(tagkey, el) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find tag " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  auto* loItem = dynamic_cast<DcmLongString*>(el);
  if (loItem == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  OFString ofString;
  if (loItem->getOFStringArray(ofString) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant get string from element " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  target = "";
  for (char i : ofString)
  {
    target += i;
  }
  return EXIT_SUCCESS;
}

int DCMTKFileReader::GetElementLO(const unsigned short group, const unsigned short element, std::vector<std::string>& target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmElement* el;
  if (this->m_Dataset->findAndGetElement(tagkey, el) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find tag " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  auto* loItem = dynamic_cast<DcmLongString*>(el);
  if (loItem == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  target.clear();
  OFString ofString;
  for (unsigned long i = 0; loItem->getOFString(ofString, i) == EC_Normal; ++i)
  {
    std::string targetStr = "";
    for (char j : ofString)
    {
      targetStr += j;
    }
    target.push_back(targetStr);
  }
  return EXIT_SUCCESS;
}

/** Get a DecimalString Item as a single string
 */
int DCMTKFileReader::GetElementDS(const unsigned short group, const unsigned short element, std::string& target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmElement* el;
  if (this->m_Dataset->findAndGetElement(tagkey, el) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find tag " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  auto* dsItem = dynamic_cast<DcmDecimalString*>(el);
  if (dsItem == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  OFString ofString;
  if (dsItem->getOFStringArray(ofString) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't get DecimalString Value at tag " << std::hex << group << ' ' << element << std::dec);
  }
  target = "";
  for (char j : ofString)
  {
    target += j;
  }
  return EXIT_SUCCESS;
}

int DCMTKFileReader::GetElementFD(const unsigned short group, const unsigned short element, int count, double* target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmElement* el;
  if (this->m_Dataset->findAndGetElement(tagkey, el) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find tag " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  auto* fdItem = dynamic_cast<DcmFloatingPointDouble*>(el);
  if (fdItem == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
  }

  std::stringstream ss;
  ss << count;
  OFString vm(ss.str().c_str());
  if (fdItem->checkValue(vm).bad())
  {
    DCMTKExceptionOrErrorReturn(<< "Value multiplicity doesn't match " << count << " at  tag " << std::hex << group << ' ' << element << std::dec);
  }
  double* doubleArray;
  if (fdItem->getFloat64Array(doubleArray) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant extract Array from DecimalString " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  for (int i = 0; i < count; ++i)
  {
    target[i] = doubleArray[i];
  }
  return EXIT_SUCCESS;
}

int DCMTKFileReader::GetElementFD(const unsigned short group, const unsigned short element, double& target, const bool throwException) const
{
  return this->GetElementFD(group, element, 1, &target, throwException);
}

int DCMTKFileReader::GetElementFL(const unsigned short group, const unsigned short element, float& target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmElement* el;
  if (this->m_Dataset->findAndGetElement(tagkey, el) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find tag " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  auto* flItem = dynamic_cast<DcmFloatingPointSingle*>(el);
  if (flItem == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  if (flItem->getFloat32(target) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant extract Array from DecimalString " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  return EXIT_SUCCESS;
}
int DCMTKFileReader::GetElementFLorOB(const unsigned short group, const unsigned short element, float& target, const bool throwException) const
{
  if (this->GetElementFL(group, element, target, false) == EXIT_SUCCESS)
  {
    return EXIT_SUCCESS;
  }
  std::string val;
  if (this->GetElementOB(group, element, val, throwException) != EXIT_SUCCESS)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  const char* data = val.c_str();
  const auto* fptr = reinterpret_cast<const float*>(data);
  target = *fptr;
  switch (this->GetTransferSyntax())
  {
    case EXS_LittleEndianImplicit:
    case EXS_LittleEndianExplicit: itk::ByteSwapper<float>::SwapFromSystemToLittleEndian(&target); break;
    case EXS_BigEndianImplicit:
    case EXS_BigEndianExplicit: itk::ByteSwapper<float>::SwapFromSystemToBigEndian(&target); break;
    default: break;
  }
  return EXIT_SUCCESS;
}

int DCMTKFileReader::GetElementUS(const unsigned short group, const unsigned short element, unsigned short& target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmElement* el;
  if (this->m_Dataset->findAndGetElement(tagkey, el) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find tag " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  auto* usItem = dynamic_cast<DcmUnsignedShort*>(el);
  if (usItem == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  if (usItem->getUint16(target) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant extract Array from DecimalString " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  return EXIT_SUCCESS;
}
int DCMTKFileReader::GetElementUS(const unsigned short group, const unsigned short element, unsigned short*& target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmElement* el;
  if (this->m_Dataset->findAndGetElement(tagkey, el) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find tag " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  auto* usItem = dynamic_cast<DcmUnsignedShort*>(el);
  if (usItem == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  if (usItem->getUint16Array(target) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant extract Array from DecimalString " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  return EXIT_SUCCESS;
}
/** Get a DecimalString Item as a single string
 */
int DCMTKFileReader::GetElementCS(const unsigned short group, const unsigned short element, std::string& target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmElement* el;
  if (this->m_Dataset->findAndGetElement(tagkey, el) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find tag " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  auto* csItem = dynamic_cast<DcmCodeString*>(el);
  if (csItem == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  OFString ofString;
  if (csItem->getOFStringArray(ofString) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't get DecimalString Value at tag " << std::hex << group << ' ' << element << std::dec);
  }
  target = "";
  for (char j : ofString)
  {
    target += j;
  }
  return EXIT_SUCCESS;
}
int DCMTKFileReader::GetElementCSorOB(const unsigned short group, const unsigned short element, std::string& target, const bool throwException) const
{
  if (this->GetElementCS(group, element, target, false) == EXIT_SUCCESS)
  {
    return EXIT_SUCCESS;
  }
  if (this->GetElementOB(group, element, target, throwException) != EXIT_SUCCESS)
  {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int DCMTKFileReader::GetElementPN(const unsigned short group, const unsigned short element, std::string& target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmElement* el;
  if (this->m_Dataset->findAndGetElement(tagkey, el) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find tag " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  auto* pnItem = dynamic_cast<DcmPersonName*>(el);
  if (pnItem == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  OFString ofString;
  if (pnItem->getOFStringArray(ofString) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't get DecimalString Value at tag " << std::hex << group << ' ' << element << std::dec);
  }
  target = "";
  for (char j : ofString)
  {
    target += j;
  }
  return EXIT_SUCCESS;
}

/** get an IS (Integer String Item
 */
int DCMTKFileReader::GetElementIS(const unsigned short group, const unsigned short element, int32_t& target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmElement* el;
  if (this->m_Dataset->findAndGetElement(tagkey, el) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find tag " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  auto* isItem = dynamic_cast<DcmIntegerString*>(el);
  if (isItem == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  Sint32 _target; // MSVC seems to have type conversion problems with
                  // using int32_t as an argument to getSint32
  if (isItem->getSint32(_target) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't get DecimalString Value at tag " << std::hex << group << ' ' << element << std::dec);
  }
  target = static_cast<itk::int32_t>(_target);
  return EXIT_SUCCESS;
}

int DCMTKFileReader::GetElementSL(const unsigned short group, const unsigned short element, int32_t& target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmElement* el;
  if (this->m_Dataset->findAndGetElement(tagkey, el) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find tag " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  auto* isItem = dynamic_cast<DcmSignedLong*>(el);
  if (isItem == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  Sint32 _target; // MSVC seems to have type conversion problems with
                  // using int32_t as an argument to getSint32
  if (isItem->getSint32(_target) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't get DecimalString Value at tag " << std::hex << group << ' ' << element << std::dec);
  }
  target = static_cast<itk::int32_t>(_target);
  return EXIT_SUCCESS;
}

int DCMTKFileReader::GetElementISorOB(const unsigned short group, const unsigned short element, int32_t& target, const bool throwException) const
{
  if (this->GetElementIS(group, element, target, false) == EXIT_SUCCESS)
  {
    return EXIT_SUCCESS;
  }
  std::string val;
  if (this->GetElementOB(group, element, val, throwException) != EXIT_SUCCESS)
  {
    return EXIT_FAILURE;
  }
  const char* data = val.c_str();
  const auto* iptr = reinterpret_cast<const int*>(data);
  target = *iptr;
  switch (this->GetTransferSyntax())
  {
    case EXS_LittleEndianImplicit:
    case EXS_LittleEndianExplicit: itk::ByteSwapper<int>::SwapFromSystemToLittleEndian(&target); break;
    case EXS_BigEndianImplicit:
    case EXS_BigEndianExplicit: itk::ByteSwapper<int>::SwapFromSystemToBigEndian(&target); break;
    default: // no idea what to do
      break;
  }

  return EXIT_SUCCESS;
}

/** get an OB OtherByte Item
 */
int DCMTKFileReader::GetElementOB(const unsigned short group, const unsigned short element, std::string& target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmElement* el;
  if (this->m_Dataset->findAndGetElement(tagkey, el) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find tag " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  auto* obItem = dynamic_cast<DcmOtherByteOtherWord*>(el);
  if (obItem == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  Uint8* bytes;
  obItem->getUint8Array(bytes);
  Uint32 length = obItem->getLength();
  std::string val;
  for (unsigned int i = 0; i < length; ++i)
  {
    val += bytes[i];
  }
  target = val;
  return EXIT_SUCCESS;
}

int DCMTKFileReader::GetElementSQ(const unsigned short group, unsigned short entry, DCMTKSequence& sequence, const bool throwException) const
{
  DcmSequenceOfItems* seq;
  DcmTagKey tagKey(group, entry);

  if (this->m_Dataset->findAndGetSequence(tagKey, seq) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't find sequence " << std::hex << group << ' ' << std::hex << entry)
  }
  sequence.SetDcmSequenceOfItems(seq);
  return EXIT_SUCCESS;
}

int DCMTKFileReader::GetElementUI(const unsigned short group, unsigned short entry, std::string& target, const bool throwException) const
{
  DcmTagKey tagKey(group, entry);
  DcmElement* el;
  if (this->m_Dataset->findAndGetElement(tagKey, el) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find tag " << std::hex << group << ' ' << std::hex << entry << std::dec);
  }
  auto* uiItem = dynamic_cast<DcmUniqueIdentifier*>(el);
  if (uiItem == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't convert data item " << group << ',' << entry);
  }
  OFString ofString;
  if (uiItem->getOFStringArray(ofString, false) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't get UID Value at tag " << std::hex << group << ' ' << std::hex << entry << std::dec);
  }
  target = "";
  for (char j : ofString)
  {
    target.push_back(j);
  }
  return EXIT_SUCCESS;
}

int DCMTKFileReader::GetElementAsString(const unsigned short group, const unsigned short element, std::string& target, const bool throwException) const
{
  DcmTagKey tagKey(group, element);
  OFString ofString;
  if (this->m_Dataset->findAndGetOFStringArray(tagKey, ofString) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find tag " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  target.assign(ofString.c_str(), ofString.length());
  return EXIT_SUCCESS;
}

int DCMTKFileReader::GetElementDA(const unsigned short group, const unsigned short element, std::string& target, const bool throwException) const
{
  DcmTagKey tagkey(group, element);
  DcmElement* el;
  if (this->m_Dataset->findAndGetElement(tagkey, el) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find tag " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  auto* dcmDate = dynamic_cast<DcmDate*>(el);
  if (dcmDate == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't get  at tag " << std::hex << group << ' ' << element << std::dec);
  }
  char* cs;
  dcmDate->getString(cs);
  target = cs;
  return EXIT_SUCCESS;
}

int DCMTKFileReader::GetElementTM(const unsigned short group, const unsigned short element, std::string& target, const bool throwException) const
{

  DcmTagKey tagkey(group, element);
  DcmElement* el;
  if (this->m_Dataset->findAndGetElement(tagkey, el) != EC_Normal)
  {
    DCMTKExceptionOrErrorReturn(<< "Cant find tag " << std::hex << group << ' ' << std::hex << element << std::dec);
  }
  auto* dcmTime = dynamic_cast<DcmTime*>(el);
  if (dcmTime == nullptr)
  {
    DCMTKExceptionOrErrorReturn(<< "Can't get  at tag " << std::hex << group << ' ' << element << std::dec);
  }
  char* cs;
  dcmTime->getString(cs);
  target = cs;
  return EXIT_SUCCESS;
}

int DCMTKFileReader::GetDirCosArray(double* const dircos) const
{
  int rval;
  DCMTKSequence planeSeq;

  dircos[0] = 1;
  dircos[1] = 0;
  dircos[2] = 0;
  dircos[3] = 0;
  dircos[4] = 1;
  dircos[5] = 0;
  // check toplevel ImageOrientationPatient (most common case)
  if ((rval = this->GetElementDS(0x0020, 0x0037, 6, dircos, false)) == EXIT_SUCCESS)
  {
    return rval;
  }
  // look inside plane sequence at top level (probably not common)
  rval = this->GetElementSQ(0x0020, 0x9116, planeSeq, false);
  if (rval == EXIT_SUCCESS)
  {
    rval = planeSeq.GetElementDS(0x0020, 0x0037, 6, dircos, false);
    return rval;
  }
  // ImageOrientationPatient not at top level or toplevel plane sequence,
  // so look in the shared and per-frame functional groups
  // as standard for multiframe (e.g. Philips) images
  unsigned short candidateSequences[2] = {
    0x9229, // check for Shared Functional Group Sequence first
    0x9230, // check the Per-frame Functional Groups Sequence
  };
  for (unsigned short candidateSequence : candidateSequences)
  {
    rval = this->GetElementSQ(0x5200, candidateSequence, planeSeq, false);
    if (rval == EXIT_SUCCESS)
    {
      DCMTKItem item;
      rval = planeSeq.GetElementItem(0, item, false);
      if (rval == EXIT_SUCCESS)
      {
        DCMTKSequence subSequence;
        // Plane Orientation sequence
        rval = item.GetElementSQ(0x0020, 0x9116, subSequence, false);
        if (rval == EXIT_SUCCESS)
        {
          // Image Orientation Patient
          rval = subSequence.GetElementDS(0x0020, 0x0037, 6, dircos, false);
          if (rval == EXIT_SUCCESS)
          {
            break;
          }
        }
      }
    }
  }
  return rval;
}

int DCMTKFileReader::GetDirCosines(vnl_vector<double>& dir1, vnl_vector<double>& dir2, vnl_vector<double>& dir3) const
{
  double dircos[6];
  int rval = this->GetDirCosArray(dircos);
  if (rval == EXIT_SUCCESS)
  {
    dir1[0] = dircos[0];
    dir1[1] = dircos[1];
    dir1[2] = dircos[2];
    dir2[0] = dircos[3];
    dir2[1] = dircos[4];
    dir2[2] = dircos[5];
    dir3 = vnl_cross_3d(dir1, dir2);
  }
  return rval;
}

int DCMTKFileReader::GetSlopeIntercept(double& slope, double& intercept) const
{
  if (this->GetElementDS<double>(0x0028, 0x1053, 1, &slope, false) != EXIT_SUCCESS)
  {
    slope = 1.0;
  }
  if (this->GetElementDS<double>(0x0028, 0x1052, 1, &intercept, false) != EXIT_SUCCESS)
  {
    intercept = 0.0;
  }
  return EXIT_SUCCESS;
}

IOPixelEnum DCMTKFileReader::GetImagePixelType() const
{
  unsigned short SamplesPerPixel;
  if (this->GetElementUS(0x0028, 0x0100, SamplesPerPixel, false) != EXIT_SUCCESS)
  {
    return IOPixelEnum::UNKNOWNPIXELTYPE;
  }
  IOPixelEnum pixelType;
  switch (SamplesPerPixel)
  {
    case 8:
    case 16: pixelType = IOPixelEnum::SCALAR; break;
    case 24: pixelType = IOPixelEnum::RGB; break;
    default: pixelType = IOPixelEnum::VECTOR;
  }
  return pixelType;
}

IOComponentEnum DCMTKFileReader::GetImageDataType() const
{
  unsigned short IsSigned;
  unsigned short BitsAllocated;
  unsigned short BitsStored;
  unsigned short HighBit;
  IOComponentEnum type = IOComponentEnum::UNKNOWNCOMPONENTTYPE;

  if (this->GetElementUS(0x0028, 0x0100, BitsAllocated, false) != EXIT_SUCCESS || this->GetElementUS(0x0028, 0x0101, BitsStored, false) != EXIT_SUCCESS
      || this->GetElementUS(0x0028, 0x0102, HighBit, false) != EXIT_SUCCESS || this->GetElementUS(0x0028, 0x0103, IsSigned, false) != EXIT_SUCCESS)
  {
    return type;
  }
  double slope, intercept;
  this->GetSlopeIntercept(slope, intercept);

  switch (BitsAllocated)
  {
    case 1:
    case 8:
    case 24: // RGB?
      if (IsSigned)
      {
        type = IOComponentEnum::CHAR;
      }
      else
      {
        type = IOComponentEnum::UCHAR;
      }
      break;
    case 12:
    case 16:
      if (IsSigned)
      {
        type = IOComponentEnum::SHORT;
      }
      else
      {
        type = IOComponentEnum::USHORT;
      }
      break;
    case 32:
      if (IsSigned)
      {
        type = IOComponentEnum::INT;
      }
      else
      {
        type = IOComponentEnum::UINT;
      }
      break;
    case 64:
      if (IsSigned)
      {
        type = IOComponentEnum::LONGLONG;
      }
      else
      {
        type = IOComponentEnum::ULONGLONG;
      }
      break;
    case 0:
    default:
      break;
      // assert(0);
  }

  return type;
}

int DCMTKFileReader::GetDimensions(unsigned short& rows, unsigned short& columns) const
{
  if (this->GetElementUS(0x0028, 0x0010, rows, false) != EXIT_SUCCESS || this->GetElementUS(0x0028, 0x0011, columns, false) != EXIT_SUCCESS)
  {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int DCMTKFileReader::GetSpacing(double* const spacing) const
{
  spacing[0] = 1.0;
  spacing[1] = 1.0;
  spacing[2] = 1.0;

  double _spacing[3] = { 1.0, 1.0, 1.0 };
  bool _have_val[3] = { false, false, false };
  //
  // There are several tags that can have spacing, and we're going
  // from most to least desirable, starting with PixelSpacing, which
  // is guaranteed to be in patient space.
  // Imager Pixel spacing is inter-pixel spacing at the sensor front plane
  // Pixel spacing

  // first, shared function groups sequence, then
  // per-frame groups sequence

  int rval = EXIT_SUCCESS;
  /*
   * According Dicom standard (DICOM PS3.6 2016b - Data Dictionary)
   * (0028, 0030) indicates physical X,Y spacing inside a slice;
   * (0018, 0088) indicates physical Z spacing between slices;
   *  which above are also consistent with dcm2niix software.
   *  when we can not get (0018, 0088), we should compute spacing
   * from the planes' positions (TODO, see PR 112).
   * */
  rval = this->GetElementDS<double>(0x0028, 0x0030, 2, _spacing, false);
  if (rval != EXIT_SUCCESS)
  {
    // imager pixel spacing
    rval = this->GetElementDS<double>(0x0018, 0x1164, 2, &_spacing[0], false);
    if (rval != EXIT_SUCCESS)
    {
      // Nominal Scanned PixelSpacing
      rval = this->GetElementDS<double>(0x0018, 0x2010, 2, &_spacing[0], false);
    }
  }
  if (rval == EXIT_SUCCESS)
  {
    // slice thickness
    spacing[0] = _spacing[1];
    spacing[1] = _spacing[0];
    _have_val[0] = true;
    _have_val[1] = true;
  }

  if (GetElementDS<double>(0x0018, 0x0088, 1, &_spacing[2], false) == EXIT_SUCCESS)
  {
    spacing[2] = _spacing[2];
    _have_val[2] = true;
  }

  if (_have_val[0] && _have_val[1] && _have_val[2])
  {
    return EXIT_SUCCESS;
  }

  // this is for multiframe images -- preferentially use the shared
  // functional group, and then the per-frame functional group
  unsigned short candidateSequences[2] = {
    0x9229, // check for Shared Functional Group Sequence first
    0x9230, // check the Per-frame Functional Groups Sequence
  };
  for (unsigned short candidateSequence : candidateSequences)
  {
    DCMTKSequence spacingSequence;
    rval = this->GetElementSQ(0x5200, candidateSequence, spacingSequence, false);
    if (rval == EXIT_SUCCESS)
    {
      DCMTKItem item;
      rval = spacingSequence.GetElementItem(0, item, false);
      if (rval == EXIT_SUCCESS)
      {
        DCMTKSequence subSequence;
        // Pixel Measures Sequence
        rval = item.GetElementSQ(0x0028, 0x9110, subSequence, false);
        if (rval == EXIT_SUCCESS)
        {
          /*
           * According Dicom standard (DICOM PS3.6 2016b - Data Dictionary)
           * (0028, 0030) indicates physical X,Y spacing inside a slice;
           * (0018, 0088) indicates physical Z spacing between slices;
           *  which above are also consistent with Dcom2iix software.
           *  when we can not get (0018, 0088), we should compute spacing
           * from the planes' positions (TODO, see PR 112).
           * */
          if (!(_have_val[0] && _have_val[1]))
          {
            if (subSequence.GetElementDS<double>(0x0028, 0x0030, 2, _spacing, false) == EXIT_SUCCESS)
            {
              spacing[0] = _spacing[1];
              spacing[1] = _spacing[0];
              _have_val[0] = true;
              _have_val[1] = true;
            }
          }
          if (!_have_val[2])
          {
            if (subSequence.GetElementDS<double>(0x0018, 0x0088, 1, &_spacing[2], false) == EXIT_SUCCESS)
            {
              spacing[2] = _spacing[2];
              _have_val[2] = true;
            }
          }
          if (_have_val[0] && _have_val[1] && _have_val[2])
          {
            return EXIT_SUCCESS;
          }
        }
      }
    }
  }

  if (_have_val[0] && _have_val[1] && _have_val[2])
  {
    return EXIT_SUCCESS;
  }

  // not all (or no) spacing values could be found.
  spacing[0] = _spacing[0];
  spacing[1] = _spacing[1];
  spacing[2] = _spacing[2];
  return EXIT_FAILURE;
}

int DCMTKFileReader::GetOrigin(double* const origin) const
{
  int rval = EXIT_SUCCESS;
  // if the origin has yet to be cached
  {
    std::fill(origin, origin + 3, 0.0);
    DCMTKSequence originSequence;
    DCMTKItem item;
    DCMTKSequence subSequence;

    rval = this->GetElementDS<double>(0x0020, 0x0032, 3, origin, false);
    if (rval == EXIT_SUCCESS)
    {
      return EXIT_SUCCESS;
    }
    // this is for multiframe images -- preferentially use the shared
    // functional group, and then the per-frame functional group
    unsigned short candidateSequences[2] = {
      0x9229, // check for Shared Functional Group Sequence first
      0x9230, // check the Per-frame Functional Groups Sequence
    };
    for (unsigned short candidateSequence : candidateSequences)
    {
      rval = this->GetElementSQ(0x5200, candidateSequence, originSequence, false);
      if (rval != EXIT_SUCCESS)
      {
        continue;
      }
      rval = originSequence.GetElementItem(0, item, false);
      if (rval != EXIT_SUCCESS)
      {
        continue;
      }
      rval = item.GetElementSQ(0x0020, 0x9113, subSequence, false);
      if (rval != EXIT_SUCCESS)
      {
        continue;
      }
      subSequence.GetElementDS<double>(0x0020, 0x0032, 3, origin, true);
      if (rval != EXIT_SUCCESS)
      {
        continue;
      }
      break;
    }
  }
  return rval;
}

bool DCMTKFileReader::HasPixelData() const
{
  DcmTagKey tagkey(0x7fe0, 0x0010);
  DcmStack resultStack;
  return this->m_DFile->search(tagkey, resultStack) == EC_Normal;
}

int DCMTKFileReader::GetFrameCount() const
{
  return this->m_FrameCount;
}

E_TransferSyntax DCMTKFileReader::GetTransferSyntax() const
{
  return m_Xfer;
}

long DCMTKFileReader::GetFileNumber() const
{
  return m_FileNumber;
}

void DCMTKFileReader::PopulateMetaDataDictionary(MetaDataDictionary& dict) const
{
  dict.Clear();
  if (m_Dataset == nullptr)
  {
    return;
  }
  const unsigned long numElements = m_Dataset->card();
  for (unsigned long i = 0; i < numElements; ++i)
  {
    DcmElement* element = m_Dataset->getElement(i);
    if (element == nullptr)
    {
      continue;
    }
    const DcmTag& tag = element->getTag();
    // Skip pixel data (7FE0,0010)
    if (tag.getGroup() == 0x7fe0 && tag.getElement() == 0x0010)
    {
      continue;
    }
    // Format key as "GGGG|EEEE" (lowercase hex, matching GDCMImageIO)
    char key[10];
    std::snprintf(key, sizeof(key), "%04x|%04x", tag.getGroup(), tag.getElement());

    const DcmEVR vr = element->getVR();
    if (vr == EVR_SQ)
    {
      // Sequences are nested datasets, not byte arrays; getUint8Array() does
      // not return their content.  Skip rather than emit an empty entry.
      continue;
    }
    if (vr == EVR_OB || vr == EVR_OW || vr == EVR_OF || vr == EVR_OD || vr == EVR_OL || vr == EVR_OV || vr == EVR_UN || vr == EVR_ox || vr == EVR_px)
    {
      // Binary VR — base64-encode the raw bytes
      Uint8* byteValue = nullptr;
      if (element->getUint8Array(byteValue) == EC_Normal && byteValue != nullptr)
      {
        const Uint32 length = element->getLength();
        if (length > 0)
        {
          int encodedLengthEstimate = 2 * static_cast<int>(length);
          encodedLengthEstimate = ((encodedLengthEstimate / 4) + 1) * 4;
          const auto bin = std::make_unique<char[]>(encodedLengthEstimate);
          const auto encodedLengthActual = static_cast<unsigned int>(
            itksysBase64_Encode(reinterpret_cast<const unsigned char*>(byteValue), static_cast<SizeValueType>(length), reinterpret_cast<unsigned char*>(bin.get()), 0));
          EncapsulateMetaData<std::string>(dict, key, std::string(bin.get(), encodedLengthActual));
        }
      }
    }
    else
    {
      OFString value;
      if (element->getOFStringArray(value) == EC_Normal)
      {
        EncapsulateMetaData<std::string>(dict, key, std::string(value.c_str()));
      }
    }
  }
}

void DCMTKFileReader::AddDictEntry(DcmDictEntry* entry)
{
  DcmDataDictionary& dict = dcmDataDict.wrlock();
  dict.addEntry(entry);
#if OFFIS_DCMTK_VERSION_NUMBER < 364
  dcmDataDict.unlock();
#else
  dcmDataDict.wrunlock();
#endif
}

bool CompareDCMTKFileReaders(DCMTKFileReader* a, DCMTKFileReader* b)
{
  return a->GetFileNumber() < b->GetFileNumber();
}

} // namespace itk
