

#include <itkTimeSeriesDatabase.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <itksys/SystemTools.hxx>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itksys/SystemTools.hxx>
#include "itkArchetypeSeriesFileNames.h"
#include <fstream>
#include <vector>

namespace itk {

  // template<class TPixel> int TimeSeriesDatabase<TPixel>::BlockSize = 16;
  template<class T> T TSD_MIN ( T a, T b ) { return a < b ? a : b; }
  template<class T> T TSD_MAX ( T a, T b ) { return a > b ? a : b; }

  
template <class TPixel>
bool TimeSeriesDatabase<TPixel>::CalculateIntersection ( Size<3> BlockIndex, 
                                                         typename OutputImageType::RegionType RequestedRegion, 
                                                         typename OutputImageType::RegionType& BlockRegion,
                                                         typename OutputImageType::RegionType& ImageRegion )
{

  // Calculate the intersection between the block at BlockIndex and the Requested Region
  bool IsFullBlock = true;
  for ( unsigned int i = 0; i < 3; i++ )
  {
    ImageRegion.SetIndex ( i, TSD_MAX ( (long unsigned int) RequestedRegion.GetIndex ( i ), TimeSeriesBlockSize * BlockIndex[i] ) );
    BlockRegion.SetIndex ( i, ImageRegion.GetIndex(i) % TimeSeriesBlockSize );

    // This is the end index
    long unsigned int Tmp = RequestedRegion.GetIndex ( i ) + RequestedRegion.GetSize ( i );
    Tmp = TSD_MIN ( (long unsigned int) Tmp, TimeSeriesBlockSize * (BlockIndex[i]+1) );
    Tmp = Tmp - ImageRegion.GetIndex(i);

    ImageRegion.SetSize ( i, Tmp );
    BlockRegion.SetSize ( i, Tmp );
    IsFullBlock = IsFullBlock & ( Tmp == TimeSeriesBlockSize );
  }
  return IsFullBlock;    
}


  
template <class TPixel>
bool TimeSeriesDatabase<TPixel>::IsOpen () const
{
  if ( this->m_DatabaseFiles.size() == 0 ) { return false; }
  return const_cast<std::fstream*>(this->m_DatabaseFiles[0].get())->is_open();
}

template <class TPixel>
void TimeSeriesDatabase<TPixel>::Disconnect ()
{
  for ( int idx = 0; idx < this->m_DatabaseFiles.size(); idx++ )
    {
    this->m_DatabaseFiles[idx]->close();
    }
  this->m_DatabaseFiles.clear();
  this->m_DatabaseFileNames.clear();
}
  
template <class TPixel>
void TimeSeriesDatabase<TPixel>::Connect ( const char* filename )
{
  // If we are still open, disconnect
  if ( this->IsOpen() )
    {
    this->Disconnect();
    }
  // Open and make sure we have the correct header!
  this->m_Filename = filename;
  ::std::fstream db ( this->m_Filename.c_str(), ::std::ios::in | ::std::ios::binary );
  // Read the first bits.
  char* buffer = new char[TimeSeriesVolumeBlockSize*sizeof(TPixel)];
  db.read ( buffer, TimeSeriesVolumeBlockSize * sizeof ( TPixel ) );
  db.close();
  // Associate it with a string
  std::string s ( buffer );
  delete[] buffer;
  ::std::istringstream o ( s );
  ::std::string foo;
  float version;
  o >> foo >> foo >> version;
  if ( version != 1.0 )
  {
    itkExceptionMacro ( "TimeSeriesDatabase::Connect: Version string does not match.  Expecting 1.0, found " << version );
  }
  // Start reading our data
  std::string dummy;
  Size<3> sz;
  o >> dummy >> m_Dimensions[0] >> m_Dimensions[1] >> m_Dimensions[2] >> m_Dimensions[3];
  o >> dummy >> sz[0] >> sz[1] >> sz[2];
  m_OutputRegion.SetSize ( sz );
  o >> dummy >> m_OutputOrigin[0] >> m_OutputOrigin[1] >> m_OutputOrigin[2];
  o >> dummy >> m_OutputSpacing[0] >> m_OutputSpacing[1] >> m_OutputSpacing[2];
  o >> dummy;
  for ( unsigned int i = 0; i < m_OutputDirection.GetVnlMatrix().rows(); i++ )
    {
    for ( unsigned int j = 0; j < m_OutputDirection.GetVnlMatrix().cols(); j++ )
      {
      o >> m_OutputDirection[i][j];
      }
    }
  for ( int idx = 0; idx < 3; idx++ )
    {
    m_BlocksPerImage[idx] = (unsigned int) ceil ( m_Dimensions[idx] / (float)TimeSeriesBlockSize );
    }
  // Number of files
  o >> dummy >> this->m_BlocksPerFile;
  int NumberOfFiles;
  o >> dummy >> NumberOfFiles;
  // Read the "Filenames:" line
  o >> dummy;
  this->m_DatabaseFiles.clear();
  this->m_DatabaseFileNames.clear();
  // Read and open the files
  for ( int idx = 0; idx < NumberOfFiles; idx++ )
    {
    std::string Filename;
    o >> Filename;
    // std::cout << "Reading file " << idx << " " << Filename << std::endl;
    this->m_DatabaseFileNames.push_back ( Filename );
    this->m_DatabaseFiles.push_back ( StreamPtr ( new std::fstream ( Filename.c_str(), ::std::ios::in | ::std::ios::binary ) ) );
    }
  /*
  std::cout << "ImageSize: " << m_OutputRegion.GetSize() << endl;
  std::cout << "ImageOrigin: " << m_OutputOrigin << endl;
  std::cout << "ImageSpacing: " << m_OutputSpacing << endl;
  std::cout << "Direction: " << m_OutputDirection << endl;
  */
}

  
template <class TPixel>
unsigned int TimeSeriesDatabase<TPixel>::CalculateFileIndex ( unsigned long Index, unsigned long BlocksPerFile )
{
  return (int) Index / BlocksPerFile;
}

template <class TPixel>
unsigned int TimeSeriesDatabase<TPixel>::CalculateFileIndex ( unsigned long index )
{
  return this->CalculateFileIndex ( index, this->m_BlocksPerFile );
}


template <class TPixel>
unsigned long TimeSeriesDatabase<TPixel>::CalculateIndex ( Size<3> p, int ImagePosition, unsigned int BlocksPerImage[3] )
{
  // Remember that we use the first 16K as our header
  unsigned long index = 1 + p[0] 
    + p[1] * BlocksPerImage[0]
    + p[2] * BlocksPerImage[0] * BlocksPerImage[1]
    + ImagePosition * BlocksPerImage[0] * BlocksPerImage[1] * BlocksPerImage[2];
  return index;
}
  

template <class TPixel>
unsigned long TimeSeriesDatabase<TPixel>::CalculateIndex ( Size<3> p, int ImagePosition )
{
  unsigned int t[3];
  t[0] = this->m_BlocksPerImage[0]; 
  t[1] = this->m_BlocksPerImage[1]; 
  t[2] = this->m_BlocksPerImage[2]; 
  return this->CalculateIndex ( p, ImagePosition, t );
}

template <class TPixel>
::std::streampos TimeSeriesDatabase<TPixel>::CalculatePosition ( unsigned long index, unsigned long BlocksPerFile )
{
  ::std::streampos position = ( index % BlocksPerFile ) * sizeof ( TPixel ) * TimeSeriesVolumeBlockSize;
  return position;
}


template <class TPixel>
typename TimeSeriesDatabase<TPixel>::CacheBlock* TimeSeriesDatabase<TPixel>::GetCacheBlock ( unsigned long index )
{
  CacheBlock* Buffer = this->m_Cache.find ( index );
  if ( Buffer == 0 ) {
    // Fill it in
    CacheBlock B;
    int FileIdx = this->CalculateFileIndex ( index );
    
    this->m_DatabaseFiles[FileIdx]->seekg ( this->CalculatePosition ( index, this->m_BlocksPerFile ) );
    this->m_DatabaseFiles[FileIdx]->read ( reinterpret_cast<char*> ( B.data ), TimeSeriesVolumeBlockSize * sizeof ( TPixel ) );
    this->m_Cache.insert ( index, B );
    Buffer = this->m_Cache.find ( index );
  }
  return Buffer;
}


template <class TPixel>
void TimeSeriesDatabase<TPixel>::GetVoxelTimeSeries ( typename OutputImageType::IndexType idx, ArrayType& array )
{
  // See if the index is inside the volume
  // and figure out which cache block we need
  Size<3> CurrentBlock;
  Size<3> Offset;
  for ( int i = 0; i < 3; i++ ) {
    if ( idx[i] < 0 || idx[i] > this->m_OutputRegion.Size[i] ) {
      throw 1;
    }
    CurrentBlock[i] = reinterpret_cast<unsigned long> ( idx[i] / (double)TimeSeriesBlockSize );
    Offset[i] = idx[i] % TimeSeriesBlockSize;
  }
  unsigned long offset = Offset[0] + Offset[1] * TimeSeriesBlockSize + Offset[2] * TimeSeriesBlockSizeP2;
  array = ArrayType ( this->m_Dimensions[3] );
  for ( int volume = 0; volume < this->m_Dimensions[3]; volume++ ) {
    CacheBlock* cache = this->GetCacheBlock ( CurrentBlock, volume );
    array[volume] = cache->data[offset];
  }
}


template <class TPixel>
void TimeSeriesDatabase<TPixel>::GenerateOutputInformation ( ) 
{
  typename OutputImageType::Pointer output = this->GetOutput();
  output->SetSpacing ( this->m_OutputSpacing );     
  output->SetOrigin ( this->m_OutputOrigin );       
  output->SetDirection ( this->m_OutputDirection ); 
  output->SetLargestPossibleRegion ( this->m_OutputRegion );
}  

template <class TPixel>
void TimeSeriesDatabase<TPixel>::GenerateData() 
{
  typename OutputImageType::Pointer output = this->GetOutput();
  typename OutputImageType::RegionType Region;
  itkDebugMacro ( << "TimeSeriesDatabase::GenerateData()  Allocating" );
  Region = output->GetRequestedRegion();
  output->SetBufferedRegion ( Region );
  output->Allocate();

  if ( !this->IsOpen() )
  {
    itkGenericExceptionMacro ( "TimeSeriesDatabase::GenerateOutputInformation: not open for reading" );
  }

  Size<3> BlockStart, BlockCount;
  for ( unsigned int i = 0; i < 3; i++ ) {
    BlockStart[i] = (int) floor ( Region.GetIndex(i) / (double)TimeSeriesBlockSize );
    BlockCount[i] = (int) TSD_MAX ( 1.0, ceil ( (Region.GetIndex(i)+Region.GetSize(i)) / (double)TimeSeriesBlockSize ) - BlockStart[i] );
  }

  bool print = Region.GetIndex(0) == 0 && Region.GetIndex(1) == 0 && Region.GetIndex(2) == 80;
  print = false;
  if ( print ) 
    {
    std::cout << "GenerateData for " << output->GetRequestedRegion() << std::endl;
    output->Print ( std::cout );
    }

  Size<3> CurrentBlock;
  // Now, read our data, caching as we go, in future, make this thread safe
  Size<3> BlockSize = { TimeSeriesBlockSize, TimeSeriesBlockSize, TimeSeriesBlockSize };
  ImageRegion<3> BlockRegion;
  BlockRegion.SetSize ( BlockSize );
  // Fetch only the blocks we need
  for ( CurrentBlock[2] = BlockStart[2]; CurrentBlock[2] < BlockStart[2] + BlockCount[2]; CurrentBlock[2]++ ) {
    for ( CurrentBlock[1] = BlockStart[1]; CurrentBlock[1] < BlockStart[1] + BlockCount[1]; CurrentBlock[1]++ ) {
      for ( CurrentBlock[0] = BlockStart[0]; CurrentBlock[0] < BlockStart[0] + BlockCount[0]; CurrentBlock[0]++ ) {
        typename OutputImageType::RegionType BR, IR;
        if ( print ) {  std::cout << "For Block Index: " << CurrentBlock << std::endl; }
        unsigned long index = this->CalculateIndex ( CurrentBlock, this->m_CurrentImage );
        CacheBlock* Buffer = this->GetCacheBlock ( index );
        if ( this->CalculateIntersection ( CurrentBlock, Region, BR, IR ) ) {
          // Just iterate over whole block
          // Good we can use an iterator!
          if ( print ) {
            std::cout << "The easy way! " << std::endl;
            std::cout << "Block Region: " << BR;
            std::cout << "Image Region: " << IR;
          }
          Index<3> BlockIndex = { CurrentBlock[0] * TimeSeriesBlockSize,  CurrentBlock[1] * TimeSeriesBlockSize,  CurrentBlock[2] * TimeSeriesBlockSize };
          BlockRegion.SetIndex ( BlockIndex );
          ImageRegionIterator<OutputImageType> it ( output, IR );
          it.GoToBegin();
          TPixel* ptr = Buffer->data;
          while ( !it.IsAtEnd() ) {
            it.Set ( *ptr );
            ++it;
            ++ptr;
          }
        } else {
          // Now we do it the hard way...
          Index<3> ImageIndex;
          Size<3> Count = BR.GetSize();
          if ( print ) {
            std::cout << "The Hard way" << std::endl;
            std::cout << "Count: " << Count << std::endl;
            std::cout << "Block Region: " << BR;
            std::cout << "Image Region: " << IR;
            std::cout << "First voxel: " << Buffer->data[0] << std::endl;
          }
          int bx, by, bz, x, y, z;
          for ( z = 0; z < Count[2]; z++ ) {
            ImageIndex[2] = IR.GetIndex(2) + z;
            bz = BR.GetIndex(2) + z;
            for ( y = 0; y < Count[1]; y++ ) {
              ImageIndex[1] = IR.GetIndex(1) + y;
              by = BR.GetIndex(1) + y;
              for ( x = 0; x < Count[0]; x++ ) {
                ImageIndex[0] = IR.GetIndex(0) + x;
                bx = BR.GetIndex(0) + x;
                /*
                int BufferIndex = bx + TimeSeriesBlockSize*by + TimeSeriesBlockSize*TimeSeriesBlockSize*bz;
                if ( ImageIndex[0] == 45 && ImageIndex[1] == 0 && ImageIndex[2] == 0 ) {
                  std::cout << "Index: " << ImageIndex << " Volume Value: " << output->GetPixel ( ImageIndex ) << " buffer: " << Buffer->data[BufferIndex] << std::endl;
                std::cout << "Index: " << ImageIndex << " From " << BufferIndex << " ( " << bx << ", " << by << ", " << bz << " )\n" << std::endl;
                }
                */

                output->SetPixel ( ImageIndex, Buffer->data[bx + TimeSeriesBlockSize*by + TimeSeriesBlockSize*TimeSeriesBlockSize*bz] );
                }
              }
            }
          }
        }
      }
    }
  
  return;
}
  

template <class TPixel>
void TimeSeriesDatabase<TPixel>::CreateFromFileArchetype ( const char* TSDFilename, const char* archetype )
{
  // How many blocks go in each file, allowing for 1GiB per file
  // 1073741824 is 1 GiB
  unsigned long FileSize = 1073741824;
  CreateFromFileArchetype ( TSDFilename, archetype, FileSize );
}

template <class TPixel>
void TimeSeriesDatabase<TPixel>::CreateFromFileArchetype ( const char* TSDFilename, const char* archetype, unsigned long FileSize )
{

  unsigned long BlocksPerFile = FileSize / ( TimeSeriesVolumeBlockSize * sizeof ( TPixel ) );

  std::vector<std::string> candidateFiles;
  std::string fileNameCollapsed = itksys::SystemTools::CollapseFullPath( archetype);
  if (!itksys::SystemTools::FileExists (fileNameCollapsed.c_str()))  {
      itkGenericExceptionMacro ( "TimeSeriesDatabase::CreateFromFileArchetype: Archetype file " << fileNameCollapsed.c_str() << " does not exist.");
      return;
    }
  ArchetypeSeriesFileNames::Pointer fit = itk::ArchetypeSeriesFileNames::New();
  fit->SetArchetype ( fileNameCollapsed );
  
  typedef Image<TPixel,3> ImageType;
  typedef ImageFileReader<ImageType> ReaderType;
  ImageRegion<3> region;

  // Load the first image's size
  candidateFiles = fit->GetFileNames();
  typename ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName ( fileNameCollapsed );
  reader->UpdateOutputInformation();
  region = reader->GetOutput()->GetLargestPossibleRegion();
  Array<unsigned int> m_Dimensions ( 4 );

  m_Dimensions[0] = region.GetSize()[0];
  m_Dimensions[1] = region.GetSize()[1];
  m_Dimensions[2] = region.GetSize()[2];
  m_Dimensions[3] = candidateFiles.size();

  typename OutputImageType::SpacingType m_OutputSpacing;
  typename OutputImageType::RegionType m_OutputRegion;
  typename OutputImageType::PointType m_OutputOrigin;
  typename OutputImageType::DirectionType m_OutputDirection;

  m_OutputRegion = reader->GetOutput()->GetLargestPossibleRegion();
  m_OutputSpacing = reader->GetOutput()->GetSpacing();
  m_OutputOrigin = reader->GetOutput()->GetOrigin();
  m_OutputDirection = reader->GetOutput()->GetDirection();


  // Make our array, and open it
  std::vector<StreamPtr> db;
  db.push_back ( StreamPtr ( new std::fstream ( TSDFilename, ::std::ios::out | ::std::ios::binary ) ) );

  // Filenames
  std::vector<std::string> Filenames;
  Filenames.push_back ( std::string ( TSDFilename ) );

  // Start reading and writing out the images, 16x16x16 blocks at a time.
  for ( unsigned int i = 0; i < candidateFiles.size(); i++ )
    {
    reader->SetFileName ( itksys::SystemTools::CollapseFullPath ( candidateFiles[i].c_str() ) );
    try {
      reader->Update();
    } catch ( ExceptionObject& e ) {
      itkGenericExceptionMacro ( << "Failed to read " << candidateFiles[i] << " caught " << e );
    }
    // Verify that we have the same size as expected
    region = reader->GetOutput()->GetLargestPossibleRegion();
    if ( m_Dimensions[0] != region.GetSize()[0]
         || m_Dimensions[1] != region.GetSize()[1]
         || m_Dimensions[2] != region.GetSize()[2] ) {

      // close them all
    for ( int idx = 0; idx < db.size(); idx++ )
      {
      db[idx]->close();
      }
    itkGenericExceptionMacro ( << " size of the data in " << candidateFiles[i] << " is (" 
                               << region.GetSize()[0] << ", "
                               << region.GetSize()[1] << ", "
                               << region.GetSize()[2] << ") "
                               << " and does not match the expectected size ("
                               << m_Dimensions[0] << ", "
                               << m_Dimensions[1] << ", "
                               << m_Dimensions[2] << ")" );
    }
    
    // Build and write our blocks
    TPixel buffer[TimeSeriesBlockSize*TimeSeriesBlockSize*TimeSeriesBlockSize];
    Size<3> BlockSize = { TimeSeriesBlockSize, TimeSeriesBlockSize, TimeSeriesBlockSize };
    ImageRegion<3> BlockRegion;
    unsigned int m_BlocksPerImage[3];
    
    BlockRegion.SetSize ( BlockSize );
    for ( int idx = 0; idx < 3; idx++ )
      {
      m_BlocksPerImage[idx] = (unsigned int) ceil ( m_Dimensions[idx] / (float)TimeSeriesBlockSize );
      }
    Size<3> CurrentBlock;
    for ( CurrentBlock[2] = 0; CurrentBlock[2] < m_BlocksPerImage[2]; CurrentBlock[2]++ )
      {
      for ( CurrentBlock[1] = 0; CurrentBlock[1] < m_BlocksPerImage[1]; CurrentBlock[1]++ )
        {
        for ( CurrentBlock[0] = 0; CurrentBlock[0] < m_BlocksPerImage[0]; CurrentBlock[0]++ )
          {
          /*
          std::cout << "Reading/Writing Block: " << CurrentBlock[0] << ", " << CurrentBlock[1] << ", " << CurrentBlock[2] << endl;
          std::cout << "Debug: " << ( ( CurrentBlock[2] * TimeSeriesBlockSize + TimeSeriesBlockSize ) < region.GetSize()[2] ) << endl;
          std::cout << "Debug: " << CurrentBlock[2] * TimeSeriesBlockSize + TimeSeriesBlockSize << " Region size  " << region.GetSize()[2] << endl;
          */
          // Load up the block, and save it at the proper index
          // Is this block fully within the image?
          if ( ( ( CurrentBlock[0] * TimeSeriesBlockSize + TimeSeriesBlockSize ) < region.GetSize()[0] )
               && ( ( CurrentBlock[1] * TimeSeriesBlockSize + TimeSeriesBlockSize ) < region.GetSize()[1] )
               && ( ( CurrentBlock[2] * TimeSeriesBlockSize + TimeSeriesBlockSize ) < region.GetSize()[2] ) )
            {
            // Good we can use an iterator!
            Index<3> BlockIndex = { CurrentBlock[0] * TimeSeriesBlockSize,  CurrentBlock[1] * TimeSeriesBlockSize,  CurrentBlock[2] * TimeSeriesBlockSize };
            BlockRegion.SetIndex ( BlockIndex );
            ImageRegionIteratorWithIndex<ImageType> it ( reader->GetOutput(), BlockRegion );
            it.GoToBegin();
            TPixel* ptr = buffer;
            while ( !it.IsAtEnd() )
              {
              *ptr = it.Value();
              ++it;
              ++ptr;
              }
            } 
          else
            {
            // cout << "The Hard way" << std::endl;
            // Now we do it the hard way...
            Index<3> BlockIndex;
            Size<3> StartIndex, EndIndex;
            for ( int i = 0; i < 3; i++ ) 
              {
              StartIndex[i] = CurrentBlock[i]*TimeSeriesBlockSize;
              EndIndex[i] = TSD_MIN ( StartIndex[i] + TimeSeriesBlockSize, region.GetSize()[i] );
              }            
            for ( int bz = StartIndex[2]; bz < EndIndex[2]; bz++ ) 
              {
              BlockIndex[2] = bz;
              for ( int by = StartIndex[1]; by < EndIndex[1]; by++ ) 
                {
                BlockIndex[1] = by;
                for ( int bx = StartIndex[0]; bx < EndIndex[0]; bx++ ) 
                  {
                  // Put bx,by,bz into bx-xoff,by-yoff,bz-zoff
                  BlockIndex[0] = bx;
                  TPixel value = reader->GetOutput()->GetPixel ( BlockIndex );
                  buffer[bx-StartIndex[0] + TimeSeriesBlockSize*(by-StartIndex[1]) + TimeSeriesBlockSize*TimeSeriesBlockSize*(bz-StartIndex[2])] = value;
                  }
                }
              }
            }
          // Calculate where to write...  This code is copied from CalculatePosition and CalculateIndex
          unsigned long index = CalculateIndex ( CurrentBlock, i, m_BlocksPerImage );
          // Adjust the position, based on the FileIndex
          ::std::streampos position = CalculatePosition ( index, BlocksPerFile );
          unsigned long FileIndex = CalculateFileIndex ( index, BlocksPerFile );
          // std::cout << "Found FileIndex : " << FileIndex << " For index: " << index << " position: " << position << std::endl;

          while ( FileIndex >= db.size() )
            {
            // push on the next one.
            ::std::ostringstream newFN;
            newFN << TSDFilename << FileIndex;
            // ::std::cout << "opening new file : " << newFN.str() << std::endl;
            db.push_back ( StreamPtr ( new std::fstream ( newFN.str().c_str(), ::std::ios::out | ::std::ios::binary ) ) );
            Filenames.push_back ( newFN.str() );
            }
          
          db[FileIndex]->seekp ( position );
          db[FileIndex]->write ( reinterpret_cast<char*> ( buffer ), TimeSeriesBlockSize*TimeSeriesBlockSize*TimeSeriesBlockSize*sizeof(TPixel) );
          }
        }
      }
    }
  // Write the header
  db[0]->seekp ( 0 );
  ::std::ostringstream b;
  b << "TimeSeriesDatabase" << ::std::endl;
  b << "Version 1.0" << ::std::endl;
  b << "Dimensions: " << m_Dimensions[0] << " " << m_Dimensions[1] << " " << m_Dimensions[2] << " " << m_Dimensions[3] << std::endl;
  b << "ImageSize: " << m_OutputRegion.GetSize()[0] << " "<< m_OutputRegion.GetSize()[1] << " " << m_OutputRegion.GetSize()[2] << std::endl;
  b << "ImageOrigin: " << m_OutputOrigin[0] << " " << m_OutputOrigin[1] << " " << m_OutputOrigin[2] << std::endl;
  b << "ImageSpacing: " << m_OutputSpacing[0] << " " << m_OutputSpacing[1] << " " << m_OutputSpacing[2] << std::endl;
  b << "Direction: ";
  for ( unsigned int i = 0; i < 3; i++ )
  {
    for ( unsigned int j = 0; j < 3; j++ )
    {
      b << m_OutputDirection.GetVnlMatrix()[i][j] << " ";
    }
  }
  b << ::std::endl;
  b << "BlocksPerFile: " << BlocksPerFile << std::endl;
  b << "NumberOfFiles: " << Filenames.size() << std::endl;
  b << "Filenames: " << std::endl;
  for ( int idx = 0; idx < db.size(); idx++ )
    {
    b << Filenames[idx] << std::endl;
    }
  // std::cout << b.str() << endl;
  db[0]->write ( b.str().c_str(), strlen ( b.str().c_str() ) );
  for ( int idx = 0; idx < db.size(); idx++ )
    {
    db[idx]->flush();
    db[idx]->close();
    }
}

template <class TPixel>
float TimeSeriesDatabase<TPixel>::GetCacheSizeInMiB() 
{
  unsigned cachesize = this->m_Cache.get_maxsize();
  return (float) cachesize * sizeof ( TPixel ) * TimeSeriesVolumeBlockSize / ( 1024*1024.);
}

template <class TPixel>
void TimeSeriesDatabase<TPixel>::SetCacheSizeInMiB ( float sz )
{
  // How many blocks is this?
  double BlockSizeInMiB = sizeof ( TPixel ) * TimeSeriesVolumeBlockSize / ( 1024*1024.);
  unsigned long int blocks = (unsigned long int) ceil ( sz * BlockSizeInMiB );
  this->m_Cache.set_maxsize ( blocks );
}



template <class TPixel>
TimeSeriesDatabase<TPixel>::TimeSeriesDatabase () : m_Cache ( 1024 ){
  this->m_Dimensions.SetSize ( 4 );
  this->m_BlocksPerImage.SetSize ( 4 );
}
  
template <class TPixel>
TimeSeriesDatabase<TPixel>::~TimeSeriesDatabase () {
  // m_Cache.statistics ( std::cout );
}
  

template <class TPixel>
void
TimeSeriesDatabase<TPixel>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Dimensions: " << m_Dimensions << "\n";
  os << indent << "Filename: " << m_Filename << "\n";
  os << indent << "BlocksPerImage: " << m_BlocksPerImage[0] << " "  << "\n";
  os << indent << "OutputSpacing: " << m_OutputSpacing << "\n";
  os << indent << "OutputRegion: " << m_OutputRegion;
  os << indent << "OutputOrigin: " << m_OutputOrigin << "\n";
  os << indent << "OutputDirection: " << m_OutputDirection << "\n";
  if ( this->IsOpen() ) {
    os << indent << "Database is open." << "\n";
    os << indent << "Blocks per file: " << this->m_BlocksPerFile << "\n";
    os << indent << "File names: " << "\n";
    for ( int idx = 0; idx < this->m_DatabaseFileNames.size(); idx++ )
      {
      os << indent << this->m_DatabaseFileNames[idx] << "\n";
      }
  } else {
    os << indent << "Database is closed." << "\n";
  }
  
  this->m_Cache.statistics ( os );
}


}
