#include "itkStochasticTractographyFilter.h"
#include "itkVectorImage.h"
#include "itkImageSeriesReader.h"  //this is needed for itk::ExposeMetaData()
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkAddImageFilter.h"
#include "itkUnaryFunctorImageFilter.h"
#include <iostream>
#include <vector>
#include "itkImageRegionConstIterator.h"
#include "StochasticTractographyCLP.h"
#include "itkTensorFractionalAnisotropyImageFilter.h"
#include "itkPathIterator.h"
#include <string>
#include <fstream>

std::string stripExtension(const std::string& fileName){
  const int length = fileName.length();
  for (int i=0; i!=length; ++i){
    if (fileName[i]=='.'){
      return fileName.substr(0,i);
    }
  } 
  return fileName; 
}


template< class TTOContainerType >
bool SamplingDirections(const char* fn, typename TTOContainerType::Pointer directions){
  char str[100];
  double x,y,z;
  std::ifstream m_file;
  typename TTOContainerType::Element direction;

  //open the file
  m_file.open(fn);
  if(!m_file.is_open()){
    std::cerr<<"Problems opening: "<< fn << std::endl;
    return false;
  }
  else{
    //load file info into valid itk representation
    //assume all entries in the file describe a vector of length 1
    //each line is arranged [x, y, z]
    unsigned int directioncount=0;

    while(!m_file.getline(str, 100, '\n').eof()){
      for(int i=0;i<3;i++){
          sscanf(str,"%lf %lf %lf",&x,&y,&z);
          direction[0]=x;
          direction[1]=y;
          direction[2]=z;
        }
        directions->InsertElement(directioncount++,direction);
    }
  }
  
  //close file
  m_file.close();
  return true;
}

template< class TractContainerType >
bool WriteTractContainerToFile( const char* fn, TractContainerType* tractcontainer ){
  std::ofstream tractfile( fn, std::ios::out | std::ios::binary );
  tractfile.seekp(0);
  if(tractfile.is_open()){
    for(int i=0; i<tractcontainer->Size(); i++ ){
      typename TractContainerType::Element tract =
        tractcontainer->GetElement(i);
      
      typename TractContainerType::Element::ObjectType::VertexListType::ConstPointer vertexlist = 
        tract->GetVertexList();
      
      for(int j=0; j<vertexlist->Size(); j++){
        typename TractContainerType::Element::ObjectType::VertexListType::Element vertex =
          vertexlist->GetElement(j);
        tractfile.write((char*)&vertex[0], sizeof(vertex[0]));
        tractfile.write((char*)&vertex[1], sizeof(vertex[1]));
        tractfile.write((char*)&vertex[2], sizeof(vertex[2]));
      }
      double endoftractsymbol = 0.0;
      tractfile.write((char*)&endoftractsymbol, sizeof(endoftractsymbol));
      tractfile.write((char*)&endoftractsymbol, sizeof(endoftractsymbol));
      tractfile.write((char*)&endoftractsymbol, sizeof(endoftractsymbol));
    }
    
    tractfile.close();
    return true;
  }
  else{
    std::cerr<<"Problems opening file to write tracts\n";
    return false;
  }
}
template< class FAContainerType >
bool WriteScalarContainerToFile( const char* fn, FAContainerType* facontainer ){
  std::ofstream fafile( fn, std::ios::out );
  if(fafile.is_open()){
    for(int i=0; i<facontainer->Size(); i++){
      fafile<<facontainer->GetElement(i)<<std::endl;
    }
    fafile.close();
    return true;
  }
  else{
    std::cerr<<"Problems opening file to write FA value\n";
    return false;
  }
}

