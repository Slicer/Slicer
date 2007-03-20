
#include <fstream>

#include "itkMGHImageIO.h"
#include "itkIOCommon.h"
#include "itkExceptionObject.h"
#include "itkByteSwapper.h"
#include "itkMetaDataObject.h"

#include <vnl/vnl_matrix.h>
#include <vnl/vnl_vector.h>
#include <vnl/vnl_cross.h>

#include <cmath>

#include <stdio.h>
#include <stdlib.h>

const int MRI_UCHAR = 0;
const int MRI_INT   = 1;
const int MRI_FLOAT = 3;
const int MRI_SHORT = 4;
const int MRI_TENSOR= 6;

const int FS_DIMENSION_HEADER_SIZE = sizeof(int) * 7;
const int FS_RAS_HEADER_SIZE = (sizeof(float) * 15) + sizeof(short);
const int FS_UNUSED_HEADER_SIZE = 256 - FS_RAS_HEADER_SIZE;
const int FS_WHOLE_HEADER_SIZE =
    FS_RAS_HEADER_SIZE + FS_DIMENSION_HEADER_SIZE + FS_UNUSED_HEADER_SIZE;


//-------------------------------
//
// Convert to BE
//
//-------------------------------

template<class T>
int TReadZ(gzFile iFile,
     T&     out)
{
  T* pt = new T(0);
  int result;
  result = gzread(iFile,
      pt,
      sizeof(T));
  itk::ByteSwapper<T>::SwapFromSystemToBigEndian(pt);
  out = *pt;
  delete pt;
  return result;
}

template<class T>
int TWriteZ(gzFile oFile,
      T value)
{
  T* pt = new T(value);
  int result;
  itk::ByteSwapper<T>::SwapFromSystemToBigEndian(pt);
  result = ::gzwrite(oFile, pt, sizeof(T));
  delete pt;
  return result;
}

template<class T>
void
TWrite(std::ostream& os,
      T value)
{
  T* pt = new T(value);
  itk::ByteSwapper<T>::SwapFromSystemToBigEndian(pt);
  os.write( (const char*)pt, sizeof(T) );
  delete pt;
}

//-------------------------------


static
std::string
GetExtension( const std::string& filename)
{
  const std::string::size_type pos = 
    filename.find_last_of(".");
  std::string extension(filename,
      pos+1,
      filename.length());
  return extension;
}

//--------------------------------------
//
// MGHImageIO
//

namespace itk
{

  MGHImageIO::MGHImageIO() 
  {
    this->SetNumberOfDimensions(3);
    const unsigned int uzero = 0;
    m_Dimensions[0] = uzero;
    m_Dimensions[1] = uzero;
    m_Dimensions[2] = uzero;

    if ( ByteSwapper<int>::SystemIsBigEndian())
      m_ByteOrder = BigEndian;
    else
      m_ByteOrder = LittleEndian;

  }

  MGHImageIO::~MGHImageIO()
  {
  }
    

  bool
  MGHImageIO::CanReadFile(const char* FileNameToRead)
  {
    std::string filename(FileNameToRead);

    if ( filename == "" )
      {
  itkExceptionMacro(<<"A FileName must be specified.");
  return false;
      }

    
    // check if the correct extension is given by the user
    std::string extension = GetExtension(filename);
    if ( extension == std::string("mgh") ||
   extension == std::string("mgz") )
      return true;
    
    if ( extension == std::string("gz") )
      {
  if ( filename.substr( filename.size() - 7 ) == std::string(".mgh.gz") )
    return true;
      }
    
    return false;
  }
  
  void
  MGHImageIO::PrintSelf(std::ostream& os, Indent indent) const
  {
    Superclass::PrintSelf(os, indent);
    std::string strSep = ", ";

    os << indent 
       << "Data Dimensions: (" 
       << m_Dimensions[0] << strSep
       << m_Dimensions[1] << strSep
       << m_Dimensions[2] << ")\n"
       << indent
       << "Data Spacing: (" 
       << m_Spacing[0] << strSep
       << m_Spacing[1] << strSep
       << m_Spacing[2] << ")\n"
       << indent
       << "Scalar Type: " << m_ComponentType << std::endl
       << indent
       << "Number of Frames: " << m_NumberOfComponents << std::endl;
    
    os << indent << "RAS to IJK matrix: " << std::endl;
  }


