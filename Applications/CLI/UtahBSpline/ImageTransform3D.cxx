#include "Common3D.h"

#include "ParametricImageTransformation3D.h"




typedef ParametricImageTransformation3D<TSurface> TTransformation;



int main(int argc, char **argv){
 try{ 
  if(argc != 4){
    std::cout << "Usage:" << std::endl;
    std::cout << argv[0] << " inputimage outputfilename transformfile" << std::endl;
    return 0;
  }

  //Read Input Image
  ImageReaderPointer imageReader = ImageReader::New();
  imageReader->SetFileName( argv[1] );
  imageReader->Update();
  ImagePointer input = imageReader->GetOutput();
  ImageRegion region = input->GetLargestPossibleRegion();
  ImageSize regionSize = region.GetSize();
  ImageIndex regionIndex = region.GetIndex();

  //read surface
  std::ifstream file;
  file.open(argv[3]);
  file >> transform;
  file.close();

  std::cout << transform << std::endl; 
  
  
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

