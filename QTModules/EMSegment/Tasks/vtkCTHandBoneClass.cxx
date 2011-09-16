#include "vtkCTHandBoneClass.h"
#include "vtkImageEMGeneral.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"


/**
 *
 */
vtkCTHandBoneClass* vtkCTHandBoneClass::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkCTHandBoneClass");
  if(ret)
    {
      return (vtkCTHandBoneClass*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkCTHandBoneClass;

}


/**
 *
 */
vtkCTHandBoneClass::vtkCTHandBoneClass() {
  // in use?
}


/**
 *
 */
int vtkCTHandBoneClass::flip(
                             const char* inputFile,
                             const char* outputFlippedImageFile,
                             const char* flipAxisX,
                             const char* flipAxisY,
                             const char* flipAxisZ)
{
  typedef itk::Image<signed short,3> ImageType;

  //Setup itk::ImageFileReader
  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( inputFile );

  //Setup itk::FlipImageFilter
  typedef itk::FlipImageFilter<ImageType> FilterType;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( reader->GetOutput() );
  typedef FilterType::FlipAxesArrayType FlipAxesArrayType;
  FlipAxesArrayType flipArray;
  flipArray[0] = flipAxisX;
  flipArray[1] = flipAxisY;
  flipArray[2] = flipAxisZ;
  filter->SetFlipAxes( flipArray );

  //Setup itk::ImageFileWriter
  typedef itk::ImageFileWriter<ImageType> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( filter->GetOutput() );
  writer->SetFileName( outputFlippedImageFile );
  writer->Update();

  return EXIT_SUCCESS;
}


/**
 *
 int vtkCTHandBoneClass::fiducial_threshold(
 const char* inputImage,
 const char* outputImage,
 const char* fiducialfile,
 const char* logfile
 )
*/
int vtkCTHandBoneClass::fiducial_threshold(
                                           const char* inputFilename,
                                           const char* outputFilename)
{
  //TODO
  /*

    typedef itk::ImageFileReader<ImageType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(inputFilename);
    reader->Update();

    ImageType::Pointer image = ImageType::New();
    image = reader->GetOutput();

    //Convert image to Slicer3 space
    ImageType::DirectionType imageDir = image->GetDirection( );
    ImageType::PointType origin = image->GetOrigin( );
    ImageType::SpacingType spacing = image->GetSpacing( );

    int dominantAxisRL = itk::Function::Max3(imageDir[0][0],imageDir[1][0],imageDir[2][0]);
    int signRL = itk::Function::Sign(imageDir[dominantAxisRL][0]);
    int dominantAxisAP = itk::Function::Max3(imageDir[0][1],imageDir[1][1],imageDir[2][1]);
    int signAP = itk::Function::Sign(imageDir[dominantAxisAP][1]);
    int dominantAxisSI = itk::Function::Max3(imageDir[0][2],imageDir[1][2],imageDir[2][2]);
    int signSI = itk::Function::Sign(imageDir[dominantAxisSI][2]);

    ImageType::DirectionType DirectionToRAS;
    DirectionToRAS.SetIdentity( );
    if (signRL == 1)
    {
    DirectionToRAS[dominantAxisRL][dominantAxisRL] = -1.0;
    origin[dominantAxisRL] *= -1.0;
    }
    if (signAP == -1)
    {
    DirectionToRAS[dominantAxisAP][dominantAxisAP] = -1.0;
    origin[dominantAxisAP] *= -1.0;
    }
    if (signSI == 1)
    {
    DirectionToRAS[dominantAxisSI][dominantAxisSI] = -1.0;
    origin[dominantAxisSI] *= -1.0;
    }

    imageDir *= DirectionToRAS;

    image->SetDirection( imageDir  );
    image->SetOrigin( origin );
    image->Update();

    ////Write flipped file out
    //typedef itk::ImageFileWriter< ImageType >  WriterType2;
    //WriterType2::Pointer writer2 = WriterType2::New();
    //writer2->SetFileName( outputImage );
    //writer2->SetInput(image);
    //writer2->Update();

    //VTK Storage
    vtkDoubleArray *xvalues = vtkDoubleArray::New();
    vtkDoubleArray *yvalues = vtkDoubleArray::New();
    vtkDoubleArray *zvalues = vtkDoubleArray::New();
    vtkDoubleArray *intensity = vtkDoubleArray::New();

    //Fiducial File Read in and manipulate
    std::ifstream trialFile;
    std::string trialString;
    trialFile.open( fiducialfile );

    double newvalue = 0;
    int totalfiducials = 0;

    int file_start = 0;

    while(!trialFile.eof())
    {
    std::getline(trialFile, trialString,'#');
    std::getline(trialFile, trialString,'#');
    std::getline(trialFile, trialString,'#');
    std::getline(trialFile, trialString,'#');
    std::getline(trialFile, trialString,'=');
    std::getline(trialFile, trialString,'#');
    totalfiducials = atoi(trialString.c_str());
    trialString.clear();
    std::getline(trialFile, trialString,'?');
    }
    trialFile.close();
    cout<< "totalfiducials: "<<totalfiducials<<endl;

    std::ifstream inputFile;
    std::string inputString;
    inputFile.open( fiducialfile );

    int num_fiducial = 0;

    while(num_fiducial < totalfiducials)
    {
    if(file_start == 0)
    {
    //Junk
    std::getline(inputFile, inputString,',');
    std::getline(inputFile, inputString,',');
    std::getline(inputFile, inputString,',');
    std::getline(inputFile, inputString,',');
    std::getline(inputFile, inputString,',');
    std::getline(inputFile, inputString,',');
    std::getline(inputFile, inputString,',');
    std::getline(inputFile, inputString,',');
    std::getline(inputFile, inputString,',');
    std::getline(inputFile, inputString,',');
    inputString.clear();
    file_start = 1;
    }
    else
    {
    //Junk
    std::getline(inputFile, inputString,',');
    inputString.clear();
    }

    //X-coord
    std::getline(inputFile, inputString,',');
    newvalue = atof(inputString.c_str());
    xvalues->InsertNextValue(newvalue);
    cout<<"x: "<<newvalue<<endl;
    inputString.clear();

    //Y-coord
    std::getline(inputFile, inputString,',');
    newvalue = atof(inputString.c_str());
    yvalues->InsertNextValue(newvalue);
    cout<<"y: "<<newvalue<<endl;
    inputString.clear();

    //Z-coord
    std::getline(inputFile, inputString,',');
    newvalue = atof(inputString.c_str());
    zvalues->InsertNextValue(newvalue);
    cout<<"z: "<<newvalue<<endl;
    inputString.clear();

    std::getline(inputFile, inputString,',');
    std::getline(inputFile, inputString,'\n');
    inputString.clear();

    num_fiducial++;

    }
    inputFile.close();

    int counter = xvalues->GetNumberOfTuples();

    cout<<"spacing: "<<spacing[0]<<" "<<spacing[1]<<" "<<spacing[2]<<endl;
    cout<<"origin: "<<origin[0]<<" "<<origin[1]<<" "<<origin[2]<<endl;

    //itk for pixel intensity
    ImageType::IndexType pixelIndex;
    ImageType::PixelType pixelValue;
    for(int i = 0; i<counter; i++)
    {
    pixelIndex[0] = ((xvalues->GetValue(i))-origin[0])/spacing[0];
    pixelIndex[1] = ((zvalues->GetValue(i))-origin[2])/spacing[2];
    pixelIndex[2] = ((yvalues->GetValue(i))+origin[1])/spacing[1];

    if(pixelIndex[0] < 0)
    {
    pixelIndex[0] = -pixelIndex[0];
    }
    if(pixelIndex[1] < 0)
    {
    pixelIndex[1] = -pixelIndex[1];
    }
    if(pixelIndex[2] < 0)
    {
    pixelIndex[2] = -pixelIndex[2];
    }

    cout<<"pixelindex: "<<pixelIndex[0]<<" "<<pixelIndex[1]<<" "<<pixelIndex[2]<<endl;

    pixelValue = image->GetPixel(pixelIndex);
    cout<<"pixelvalue: "<<pixelValue<<endl;
    intensity->InsertNextValue(pixelValue);
    }

    //Calculate mean
    double intensity_mean = 0;
    for(int i = 0; i<counter; i++)
    {
    intensity_mean += intensity->GetValue(i);
    }
    intensity_mean = intensity_mean/counter;

    cout<<"intensitymean: "<<intensity_mean<<endl;

    //Calculate standard deviation
    double stdev = 0;
    for(int i=0; i<counter; i++)
    {
    stdev = stdev + ((intensity_mean - (intensity->GetValue(i)))*(intensity_mean - (intensity->GetValue(i))));
    }
    stdev = sqrt(stdev/counter);

    int threshvalue = intensity_mean - stdev;

    cout<<"threshvalue: "<<threshvalue<<endl;

    ofstream outfile;
    outfile.open ( logfile, ios::app);
    outfile << "Theshold Value "<< inputImage <<" "<<threshvalue<<" mean "<<intensity_mean<<" stdev "<<stdev<<endl;
    outfile.close();
  */

  typedef itk::Image<float,3> ImageType;

  //Setup itk::ImageFileReader
  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( inputFilename );

  //Setup itk::BinaryThresholdImageFilter
  typedef itk::BinaryThresholdImageFilter<ImageType,ImageType> FilterType;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( reader->GetOutput() );
  filter->SetOutsideValue(0);
  filter->SetInsideValue(1);
  //    filter->SetLowerThreshold(threshvalue);
  // TODO
  filter->SetLowerThreshold(150);
  filter->SetUpperThreshold(3000);

  //typedef itk::RescaleIntensityImageFilter<ImageType, ImageType> RescaleFilterType;
  //RescaleFilterType::Pointer rescaler = RescaleFilterType::New();
  //rescaler->SetOutputMinimum(0);
  //rescaler->SetOutputMaximum(1);
  //rescaler->SetInput(filter->GetOutput());

  //typedef itk::ThresholdImageFilter<ImageType> FilterType;
  //FilterType::Pointer filter = FilterType::New();
  //filter->SetInput(input2->GetOutput());
  //filter->SetOutsideValue(-1000);
  //filter->ThresholdBelow(threshvalue);
  //filter->Update();

  //Setup itk::ImageFileWriter
  typedef itk::ImageFileWriter<ImageType> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( filter->GetOutput() );
  writer->SetFileName( outputFilename );
  writer->Update();

  return EXIT_SUCCESS;
}


/**
 *
 */
int vtkCTHandBoneClass::blur(
                             const char* inputImage,
                             const char* outputImage,
                             const char* gaussianVariance,
                             const char* maxKernelWidth)
{
  typedef  signed short PixelType;

  typedef itk::Image<signed short,3>  InputImageType;
  typedef itk::Image<unsigned char,3> OutputImageType;

  //Setup itk::ImageFileReader
  typedef itk::ImageFileReader<InputImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( inputImage );

  //Setup itk::RescaleIntensityImageFilter
  typedef itk::RescaleIntensityImageFilter<InputImageType,InputImageType> RescaleFilterType;
  RescaleFilterType::Pointer rescaler = RescaleFilterType::New();
  rescaler->SetInput( reader->GetOutput() );
  rescaler->SetOutputMinimum(0);
  rescaler->SetOutputMaximum(255);

  //Setup itk::DiscreteGaussianImageFilter
  typedef itk::DiscreteGaussianImageFilter<InputImageType,InputImageType> FilterType;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput(rescaler->GetOutput());
  filter->SetVariance( atoi(gaussianVariance) );
  filter->SetMaximumKernelWidth( atoi(maxKernelWidth) );

  //Setup itk::RescaleIntensityImageFilter
  typedef itk::RescaleIntensityImageFilter<InputImageType, OutputImageType> RescaleFilterType2;
  RescaleFilterType2::Pointer rescaler2 = RescaleFilterType2::New();
  rescaler2->SetInput( filter->GetOutput() );
  rescaler2->SetOutputMinimum(0);
  rescaler2->SetOutputMaximum(255);

  //Setup itk::ImageFileWriter
  typedef itk::ImageFileWriter<OutputImageType> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput(rescaler2->GetOutput());
  writer->SetFileName( outputImage );
  writer->Update();

  return EXIT_SUCCESS;
}


/**
 *
 */
int vtkCTHandBoneClass::binary_threshold(
                                         const char* inputFilename,
                                         const char* outputFilename,
                                         const char* _lowerThreshold,
                                         const char* _upperThreshold)
{
  // TODO signed vs unsigned
  typedef  signed short  InputPixelType;
  const InputPixelType lowerThreshold = atoi( _lowerThreshold );
  const InputPixelType upperThreshold = atoi( _upperThreshold );

  // Setup pixel type and input image type
  typedef itk::Image<InputPixelType,3>   ImageType;

  ////Read in dicom series
  //itk::GDCMSeriesFileNames::Pointer FileNameGenerator = itk::GDCMSeriesFileNames::New();
  //FileNameGenerator->SetUseSeriesDetails(false);
  //FileNameGenerator->SetDirectory( inputDirectory );

  //std::vector<std::string> seriesUIDs = FileNameGenerator->GetSeriesUIDs();

  //typedef itk::ImageSeriesReader< ImageType > ImageSeriesReaderType;
  //ImageSeriesReaderType::Pointer imageSeriesReader = ImageSeriesReaderType::New();
  //itk::GDCMImageIO::Pointer dicomIO = itk::GDCMImageIO::New();

  //imageSeriesReader->SetFileNames( FileNameGenerator->GetFileNames( seriesUIDs[0] ) );
  //imageSeriesReader->SetImageIO( dicomIO );
  //imageSeriesReader->Update();

  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer input = ReaderType::New();
  input->SetFileName(inputFilename);

  typedef itk::BinaryThresholdImageFilter<ImageType,ImageType> FilterType;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput(input->GetOutput());
  filter->SetOutsideValue(255);
  filter->SetInsideValue(0);
  filter->SetLowerThreshold(lowerThreshold);
  filter->SetUpperThreshold(upperThreshold);

  typedef itk::RescaleIntensityImageFilter<ImageType,ImageType> RescaleFilterType;
  RescaleFilterType::Pointer rescaler = RescaleFilterType::New();
  rescaler->SetInput(filter->GetOutput());
  rescaler->SetOutputMinimum(0);
  rescaler->SetOutputMaximum(255);

  typedef itk::ImageFileWriter<ImageType>  WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput(rescaler->GetOutput());
  writer->SetFileName(outputFilename);
  writer->Update();

  return EXIT_SUCCESS;
}


/**
 *
 * std::cerr << "Function: Island Removal and Fill: " << std::endl;
 *
 * Extract Label of Interest
 */
int vtkCTHandBoneClass::largest_component(
                                          const char* inputImage,
                                          const char* outputImage)
{
  typedef unsigned char PixelType;
  const int dimension=3;
  typedef itk::Image<PixelType,dimension> ImageType;

  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer fileReader = ReaderType::New();
  fileReader->SetFileName( inputImage );


  std::cout << "Extract Label" << std::endl;
  typedef itk::BinaryThresholdImageFilter<ImageType,ImageType> BinaryFilterType;
  BinaryFilterType::Pointer extractLabelFilter = BinaryFilterType::New();
  extractLabelFilter->SetInput( fileReader->GetOutput() );
  extractLabelFilter->SetLowerThreshold(1);
  extractLabelFilter->SetUpperThreshold(255);
  extractLabelFilter->SetInsideValue(1);
  extractLabelFilter->SetOutsideValue(0);


  std::cout << "Connected Component" << std::endl;
  typedef itk::ConnectedComponentImageFilter<ImageType,ImageType,ImageType> ConnectedFilterType;
  ConnectedFilterType::Pointer connectedFilter = ConnectedFilterType::New();
  connectedFilter->SetInput( extractLabelFilter->GetOutput() );


  std::cout << "Relabel Image" << std::endl;
  typedef itk::RelabelComponentImageFilter<ImageType,ImageType> RelabelFilterType;
  RelabelFilterType::Pointer relabelFilter = RelabelFilterType::New();
  relabelFilter->SetInput(connectedFilter->GetOutput());


  std::cout << "Binary Image" << std::endl;
  BinaryFilterType::Pointer binaryFilter = BinaryFilterType::New();
  binaryFilter->SetInput( relabelFilter->GetOutput() );
  binaryFilter->SetLowerThreshold(1);
  binaryFilter->SetUpperThreshold(1);
  binaryFilter->SetInsideValue(1);
  binaryFilter->SetOutsideValue(0);


  typedef itk::ImageFileWriter<ImageType> WriterType;
  WriterType::Pointer fileWriter= WriterType::New();
  fileWriter->SetInput( binaryFilter->GetOutput() );
  fileWriter->SetFileName( outputImage );
  fileWriter->Update();

  return EXIT_SUCCESS;
}


/**
 *
 */
void vtkCTHandBoneClass::PrintSelf(ostream& os, vtkIndent indent) {
  os << indent << "-----------vtkCTHandBoneClass------- CLASS ------------------" << endl;
}