  void
  MGHImageIO::ReadImageInformation()
  {
    gzFile     fp;
    
    fp = gzopen( m_FileName.c_str(), "rb");
    if ( !fp )
      {
  itkExceptionMacro(<<"Can't find/open file: " << m_FileName);
  return;
      }

    ReadVolumeHeader(fp);

    gzclose(fp);
  }

  void
  MGHImageIO::ReadVolumeHeader(gzFile fp)
  {
    int        version;
    int        bufInt; // buffer -> int type (most ITK types are unsigned)
    int        type;
    int        dof;
    short      RASgood;

    // check file reading

    if ( !fp )
      {
  itkExceptionMacro(<< "Can't find/open file: " << this->m_FileName);
  return;
      }
    TReadZ(fp, version);
    TReadZ(fp, bufInt);   m_Dimensions[0] = (unsigned int)bufInt;
    TReadZ(fp, bufInt);   m_Dimensions[1] = (unsigned int)bufInt;
    TReadZ(fp, bufInt);   m_Dimensions[2] = (unsigned int)bufInt;
    // next is nframes
    TReadZ(fp, bufInt);   m_NumberOfComponents = (unsigned int)bufInt;
    TReadZ(fp, type);
    TReadZ(fp, dof); // what does this do?

    // Convert type to an ITK type
    switch( type )
      {
      case MRI_UCHAR: 
  m_ComponentType = UCHAR; break;
      case MRI_INT:
  m_ComponentType = INT; break;
      case MRI_FLOAT:
  m_ComponentType = FLOAT; break;
      case MRI_SHORT:
  m_ComponentType = SHORT; break;
      case MRI_TENSOR:
  m_ComponentType = FLOAT; m_NumberOfComponents = 9; break;
      default:
  itkExceptionMacro(<<" Unknown data type " << type << " using float by default.");
  m_ComponentType = FLOAT;
      }

    // Next short says whether RAS registration information is good. 
    // If so, read the voxel size and then the matrix
    TReadZ( fp, RASgood);
    float spacing;
    if ( RASgood )
      {
  for( int nSpacing = 0; nSpacing<3; ++nSpacing)
    {
      TReadZ( fp, spacing); // type is different
      m_Spacing[nSpacing] = spacing;
    }
  /*
    From http://www.nmr.mgh.harvard.edu/~tosa/#coords:
    To go from freesurfer voxel coordinates to RAS coordinates, they use:
    translation:  t_r, t_a, t_s is defined using c_r, c_a, c_s centre voxel position in RAS
    rotation: direction cosines x_(r,a,s), y_(r,a,s), z_(r,a,s)
    voxel size for scale: s_x, s_y, s_z
    
    [ x_r y_r z_r t_r][s_x  0   0  0]
    [ x_a y_a z_a t_a][0   s_y  0  0]
    [ x_s y_s z_s t_s][0    0  s_z 0]
    [  0   0   0   1 ][0    0   0  1]
    Voxel center is a column matrix, multipled from the right
    [v_x]
    [v_y]
    [v_z]
    [ 1 ]
    
    In the MGH header, they hold:
    x_r x_a x_s
    y_r y_a y_s
    z_r z_a z_s
    c_r c_a c_s
  */
  typedef itk::Matrix<double> MatrixType;
  MatrixType matrix;
  
  float fBuffer;
  float c[3];

  // reading in x_r x_a x_s y_r y_a y_s z_r z_a z_s and putting it into the
  // matrix as:
  // x_r y_r z_r
  // x_a y_a z_a
  // x_s y_s z_s
  for(unsigned int uj=0; uj<3; ++uj)
    {
      for(unsigned int ui=0; ui<3; ++ui)
        {
    TReadZ(fp, fBuffer);
    matrix[ui][uj] = fBuffer;
//    std::cout << "itkMGHImageIO ReadVolumeHeader: matrix[" << ui << "][" << uj << "] = " << matrix[ui][uj] << "\n";
        }
    }
  for(unsigned int ui=0; ui<3; ++ui)
  {
    TReadZ(fp, c[ui]);
  }

  std::string orientation = GetOrientation( matrix );
 
  // now take x_r, x_a, x_s out of the matrix and set it to the direction
  // vector 0, same for y_* and direction vector 1, z_* and vector 2
  for(unsigned int ui=0; ui<3; ++ui)
    {
      std::vector<double> vDir;
      // convert the coordinates from RAS to LPS, as the ITK archetype assumes
      // LPS volumes
      // volume orientation not related to scan order, always convert
      matrix[0][ui] *= -1.0; // R -> L
      matrix[1][ui] *= -1.0; // A -> P
      for(unsigned int uj=0; uj<3; ++uj)
      {
        vDir.push_back( matrix[uj][ui] );
      }
      //std::cout << "itkMGHImageIO ReadVolumeHeader: setting " << ui << " direction in LPS: " << vDir[0] << "," << vDir[1] << "," << vDir[2] << "\n";
      SetDirection( ui, vDir );
    }
  
  //MriDirCos(); // convert direction cosines

  // finally, store the origin of the image -> only works 
  // if the image is properly orriented in the sequel
  // 
  // computed in CORONAL orientation = ITK_COORDINATE_ORIENTATION_LIA
  float fcx,fcy,fcz;
  fcx = static_cast<float>(m_Dimensions[0])/2.0f;
  fcy = static_cast<float>(m_Dimensions[1])/2.0f;
  fcz = static_cast<float>(m_Dimensions[2])/2.0f;

  for( unsigned int ui=0; ui<3; ++ui)
    m_Origin[ui] = c[ui] -
      ( matrix[ui][0]*m_Spacing[0]*fcx +
        matrix[ui][1]*m_Spacing[1]*fcy +
        matrix[ui][2]*m_Spacing[2]*fcz );

      }
   
    //==================
    // read tags at the end of file

    unsigned long numValues = m_Dimensions[0]*m_Dimensions[1]*m_Dimensions[2];
    gzseek(fp, FS_WHOLE_HEADER_SIZE + 
     ( m_NumberOfComponents * numValues * this->GetComponentSize() ),
     SEEK_SET);

    float fBuf;
    // read TR, Flip, TE, FI, FOV
    if ( TReadZ( fp, fBuf) )
      {
  itk::MetaDataDictionary &thisDic = this->GetMetaDataDictionary();
  itk::EncapsulateMetaData<float>(thisDic,
         std::string("TR"),
         fBuf);

  // try to read flipAngle
  if ( TReadZ( fp, fBuf ) )
    {
      itk::EncapsulateMetaData<float>(thisDic,
             std::string("FlipAngle"),
             fBuf);
      // TE
      if ( TReadZ( fp, fBuf ) )
        {
    itk::EncapsulateMetaData<float>(thisDic,
           std::string("TE"),
           fBuf);
    // TI
    if ( TReadZ(fp, fBuf) )
      {
        itk::EncapsulateMetaData<float>(thisDic,
               std::string("TI"),
               fBuf);
        // FOV
        if ( TReadZ(fp, fBuf) )
          {
      itk::EncapsulateMetaData<float>(thisDic,
             std::string("FoV"),
             fBuf);
          }
      }
        }
    }
      }
  }

