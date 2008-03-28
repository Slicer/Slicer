

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include <fstream>

#include "Point2D.h"
#include "Point3D.h"

#include "BSpline3D.h"
#include "BSpline2D.h"
#include "ParametricImageTransformation3D.h"
#include "ParametricImageTransformation2D.h"

#define PRECISION double
#define DIMENSION 3

#if DIMENSION == 3
  typedef Point3D< PRECISION > TControlPoint;
  typedef BSpline3D<TControlPoint> TParametric;
  typedef ParametricImageTransformation3D<TParametric> TTransformation;
#else
  typedef Point2D< PRECISION > TControlPoint;
  typedef BSpline2D<TControlPoint> TParametric;
  typedef ParametricImageTransformation2D<TParametric> TTransformation;
#endif

typedef TTransformation::Image Image;
typedef TTransformation::ImagePointer ImagePointer;
typedef Image::RegionType ImageRegion;
typedef Image::IndexType ImageIndex;
typedef Image::SizeType ImageSize;

typedef itk::ImageFileReader<Image> ImageReader;
typedef ImageReader::Pointer ImageReaderPointer;

typedef itk::ImageFileWriter<Image> ImageWriter;
typedef ImageWriter::Pointer ImageWriterPointer;


int main(int argc, char **argv){
 try{ 
  if(argc != 5){
    std::cout << "Usage:" << std::endl;
    std::cout << argv[0] << " inputimage outputfilename transformfile istTransform";
    std::cout << "isTransform" << std::endl;
    return 0;
  }

  bool isTransform = atoi(argv[4]) !=0 ;

  //Read Input Image
  ImageReaderPointer imageReader = ImageReader::New();
  imageReader->SetFileName( argv[1] );
  imageReader->Update();
  ImagePointer input = imageReader->GetOutput();
  ImageRegion region = input->GetLargestPossibleRegion();
  ImageSize regionSize = region.GetSize();
  ImageIndex regionIndex = region.GetIndex();



  //read surface
  TTransformation transform;
  std::ifstream file;
  file.open(argv[3]);


  if(isTransform){
    file >> transform;
  }
  else{
    TParametric surface;
    file >> surface;
    transform.SetParametric(surface);
    transform.SetRange(region);
  }

  file.close();
  transform.SetImage(input);
  
  
  //Setup Image transformation
  std::cout << "Starting transformation" << std::endl;
  ImagePointer output = transform.Transform();
  std::cout << "Finished transformation" << std::endl;
  

  //Write ouput image
  ImageWriterPointer imageWriter = ImageWriter::New();
  imageWriter->SetFileName(argv[2]);
  imageWriter->SetInput(output);
  imageWriter->Update();
  imageWriter->Write();

  return 0;
 }
 catch(const char *err){
  std::cout << err << std::endl;
 }
}

