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
#ifndef itkDCMTKFileReader_h
#define itkDCMTKFileReader_h

// Vendored copy: define the ITKIODCMTK export macros as no-ops so the
// reader builds directly into the consuming module (parity with
// SlicerExt PETDICOMExtension/SUVFactorCalculatorCLI). Sourced from ITK
// Modules/IO/DCMTK/src after ITK commit db492d9b09 made the header private.
#ifndef ITKIODCMTK_EXPORT
# define ITKIODCMTK_EXPORT
#endif
#include <stack>
#include <vector>
#include "itkByteSwapper.h"
#include "itkIntTypes.h"
#include "vnl/vnl_vector.h"
#include "dcmtk/dcmdata/dcdict.h" // For DcmDataDictionary
#include "dcmtk/dcmdata/dcdicent.h"
#include "dcmtk/dcmdata/dcxfer.h"
#include "dcmtk/dcmdata/dcvrds.h"
#include "dcmtk/dcmdata/dcstack.h"
#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmdata/dcitem.h"
#include "dcmtk/dcmdata/dcvrobow.h"
#include "dcmtk/dcmdata/dcsequen.h"
#include "itkMacro.h"
#include "itkImageIOBase.h"
#include "itkMetaDataDictionary.h"

class DcmSequenceOfItems;
class DcmFileFormat;
class DcmDictEntry;

// Don't print error messages if you're not throwing
// an exception
//     std::cerr body;
#define DCMTKExceptionOrErrorReturn(body) \
  {                                       \
    if (throwException)                   \
    {                                     \
      itkGenericExceptionMacro(body);     \
    }                                     \
    else                                  \
    {                                     \
      return EXIT_FAILURE;                \
    }                                     \
  }

namespace itk
{
// Forward reference because of circular dependencies
class ITK_FORWARD_EXPORT DCMTKSequence;

class ITKIODCMTK_EXPORT DCMTKItem
{
public:
  DCMTKItem() = default;

  void SetDcmItem(DcmItem* item);
  int GetElementSQ(const unsigned short group, const unsigned short entry, DCMTKSequence& sequence, const bool throwException = true) const;

private:
  DcmItem* m_DcmItem{ nullptr };
};

class ITKIODCMTK_EXPORT DCMTKSequence
{
public:
  DCMTKSequence() = default;
  void SetDcmSequenceOfItems(DcmSequenceOfItems* seq);
  int card() const;
  int GetSequence(unsigned long index, DCMTKSequence& target, const bool throwException = true) const;
  int GetStack(const unsigned short group, const unsigned short element, DcmStack& resultStack, const bool throwException = true) const;
  int GetElementCS(const unsigned short group, const unsigned short element, std::string& target, const bool throwException = true) const;

  int GetElementOB(const unsigned short group, const unsigned short element, std::string& target, const bool throwException = true) const;

  int GetElementCSorOB(const unsigned short group, const unsigned short element, std::string& target, const bool throwException = true) const;

  template <typename TType>
  int GetElementDSorOB(const unsigned short group, const unsigned short element, TType& target, const bool throwException = true) const
  {
    if (this->GetElementDS<TType>(group, element, 1, &target, false) == EXIT_SUCCESS)
    {
      return EXIT_SUCCESS;
    }
    std::string val;
    if (this->GetElementOB(group, element, val, throwException) != EXIT_SUCCESS)
    {
      DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
    }
    const char* data = val.c_str();
    const auto* fptr = reinterpret_cast<const TType*>(data);
    target = *fptr;
    return EXIT_SUCCESS;
  }

  template <typename TType>
  int GetElementDSorOB(const unsigned short group, const unsigned short element, int count, TType* target, const bool throwException = true) const
  {
    if (this->GetElementDS<TType>(group, element, count, target, false) == EXIT_SUCCESS)
    {
      return EXIT_SUCCESS;
    }
    std::string val;
    if (this->GetElementOB(group, element, val, throwException) != EXIT_SUCCESS)
    {
      DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
    }
    const char* data = val.c_str();
    const auto* fptr = reinterpret_cast<const TType*>(data);
    for (int i = 0; i < count; ++i)
    {
      target[i] = fptr[i];
    }
    return EXIT_SUCCESS;
  }