namespace Functor {  
   
template< class TInput, class TOutput>
class ZeroDWITest
{
public:
  ZeroDWITest() {};
  ~ZeroDWITest() {};
  bool operator!=( const ZeroDWITest & ) const
  {
    return false;
  }
  bool operator==( const ZeroDWITest & other ) const
  {
    return !(*this != other);
  }
  inline TOutput operator()( const TInput & A )
  {
    /*for(int i=0;i<A.GetSize();i++){
      if(A[0]<100){
        std::cout<<"Invalid Voxel\n";
        return 0;
      }
    }
    */
    return 10;
  }
}; 

template< class TInput, class TOutput>
class ScalarMultiply
{
public:
  ScalarMultiply() {};
  ~ScalarMultiply() {};
  bool operator!=( const ScalarMultiply & ) const
  {
    return false;
  }
  bool operator==( const ScalarMultiply & other ) const
  {
    return !(*this != other);
  }
  inline TOutput operator()( const TInput & A )
  {
    /*for(int i=0;i<A.GetSize();i++){
      if(A[0]<100){
        std::cout<<"Invalid Voxel\n";
        return 0;
      }
    }
    */
    return A*1000;
  }
}; 

}

int main(int argc, char* argv[]){
  PARSE_ARGS;
  //define the input/output types
  typedef itk::VectorImage< unsigned short int, 3 > DWIVectorImageType;
  typedef itk::Image< float, 3 > WMPImageType;
  typedef itk::Image< short int, 3 > ROIImageType;
  typedef itk::Image< unsigned int, 3 > CImageType;

  //define an iterator for the ROI image
  typedef itk::ImageRegionConstIterator< ROIImageType > ROIImageIteratorType;
  
  //define reader and writer
  typedef itk::ImageFileReader< DWIVectorImageType > DWIVectorImageReaderType;
  typedef itk::ImageFileReader< WMPImageType > WMPImageReaderType;
  typedef itk::ImageFileReader< ROIImageType > ROIImageReaderType;
  typedef itk::ImageFileWriter< CImageType > CImageWriterType;
  typedef itk::ImageFileWriter< WMPImageType > WMPImageWriterType;
  
  //define metadata dictionary types
  typedef itk::MetaDataDictionary DictionaryType;
  typedef DictionaryType::ConstIterator DictionaryIteratorType;
    
  //define a probabilistic tractography filter type and associated bValue,
  //gradient direction, and measurement frame types
  typedef itk::StochasticTractographyFilter< DWIVectorImageType, WMPImageType,
    CImageType > 
    PTFilterType;
  typedef PTFilterType::bValueContainerType bValueContainerType;
  typedef PTFilterType::GradientDirectionContainerType GDContainerType;
  typedef PTFilterType::MeasurementFrameType MeasurementFrameType;
  
  //define a filter to generate a mask image that excludes zero dwi values
  typedef Functor::ZeroDWITest< DWIVectorImageType::PixelType, WMPImageType::PixelType >
    ZeroDWITestType;
  typedef itk::UnaryFunctorImageFilter< DWIVectorImageType, WMPImageType, 
    ZeroDWITestType > ExcludeZeroDWIFilterType;
  
  //FA stuff
  typedef itk::Image< double, 3 > FAImageType;
  typedef itk::TensorFractionalAnisotropyImageFilter< PTFilterType::OutputTensorImageType,
    FAImageType > FAFilterType;
  typedef itk::ImageFileWriter< FAImageType > FAImageWriterType;
    
  //define a filter to multiply a FA image by 1000
  typedef Functor::ScalarMultiply< FAImageType::PixelType, FAImageType::PixelType >
    ScalarMultiplyType;
  typedef itk::UnaryFunctorImageFilter< FAImageType, FAImageType,
    ScalarMultiplyType > MultiplyFAFilterType;
    
  //define AddImageFilterType to accumulate the connectivity maps of the pixels in the ROI
  typedef itk::AddImageFilter< CImageType, CImageType, CImageType> AddImageFilterType;
  
  //parse command line arguments
  //if(argc < 3){
  //  std::cerr << "Usage: " << argv[0];
  //  std::cerr<< " DWIFile(.nhdr) ROIFile(.nhdr) ConnectivityFile(.nhdr) ";
  //  std::cerr<< "LabelNumber NumberOfTracts MaxTractLength MaxLikelihoodCacheSize\n";
  //  return EXIT_FAILURE;
  //}
  /*
  char* dwifilename = argv[1];
  char* roifilename = argv[2];
  char* cfilename = argv[3];
  unsigned int labelnumber = atoi(argv[4]);
  unsigned int totaltracts = atoi(argv[5]);
  unsigned int maxtractlength = atoi(argv[6]);
  unsigned int maxlikelihoodcachesize = atoi(argv[7]);
  */
  //setup output stat files
  std::string fafilename = outputprefix + "_CONDFAValues.txt";
  //sprintf( fafilename, "%s_CONDFAValues.txt", outputprefix.c_str() );
  std::string lengthfilename = outputprefix + "_CONDLENGTHValues.txt";
  //sprintf( lengthfilename, "%s_CONDLENGTHValues.txt", outputprefix.c_str() );
  
  //open these files
  std::ofstream fafile( fafilename.c_str() );
  if(!fafile.is_open()){
    std::cerr<<"Could not open FA file!\n";
    return EXIT_FAILURE;
  }
  
  std::ofstream lengthfile( lengthfilename.c_str() );
  //if(!lengthfile.is_open()){
  //  std::cerr<<"Could not open Length file!\n";
  //  return EXIT_FAILURE;
 // }
  
  //read in the DWI image
  DWIVectorImageReaderType::Pointer dwireaderPtr = DWIVectorImageReaderType::New();
  dwireaderPtr->SetFileName(dwifilename);
  dwireaderPtr->Update();
  
  //Obtain bValue, gradient directions and measurement frame from metadata dictionary
  DictionaryType& dictionary = dwireaderPtr->GetMetaDataDictionary();
  bValueContainerType::Pointer bValuesPtr = bValueContainerType::New();
  MeasurementFrameType measurement_frame;
  GDContainerType::Pointer gradientsPtr = GDContainerType::New();
  bValueContainerType::Element scaling_bValue = 0;
  GDContainerType::Element g_i;

  //bad choice of variable names: dictit->first refers to the key in the Map
  //dictit->second refers to the Value associated with that Key
  for(DictionaryIteratorType dictit = dictionary.Begin();
      dictit!=dictionary.End();
      ++dictit){
    if(dictit->first.find("DWMRI_gradient") != std::string::npos){
      std::string metaDataValue;
      itk::ExposeMetaData< std::string > (dictionary, dictit->first, metaDataValue);
      sscanf(metaDataValue.c_str(), "%lf %lf %lf\n", &g_i[0], &g_i[1], &g_i[2]);
      //normalize the gradient vector
      gradientsPtr->InsertElement( gradientsPtr->Size(), g_i.normalize() );
    }
    else if(dictit->first.find("DWMRI_b-value") != std::string::npos){
      std::string metaDataValue;
      itk::ExposeMetaData< std::string >(dictionary, dictit->first, metaDataValue);
      scaling_bValue = atof(metaDataValue.c_str());
    }
    else if(dictit->first.find("NRRD_measurement frame") != std::string::npos){
      std::vector< std::vector < double > > metaDataValue;
      itk::ExposeMetaData< std::vector< std::vector<double> > >
        (dictionary, dictit->first, metaDataValue);
      for(int i=0;i<3;i++){
        for(int j=0;j<3;j++)
          measurement_frame(i,j) = metaDataValue[j][i];
      }
    }
    else{
      std::cout << dictit->first << std::endl;
    }
  }
  
  //check to see if bValue, gradients, or measurement frame is missing
  if(scaling_bValue == 0){
    std::cerr << "scaling_bValue should never be 0, possibly not found in Nrrd header\n";
    return EXIT_FAILURE;
  }
  else if(gradientsPtr->Size() == 0){
    std::cerr <<"no gradients were found!";
    return EXIT_FAILURE;
  }
  else if(measurement_frame.size() == 0){
    std::cerr <<"no measurement frame was found!";
    return EXIT_FAILURE;
  }
  //correct the measurement frame
  
  std::cout << scaling_bValue << std::endl;
  for(unsigned int i=0; i<gradientsPtr->Size(); i++)
    std::cout << gradientsPtr->GetElement(i) << std::endl;
  
  //std::cout << measurement_frame <<std::endl;
  
  for(unsigned int i=0; i<measurement_frame.rows(); i++){
    for(unsigned int j=0; j<measurement_frame.columns(); j++){
      std::cout<<measurement_frame(i,j) << " ";
    }
    std::cout << std::endl;
  }
 
  //correct the measurement frame since the image is now in LPS from RAS frame
  //NRRDIO should do this, but we do it here as a work around
  
  //fill up bValue container with the scaling_bValue;
  for(unsigned int i=0; i<gradientsPtr->Size() ;i++){
    if(gradientsPtr->GetElement(i).squared_magnitude() == 0){
      bValuesPtr->InsertElement(i, 0);
    }
    else{
      //this matters in the calculations for the constrained model but not the tensor model
      //since a gradient direction of all zeros handles it
      bValuesPtr->InsertElement(i, scaling_bValue);
    }
  }
  
  //setup the ROI image reader
  ROIImageReaderType::Pointer roireaderPtr = ROIImageReaderType::New();
  roireaderPtr->SetFileName(roifilename);
  roireaderPtr->Update();
  
  //setup the white matter probability image reader
  WMPImageReaderType::Pointer wmpreader = WMPImageReaderType::New();
  wmpreader->SetFileName(wmpfilename);
  wmpreader->Update();
  
  //optionally set the origins of these images to be the same as the DWI
  if(recenteroriginswitch==true){
    roireaderPtr->GetOutput()->SetOrigin( dwireaderPtr->GetOutput()->GetOrigin() );
    wmpreader->GetOutput()->SetOrigin( dwireaderPtr->GetOutput()->GetOrigin() );
  }
  /*
  //set list of directions
  typedef PTFilterType::TractOrientationContainerType TOCType;
  TOCType::Pointer directionsPtr = TOCType::New();
  
  if(SamplingDirections<TOCType>("SD.txt", directionsPtr));
  else return EXIT_FAILURE;
  */
  /*
  //write out the directions
  std::ofstream outfile("crapola.txt");
  
 
  for(int i=0; i<directionsPtr->Size(); i++){
    TOCType::Element dir = directionsPtr->GetElement(i);
    outfile<<"{"<<dir[0]<<", "<<dir[1]<<", "<<dir[2]<<"},"<<std::endl;
  }
  outfile.close();
  */
  
  //Create a default Mask Image which 
  //excludes DWI pixels which contain values that are zero
  //ExcludeZeroDWIFilterType::Pointer ezdwifilter = ExcludeZeroDWIFilterType::New();
  //ezdwifilter->SetInput( dwireaderPtr->GetOutput() );
  //std::cout<<"Start mask image\n";  
  //write out the mask image
  //MaskImageWriterType::Pointer maskwriterPtr = MaskImageWriterType::New();
  //maskwriterPtr->SetInput( ezdwifilter->GetOutput() );
  //maskwriterPtr->SetFileName( "maskimage.nhdr" );
  //maskwriterPtr->Update();
  //std::cout<<"Finish the mask image\n";
  
  std::cout<<"Create PTFilter\n";
  //Setup the PTFilter
  PTFilterType::Pointer ptfilterPtr = PTFilterType::New();
  ptfilterPtr->SetInput( dwireaderPtr->GetOutput() );
  //ptfilterPtr->SetMaskImageInput( ezdwifilter->GetOutput() );
  ptfilterPtr->SetWhiteMatterProbabilityImageInput( wmpreader->GetOutput() );
  ptfilterPtr->SetbValues(bValuesPtr);
  ptfilterPtr->SetGradients( gradientsPtr );
  ptfilterPtr->SetMeasurementFrame( measurement_frame );
  //ptfilterPtr->SetSampleDirections(directionsPtr);
  ptfilterPtr->SetMaxTractLength( maxtractlength );
  ptfilterPtr->SetTotalTracts( totaltracts );
  ptfilterPtr->SetMaxLikelihoodCacheSize( maxlikelihoodcachesize );
  if(totalthreads!=0) ptfilterPtr->SetNumberOfThreads( totalthreads );
  
  //calculate the tensor image
  ptfilterPtr->GenerateTensorImageOutput();
  
  //Setup the FAImageFilter
  FAFilterType::Pointer fafilter =  FAFilterType::New();
  
  fafilter->SetInput( ptfilterPtr->GetOutputTensorImage() );
  fafilter->Update();
  
  //Setup the FA container to hold FA values for tracts of interest
  typedef itk::VectorContainer< unsigned int, double > FAContainerType;
  //FAContainerType::Pointer facontainer = FAContainerType::New();
  
  //Setup the length container to hold lengths for tracts of interest
  typedef itk::VectorContainer< unsigned int, unsigned int > LengthContainerType;
  //LengthContainerType::Pointer lengthcontainer = LengthContainerType::New();
  
  //Setup the AddImageFilter
  AddImageFilterType::Pointer addimagefilterPtr = AddImageFilterType::New();
  
  //Create a zeroed Connectivity Image
  CImageType::Pointer accumulatedcimagePtr = CImageType::New();
  accumulatedcimagePtr->CopyInformation( dwireaderPtr->GetOutput() );
  accumulatedcimagePtr->SetBufferedRegion( dwireaderPtr->GetOutput()->GetBufferedRegion() );
  accumulatedcimagePtr->SetRequestedRegion( dwireaderPtr->GetOutput()->GetRequestedRegion() );
  accumulatedcimagePtr->Allocate();
  accumulatedcimagePtr->FillBuffer(0);
  
  //Create another Connectivity Map for just the tracts
  //which make it to the second ROI
  CImageType::Pointer conditionedcimagePtr = CImageType::New();
  conditionedcimagePtr->CopyInformation( dwireaderPtr->GetOutput() );
  conditionedcimagePtr->SetBufferedRegion( dwireaderPtr->GetOutput()->GetBufferedRegion() );
  conditionedcimagePtr->SetRequestedRegion( dwireaderPtr->GetOutput()->GetRequestedRegion() );
  conditionedcimagePtr->Allocate();
  conditionedcimagePtr->FillBuffer(0);
    
  //graft this onto the output of the addimagefilter
  addimagefilterPtr->GraftOutput(accumulatedcimagePtr);
  addimagefilterPtr->SetInput1(ptfilterPtr->GetOutput());
  addimagefilterPtr->SetInput2(addimagefilterPtr->GetOutput());
  
  ROIImageIteratorType ROIImageIt( roireaderPtr->GetOutput(),
    roireaderPtr->GetOutput()->GetRequestedRegion() );
  for(ROIImageIt.GoToBegin(); !ROIImageIt.IsAtEnd(); ++ROIImageIt){
    if(ROIImageIt.Get() == labelnumber){
      std::cout << "PixelIndex: "<< ROIImageIt.GetIndex() << std::endl;
      ptfilterPtr->SetSeedIndex( ROIImageIt.GetIndex() );
      //ptfilterPtr->GenerateTractContainerOutput();
      //write out the tract info

      //WriteTractContainerToFile( filename, ptfilterPtr->GetOutputTractContainer() );
      addimagefilterPtr->Update();
      
      std::cout<<"Calculate Statistics\n";
      //calculate the FA stats for tracts which pass through a second ROI
      PTFilterType::TractContainerType::Pointer tractcontainer = 
        ptfilterPtr->GetOutputTractContainer();

      typedef itk::PathIterator< ROIImageType, PTFilterType::TractType > 
        ROITractIteratorType;
  
      for(int i=0; i<tractcontainer->Size(); i++ ){
        PTFilterType::TractType::Pointer currtract = tractcontainer->GetElement(i);
        ROITractIteratorType roitractIt( roireaderPtr->GetOutput(),
          currtract );
        
        for(roitractIt.GoToBegin(); !roitractIt.IsAtEnd(); ++roitractIt){
          if(roitractIt.Get() == endlabelnumber){
            double accumFA=0;
            unsigned int stepcount=0;
            //integrate fa along length of tract
            for(double t=0; t<currtract->EndOfInput(); t+=0.2){
              stepcount++;
              //std::cout<<fafilter->GetOutput()->GetPixel(roitractIt.GetIndex())<<std::endl;
              accumFA+=fafilter->GetOutput()->GetPixel(currtract->EvaluateToIndex(t));
            }
            fafile<<accumFA/((double)stepcount)<<std::endl;
            lengthfile<<tractcontainer->GetElement(i)->EndOfInput()<<std::endl;
            if(fafile.fail() || lengthfile.fail() ){
              std::cerr<<"Error writing to text files\n";
              return EXIT_FAILURE;
            }
            //lengthcontainer->InsertElement( lengthcontainer->Size(),
              //(unsigned int) tractcontainer->GetElement(i)->EndOfInput() );
            //facontainer->InsertElement( facontainer->Size(), accumFA/((double)stepcount) );
 
            //color in the tracts
            for(roitractIt.GoToBegin(); !roitractIt.IsAtEnd(); ++roitractIt){
              conditionedcimagePtr->GetPixel(roitractIt.GetIndex())++;
            }
            break;
          }
        }
      }
    }
  }        
  
  std::cout<<"DWI image origin:"<< dwireaderPtr->GetOutput()->GetOrigin() <<std::endl;
  std::cout<<"ROI image origin:"<< roireaderPtr->GetOutput()->GetOrigin() <<std::endl;
  std::cout<<"wmp image origin:"<< wmpreader->GetOutput()->GetOrigin() <<std::endl;
  if(outputimageswitch){
    //Write out the Connectivity Map
    std::string cfilename = outputprefix + "_CMAP.nhdr";
    //sprintf(cfilename, "%s_CMAP.nhdr", outputprefix.c_str() );
    CImageWriterType::Pointer writerPtr = CImageWriterType::New();
    writerPtr->SetInput( accumulatedcimagePtr );
    writerPtr->SetFileName( cfilename.c_str() );
    writerPtr->Update();
    
    //Write out TensorImage
    std::string tensorimagefilename = outputprefix + "_TENSOR.nhdr";
    //sprintf(tensorimagefilename, "%s_TENSOR.nhdr", outputprefix.c_str() );
    typedef itk::ImageFileWriter< PTFilterType::OutputTensorImageType > TensorImageWriterType;
    TensorImageWriterType::Pointer tensorwriter = TensorImageWriterType::New();
    tensorwriter->SetInput( ptfilterPtr->GetOutputTensorImage() );
    tensorwriter->SetFileName( tensorimagefilename.c_str() );
    tensorwriter->Update();
    
    //Create a default Mask Image which 
    //excludes DWI pixels which contain values that are zero
    MultiplyFAFilterType::Pointer multFAfilter = MultiplyFAFilterType::New();
    multFAfilter->SetInput( fafilter->GetOutput() );  
    //write out the FA image
    std::string faimagefilename = outputprefix + "_FA.nhdr";
    //sprintf(faimagefilename, "%s_FA.nhdr", outputprefix.c_str() );
    FAImageWriterType::Pointer fawriter = FAImageWriterType::New();
    fawriter->SetInput( multFAfilter->GetOutput() );
    fawriter->SetFileName( faimagefilename.c_str() );
    fawriter->Update();
    
    //Write out the conditioned connectivity map
    CImageWriterType::Pointer condcmapwriterPtr = CImageWriterType::New();
    condcmapwriterPtr->SetInput( conditionedcimagePtr );
    std::string condcmapfilename = outputprefix + "_COND.nhdr";
    //sprintf(condcmapfilename, "%s_COND.nhdr", outputprefix.c_str());
    condcmapwriterPtr->SetFileName( condcmapfilename );
    condcmapwriterPtr->Update();
  }
  //close files
  fafile.close();
  lengthfile.close();
  if(fafile.fail() || lengthfile.fail() ){
    std::cerr<<"Error closing text files\n";
    return EXIT_FAILURE;
  }
  //Write out FA container
  //char fafilename[100];
  //sprintf( fafilename, "%s_CONDFAValues.txt", outputprefix.c_str() );
  //WriteScalarContainerToFile< FAContainerType >( fafilename, facontainer );
  
  //Write out tract length container
  //char lengthfilename[100];
  //sprintf( lengthfilename, "%s_CONDLENGTHValues.txt", outputprefix.c_str() );
  //WriteScalarContainerToFile< LengthContainerType >( lengthfilename, lengthcontainer );
  return EXIT_SUCCESS;
}