  void
  MGHImageIO::Read(void* pData)
  {
    gzFile fp;
    fp = gzopen( m_FileName.c_str(), "rb");
    if ( !fp )
      {
  itkExceptionMacro(<<"Can't find/open file: " << m_FileName);
  return;
      }

    const unsigned long numPixels = m_Dimensions[0]*m_Dimensions[1]* m_Dimensions[2];

    const unsigned int componentSize( this->GetComponentSize() );
    
    // check that the offset is actually computed wrt. the beginning
    gzseek(fp, FS_WHOLE_HEADER_SIZE, SEEK_SET );
    
    const unsigned int frameSize = numPixels*componentSize;

    if ( m_NumberOfComponents > 1  )
      {
  char* pBuffer = new char[ frameSize ];
  
  const unsigned int pixelSize = componentSize*m_NumberOfComponents;
  
  for(unsigned int frameIndex = 0;
      frameIndex < m_NumberOfComponents;
      ++frameIndex)
    {
      // read current frame
      gzread( fp, pBuffer, frameSize ); 
      // copy memory location in the final buffer
      
      char* pSrc = (char*)pBuffer;
      char* pDst = (char*)pData;
      
      pDst += frameIndex * componentSize;
      for( unsigned int ui=0;
     ui < numPixels; 
     ++ui, pSrc+=componentSize, pDst+=pixelSize)
        {
    for(unsigned int byteCount = 0;
        byteCount < componentSize; ++byteCount)
      *(pDst+byteCount) = *(pSrc+byteCount);
        } // next ui
    } // next frameIndex

  // clear resources
  delete[] pBuffer;
      }
    else
      {
  gzread( fp, pData, frameSize);
      }
    
    gzclose(fp);
    
    SwapBytesIfNecessary( pData, numPixels*m_NumberOfComponents );

  } // end Read function