  int GetElementFD(const unsigned short group, const unsigned short element, int count, double* target, const bool throwException = true) const;
  int GetElementFD(const unsigned short group, const unsigned short element, double& target, const bool throwException = true) const;
  int GetElementDS(const unsigned short group, const unsigned short element, std::string& target, const bool throwException = true) const;
  int GetElementTM(const unsigned short group, const unsigned short element, std::string& target, const bool throwException = true) const;
  /** Get an array of data values, as contained in a DICOM
   * DecimalString Item
   */
  template <typename TType>
  int GetElementDS(const unsigned short group, const unsigned short element, unsigned short count, TType* target, const bool throwException = true) const
  {
    DcmStack resultStack;
    if (this->GetStack(group, element, resultStack, throwException) != EXIT_SUCCESS)
    {
      return EXIT_FAILURE;
    }
    auto* dsItem = dynamic_cast<DcmDecimalString*>(resultStack.top());
    if (dsItem == nullptr)
    {
      DCMTKExceptionOrErrorReturn(<< "Can't get DecimalString Element at tag " << std::hex << group << ' ' << element << std::dec);
    }

    OFVector<Float64> doubleVals;
    if (dsItem->getFloat64Vector(doubleVals) != EC_Normal)
    {
      DCMTKExceptionOrErrorReturn(<< "Cant extract Array from DecimalString " << std::hex << group << ' ' << std::hex << element << std::dec);
    }
    if (doubleVals.size() != count)
    {
      DCMTKExceptionOrErrorReturn(<< "DecimalString " << std::hex << group << ' ' << std::hex << element << " expected " << count << "items, but found " << doubleVals.size()
                                  << std::dec);
    }
    for (unsigned int i = 0; i < count; ++i)
    {
      target[i] = static_cast<TType>(doubleVals[i]);
    }
    return EXIT_SUCCESS;
  }
  int GetElementSQ(const unsigned short group, const unsigned short element, DCMTKSequence& target, const bool throwException = true) const;
  int GetElementItem(unsigned short itemIndex, DCMTKItem& target, const bool throwException = true) const;

  void print(std::ostream& out) const { this->m_DcmSequenceOfItems->print(out); }

private:
  DcmSequenceOfItems* m_DcmSequenceOfItems{ nullptr };
};

class ITKIODCMTK_EXPORT DCMTKFileReader
{
public:
  using Self = DCMTKFileReader;

  DCMTKFileReader() = default;
  ~DCMTKFileReader();

  void SetFileName(const std::string& fileName);

  const std::string& GetFileName() const;

  void LoadFile();

  int GetElementLO(const unsigned short group, const unsigned short element, std::string& target, const bool throwException = true) const;
  int GetElementLO(const unsigned short group, const unsigned short element, std::vector<std::string>& target, const bool throwException = true) const;

  /** Get an array of data values, as contained in a DICOM
   * DecimalString Item
   */
  template <typename TType>
  int GetElementDS(const unsigned short group, const unsigned short element, unsigned short count, TType* target, const bool throwException = true) const
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
    OFVector<Float64> doubleVals;
    if (dsItem->getFloat64Vector(doubleVals) != EC_Normal)
    {
      DCMTKExceptionOrErrorReturn(<< "Cant extract Array from DecimalString " << std::hex << group << ' ' << std::hex << element << std::dec);
    }
    if (doubleVals.size() != count)
    {
      DCMTKExceptionOrErrorReturn(<< "DecimalString " << std::hex << group << ' ' << std::hex << element << " expected " << count << "items, but found " << doubleVals.size()
                                  << std::dec);
    }
    for (unsigned int i = 0; i < count; ++i)
    {
      target[i] = static_cast<TType>(doubleVals[i]);
    }
    return EXIT_SUCCESS;
  }

