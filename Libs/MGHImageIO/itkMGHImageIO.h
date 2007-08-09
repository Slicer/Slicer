

#ifndef H_ITK_IMAGE_IO_H
#define H_ITK_IMAGE_IO_H

// STL includes

// ITK includes
#include "itkImageIOBase.h"
#include "itkMatrix.h"

#include <itk_zlib.h>

//#include "itkMGHIOWin32Header.h"

namespace itk
{

  class  MGHImageIO : public ImageIOBase
  {
  public:
    typedef MGHImageIO              Self;
    typedef ImageIOBase             Superclass;
    typedef SmartPointer<Self>      Pointer;

    /** Method for creation through the object factory **/
    itkNewMacro(Self);
    /** RTTI (and related methods) **/
    itkTypeMacro(MGHImageIO, Superclass);

    
    /**--------------- Read the data----------------- **/
    virtual bool CanReadFile(const char* FileNameToRead);
    /* Set the spacing and dimension information for the set file name */
    virtual void ReadImageInformation();
    /* Read the data from the disk into provided memory buffer */
    virtual void Read(void* buffer);
    
    /**---------------Write the data------------------**/
    
    virtual bool CanWriteFile(const char* FileNameToWrite);
    /* Set the spacing and dimension information for the set file name */
    virtual void WriteImageInformation();
    /* Write the data to the disk from the provided memory buffer */
    virtual void Write(const void* buffer);

  protected:
    MGHImageIO();
    ~MGHImageIO();
    void PrintSelf(std::ostream& os, Indent indent) const;

    void ReadVolumeHeader(gzFile fp);

  private:
    // processes the actual data buffer
    void SwapBytesIfNecessary(void* buffer, 
            unsigned long numberOfPixels);
    // examines the direction cosines and creates encapsulation data
    //void MriDirCos();
    
    void WriteUncompressedHeader();
    void WriteCompressedHeader(gzFile fp);
    
    void WriteUncompressedData(const void* buffer);
    void WriteCompressedData(gzFile fp, const void* buffer);

    void PermuteFrameValues(const void* buffer, char* tempmemory);

    unsigned int GetComponentSize() const;

    std::string GetOrientation( itk::Matrix< double > directions );    

  };

}


#endif // H_ITK_IMAGE_IO_H