  void
  MGHImageIO::SwapBytesIfNecessary(void* buffer,
           unsigned long numberOfPixels)
  {
    // NOTE: If machine order is little endian, and the data needs to be
    // swapped, the SwapFromBigEndianToSystem is equivalent to
    // SwapFromSystemToBigEndian.

    switch(m_ComponentType)
      {
      case UCHAR:
  ByteSwapper<unsigned char>::SwapRangeFromSystemToBigEndian((unsigned char*)buffer,
                   numberOfPixels);
  break;
      case SHORT:
  ByteSwapper<short>::SwapRangeFromSystemToBigEndian((short*)buffer,
                 numberOfPixels);
  break;
      case INT:
  ByteSwapper<int>::SwapRangeFromSystemToBigEndian((int*)buffer,
               numberOfPixels);
  break;
      case FLOAT:
  ByteSwapper<float>::SwapRangeFromSystemToBigEndian((float*)buffer,
                 numberOfPixels);
  break;
      default:
  ExceptionObject exception(__FILE__,__LINE__);
  exception.SetDescription("Pixel Type Unknown");
  throw exception;
      } // end switch
  }


  bool
  MGHImageIO::CanWriteFile(const char* name)
  {
    std::string filename(name);

    if ( filename == "" )
      {
  itkExceptionMacro(<<"A FileName must be specified.");
  return false;
      }

    std::string extension = GetExtension(filename);
    if ( extension != std::string("mgh") &&
   extension != std::string("mgz") )
      return false;

    return true;
  }

  void
  MGHImageIO::WriteImageInformation()
  {
    std::string extension = GetExtension(m_FileName);

    if ( extension == std::string("mgh") )
      WriteUncompressedHeader();
    else
      {
  gzFile fp = gzopen(m_FileName.c_str(), "wb");
  if(!fp)
    itkExceptionMacro(<<" Failed to open gzFile for writing");
  WriteCompressedHeader(fp);
  gzclose(fp);
      }
  }
  

  void
  MGHImageIO::Write(const void* buffer)
  {
    // Write the image information before writing data

    std::string extension = GetExtension(m_FileName);
    if ( extension == std::string("mgh") )
      {
  this->WriteUncompressedHeader();
  this->WriteUncompressedData(buffer);
      }
    else
      {
  gzFile file_p = gzopen( m_FileName.c_str(), "wb");
  if (!file_p)
    itkExceptionMacro(<<" Failed to open gzFile for writing");

  this->WriteCompressedHeader(file_p);
  this->WriteCompressedData(file_p, buffer);
  
  gzclose(file_p);
      }

  }