  template <typename TType>
  int GetElementDSorOB(const unsigned short group, const unsigned short element, TType& target, const bool throwException = true) const
  {
    if (this->GetElementDS<TType>(group, element, 1, &target, false) == EXIT_SUCCESS)
    {
      return EXIT_SUCCESS;
    }
    std::string val;
    if (this->GetElementOB(group, element, val, throwException) != EXIT_SUCCESS)
    {
      DCMTKExceptionOrErrorReturn(<< "Cant find DecimalString element " << std::hex << group << ' ' << std::hex << element << std::dec);
    }
    const char* data = val.c_str();
    const auto* fptr = reinterpret_cast<const TType*>(data);
    target = *fptr;
    switch (this->GetTransferSyntax())
    {
      case EXS_LittleEndianImplicit:
      case EXS_LittleEndianExplicit: itk::ByteSwapper<TType>::SwapFromSystemToLittleEndian(&target); break;
      case EXS_BigEndianImplicit:
      case EXS_BigEndianExplicit: itk::ByteSwapper<TType>::SwapFromSystemToBigEndian(&target); break;
      default: break;
    }
    return EXIT_SUCCESS;
  }
  /** Get a DecimalString Item as a single string
   */
  int GetElementDS(const unsigned short group, const unsigned short element, std::string& target, const bool throwException = true) const;
  int GetElementFD(const unsigned short group, const unsigned short element, double& target, const bool throwException = true) const;
  int GetElementFD(const unsigned short group, const unsigned short element, int count, double* target, const bool throwException = true) const;
  int GetElementFL(const unsigned short group, const unsigned short element, float& target, const bool throwException = true) const;
  int GetElementFLorOB(const unsigned short group, const unsigned short element, float& target, const bool throwException = true) const;

  int GetElementUS(const unsigned short group, const unsigned short element, unsigned short& target, const bool throwException = true) const;
  int GetElementUS(const unsigned short group, const unsigned short element, unsigned short*& target, const bool throwException = true) const;
  /** Get a DecimalString Item as a single string
   */
  int GetElementCS(const unsigned short group, const unsigned short element, std::string& target, const bool throwException = true) const;

  /** Get a PersonName Item as a single string
   */
  int GetElementPN(const unsigned short group, const unsigned short element, std::string& target, const bool throwException = true) const;

  /** get an IS (Integer String Item
   */
  int GetElementIS(const unsigned short group, const unsigned short element, itk::int32_t& target, const bool throwException = true) const;

  int GetElementSL(const unsigned short group, const unsigned short element, itk::int32_t& target, const bool throwException = true) const;

  int GetElementISorOB(const unsigned short group, const unsigned short element, itk::int32_t& target, const bool throwException = true) const;

  int GetElementCSorOB(const unsigned short group, const unsigned short element, std::string& target, const bool throwException = true) const;

  /** get an OB OtherByte Item
   */
  int GetElementOB(const unsigned short group, const unsigned short element, std::string& target, const bool throwException = true) const;

  int GetElementSQ(const unsigned short group, unsigned short entry, DCMTKSequence& sequence, const bool throwException = true) const;

  int GetElementUI(const unsigned short group, unsigned short entry, std::string& target, const bool throwException = true) const;

  /** Read any element as its string representation, regardless of VR. */
  int GetElementAsString(const unsigned short group, const unsigned short element, std::string& target, const bool throwException = true) const;

  int GetElementDA(const unsigned short group, const unsigned short element, std::string& target, const bool throwException = true) const;

  int GetElementTM(const unsigned short group, const unsigned short element, std::string& target, const bool throwException = true) const;

  int GetDirCosines(vnl_vector<double>& dir1, vnl_vector<double>& dir2, vnl_vector<double>& dir3) const;

  int GetDirCosArray(double* const dircos) const;

  int GetFrameCount() const;

  int GetSlopeIntercept(double& slope, double& intercept) const;

  int GetDimensions(unsigned short& rows, unsigned short& columns) const;

  IOComponentEnum GetImageDataType() const;
  IOPixelEnum GetImagePixelType() const;

  int GetSpacing(double* const spacing) const;
  int GetOrigin(double* const origin) const;

  bool HasPixelData() const;

  E_TransferSyntax GetTransferSyntax() const;

  long GetFileNumber() const;
  static void AddDictEntry(DcmDictEntry* entry);

  static bool CanReadFile(const std::string& filename);
  static bool IsImageFile(const std::string& filename);

  void PopulateMetaDataDictionary(MetaDataDictionary& dict) const;

private:
  std::string m_FileName;
  DcmFileFormat* m_DFile{ nullptr };
  DcmDataset* m_Dataset{ nullptr };
  E_TransferSyntax m_Xfer{ EXS_Unknown };
  Sint32 m_FrameCount{ 0 };
  long m_FileNumber{ -1L };
};

extern bool CompareDCMTKFileReaders(DCMTKFileReader* a, DCMTKFileReader* b);
} // namespace itk

#endif // itkDCMTKFileReader_h