  void
  MGHImageIO::WriteUncompressedHeader()
  {
    const int mghVersion = 1;

    std::ofstream ofs;
    ofs.open( m_FileName.c_str(),
        std::ios::out | std::ios::binary );
    if ( ofs.fail() )
      {
  itkExceptionMacro(<< " File cannot be written");
      }
    
    // version
    TWrite( ofs, mghVersion );

    // dimensions
    for(unsigned int ui=0; ui<3; ++ui)
      TWrite( ofs, (int)m_Dimensions[ui] );
    
    // nframes
    TWrite( ofs, (int)m_NumberOfComponents );

    // type
    switch( m_ComponentType )
      {
      case UCHAR:
  TWrite( ofs, MRI_UCHAR); break;
      case INT:
  TWrite( ofs, MRI_INT); break;
      case FLOAT:
  TWrite( ofs, MRI_FLOAT); break;
      case SHORT:
  TWrite( ofs, MRI_SHORT); break;
      }

    // dof !?! -> default value = 1
    TWrite( ofs, 1);
    
    // write RAS and voxel size info
    // for now, RAS flag will be good
    // in the future, check if the m_Directions matrix is a permutation matrix
    TWrite( ofs, (short)1);
    
    // spacing
    for(unsigned int ui=0; ui<3; ++ui)
      TWrite( ofs, (float)m_Spacing[ui]);
         
    // get directions matrix
    std::vector<std::vector<double> > vvRas;
    for(unsigned int ui=0; ui<3; ++ui) vvRas.push_back( GetDirection(ui) );
    // transpose data before writing it
    std::vector<float> vBufRas;
    // transpose the matrix
    for(unsigned int ui(0), count(0);
  ui < 3; ++ui)
      for(unsigned int uj(0); uj<3; ++uj)
  vBufRas.push_back( (float)vvRas[uj][ui] );
    
    for(std::vector<float>::const_iterator cit = vBufRas.begin();
  cit != vBufRas.end(); ++cit )
      TWrite( ofs, *cit);
    
    // write c_r, c_a, c_s
    // defined as origin + DC x resolution x ( dim0/2 , dim1/2, dim2/2 )
    float crasBuf;
    for(unsigned int ui=0; ui<3; ++ui)
      {
  crasBuf = m_Origin[ui];
  for(unsigned int uj=0; uj<3; ++uj)
    crasBuf += vvRas[ui][uj]*m_Spacing[uj]*(float)m_Dimensions[uj]/2.0f;

  TWrite( ofs, crasBuf );
      } // next ui
    
    // fill the rest of the buffer with zeros
    char* buffer = new char[ FS_UNUSED_HEADER_SIZE *sizeof(char) ];
    memset(buffer, 0, FS_UNUSED_HEADER_SIZE *sizeof(char) );
    ofs.write( buffer, FS_UNUSED_HEADER_SIZE );
    delete[] buffer;

    // the stream is automatically closed when quiting
  }

  void
  MGHImageIO::WriteCompressedHeader(gzFile file_p)
  {
    const int mghVersion = 1;

    // version
    TWriteZ( file_p, mghVersion);

    // dimensions
    for(unsigned int ui=0; ui<3; ++ui)
      TWriteZ( file_p, (int)m_Dimensions[ui]);
    
    // nframes
    TWriteZ( file_p, (int)m_NumberOfComponents);

    // type
    switch( m_ComponentType )
      {
      case UCHAR:
  TWriteZ( file_p, MRI_UCHAR); break;
      case INT:
  TWriteZ( file_p, MRI_INT); break;
      case FLOAT:
  TWriteZ( file_p, MRI_FLOAT); break;
      case SHORT:
  TWriteZ( file_p, MRI_SHORT); break;
      } 

    // dof -> default value = 1
    TWriteZ( file_p, 1 );

    // write RAS and voxel size info
    // good ras flag
    TWriteZ( file_p, (short)1 );

    // write the spacing
    for(unsigned int ui=0; ui<3; ++ui)
      TWriteZ( file_p, (float)m_Spacing[ui]);

    //===================================================================
    // get directions matrix
    std::vector<std::vector<double> > vvRas;
    for(unsigned int ui=0; ui<3; ++ui) vvRas.push_back( GetDirection(ui) );
    // transpose data before writing it
    std::vector<float> vBufRas;
    // transpose the matrix
    for(unsigned int ui(0), count(0);
  ui < 3; ++ui)
      for(unsigned int uj(0); uj<3; ++uj)
  vBufRas.push_back( (float)vvRas[uj][ui] );
    //===================================================================

    for(std::vector<float>::const_iterator cit = vBufRas.begin();
  cit != vBufRas.end(); ++cit )
      TWriteZ( file_p, *cit );

    // write c_r, c_a, c_s
    float crasBuf;
    for(unsigned int ui=0; ui<3; ++ui)
      {
  crasBuf = m_Origin[ui];
  for(unsigned int uj=0; uj<3; ++uj)
    crasBuf += vvRas[ui][uj]*m_Spacing[uj]*(float)m_Dimensions[uj]/2.0f;
  
  TWriteZ( file_p, crasBuf );
      }
    
    // fill the rest of the buffer with zeros
    char* buffer = new char[ FS_UNUSED_HEADER_SIZE *sizeof(char) ];
    memset(buffer, 0, FS_UNUSED_HEADER_SIZE *sizeof(char) );
    ::gzwrite( file_p, buffer, FS_UNUSED_HEADER_SIZE *sizeof(char) );

    // free resources
    delete[] buffer;

  }

  void
  MGHImageIO::WriteCompressedData(gzFile fp,
          const void* buffer)
  {

    // swap bytes if necessary
    const unsigned long int numvalues = m_Dimensions[0]
      * m_Dimensions[1] * m_Dimensions[2] * m_NumberOfComponents;
    const unsigned long int numbytes = this->GetComponentSize() * numvalues;

    char* tempmemory = new char[numbytes];

    // re-arrange data in frames
    if ( m_NumberOfComponents > 1 )
      {
  PermuteFrameValues(buffer, tempmemory);
      }
    else
      memcpy(tempmemory, buffer, numbytes);


    this->SwapBytesIfNecessary(tempmemory,numvalues);
    
    int result = gzwrite( fp, tempmemory, numbytes );

    delete[] tempmemory;

        itk::MetaDataDictionary &thisDic=this->GetMetaDataDictionary();
    
    float fScanBuffer;
    
    if ( ExposeMetaData<float>(thisDic, "TR", fScanBuffer) )
      {
  TWriteZ( fp, fScanBuffer );
  
  if ( ExposeMetaData<float>(thisDic, "FlipAngle", fScanBuffer) )
    {
      TWriteZ( fp, fScanBuffer);
      
      if ( ExposeMetaData<float>(thisDic, "TE", fScanBuffer) )
        {
    TWriteZ( fp, fScanBuffer);
    
    if ( ExposeMetaData<float>(thisDic, "TI", fScanBuffer) )
      {
        TWriteZ( fp, fScanBuffer);

        if ( ExposeMetaData<float>(thisDic, "FoV", fScanBuffer) )
          TWriteZ( fp, fScanBuffer);
      } // end TI
        } // end TE
    } // end FlipAngle
      } // end TR

  }

  void
  MGHImageIO::WriteUncompressedData(const void* buffer)
  {
    std::ofstream ofs;
    ofs.open(m_FileName.c_str(), std::ios::out | std::ios::binary | std::ios::app );
    if ( ofs.fail() )
      {
  itkExceptionMacro( << " failed to open stream for writing uncompressed data");
      }
    // no need to jump - just append at the end of the stream 

    // swap bytes if necessary
    const unsigned int numPixels =  m_Dimensions[0]
      * m_Dimensions[1] * m_Dimensions[2];
    const unsigned long int numvalues = numPixels * m_NumberOfComponents;
    const unsigned long int numbytes = this->GetComponentSize() * numvalues;

    char* tempmemory = new char[numbytes];

    // re-arrange data in frames
    if ( m_NumberOfComponents > 1 )
      {
  PermuteFrameValues(buffer, tempmemory);
      }
    else
      memcpy(tempmemory, buffer, numbytes);

    this->SwapBytesIfNecessary(tempmemory, numvalues);

    ofs.write( static_cast<const char*>(tempmemory), this->GetImageSizeInBytes() );
    delete[] tempmemory;

    // if present, the scan parameters are present at the end of the file, so now's the time to write them
    itk::MetaDataDictionary &thisDic=this->GetMetaDataDictionary();
    
    float fScanBuffer;
    
    if ( ExposeMetaData<float>(thisDic, "TR", fScanBuffer) )
      {
  TWrite( ofs, fScanBuffer );
  
  if ( ExposeMetaData<float>(thisDic, "FlipAngle", fScanBuffer) )
    {
      TWrite( ofs, fScanBuffer);
      
      if ( ExposeMetaData<float>(thisDic, "TE", fScanBuffer) )
        {
    TWrite( ofs, fScanBuffer);
    
    if ( ExposeMetaData<float>(thisDic, "TI", fScanBuffer) )
      {
        TWrite( ofs, fScanBuffer);

        if ( ExposeMetaData<float>(thisDic, "FoV", fScanBuffer) )
          TWrite( ofs, fScanBuffer);
      } // end TI
        } // end TE
    } // end FlipAngle
      } // end TR


    // no need to close the stream
  }

  void
  MGHImageIO::PermuteFrameValues(const void* buffer,
         char* tempmemory)
  {
    const unsigned int numPixels =  m_Dimensions[0]
      * m_Dimensions[1] * m_Dimensions[2];
    const unsigned long int numvalues = numPixels * m_NumberOfComponents;
    const unsigned long int numbytes = this->GetComponentSize() * numvalues;
    

    const unsigned int valueSize( this->GetComponentSize() );

    const unsigned int frameSize = numPixels * valueSize;
    
    const char* pSrc = (const char*)buffer;
    char* pDst = (char*)tempmemory;
    
    for(unsigned int pixelIndex = 0;
        pixelIndex < numPixels; ++pixelIndex, pDst+=valueSize )
      {
      for(unsigned int componentIndex =0 ;
          componentIndex < m_NumberOfComponents;
          ++componentIndex, pSrc+=valueSize)
        {
        std::copy( pSrc, pSrc+ valueSize,
                   pDst + frameSize*componentIndex );
        } // next component index
      } // next pixelIndex 
  }

  unsigned int
  MGHImageIO::GetComponentSize() const
  {
    unsigned int returnValue;
    switch ( m_ComponentType )
      {
      case UCHAR: returnValue = sizeof(unsigned char); break;
      case SHORT: returnValue = sizeof(short); break;
      case INT:   returnValue = sizeof(int); break;
      case FLOAT: returnValue = sizeof(float); break;
      }
    return returnValue;
  }
  
  /**
   * Examines the direction cosines and creates an Orientation String. 
   * The Orientation String is a three character string indicating the primary 
   * direction of each axis in the 3d matrix. The characters can be L,R,A,P,I,S.
   **/
  std::string 
  MGHImageIO::GetOrientation( itk::Matrix< double > directions )
  {
    
    std::string orientation = "";
  
    for(int cAxes=0; cAxes<3; cAxes++)
      {
      const double sag = directions( 0, cAxes ); // LR axis
      const double cor = directions( 1, cAxes ); // PA axis
      const double ax  = directions( 2, cAxes ); // IS axis
      if(fabs(sag) > fabs(cor) &&
         fabs(sag) > fabs(ax))
        {
        if(sag > 0)
          {
          orientation += "R";
          }
        else
          {
          orientation += "L";
          }
        continue;
        }
      if(fabs(cor) > fabs(ax))
        {
        if(cor > 0)
          {
          orientation += "A";
          }
        else
          {
          orientation += "P";
          }
        continue;
        }
      if(ax > 0)
        {
        orientation += "S";
        }
      else
        {
        orientation += "I";
        }
    }
    //std::cout << "GetOrientation returning " << orientation.c_str() << std::endl;
    return orientation;
  }    

} // end NAMESPACE ITK


