#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <itkTransformFileReader.h>
#include <itkImageFileWriter.h>
#include <itkBSplineDeformableTransform.h>
#include <itkImageFileReader.h>

#include <BSplineToDeformationFieldCLP.h>

using namespace std;

int main( int argc, char * argv[] )
{
  PARSE_ARGS;
  const int Dimension = 3;

  // /////////read BSpline Transform
  typedef itk::TransformFileReader::Pointer TransformReaderPointer;
  TransformReaderPointer transformFile = itk::TransformFileReader::New();
  transformFile->SetFileName(inputTransformName.c_str() );
  transformFile->Update();

  /*
  // DEBUG output

  std::cout << "size: " << transformFile->GetTransformList()->size() << std::endl;

  itk::TransformFileReader::TransformListType::iterator iteratorA;

  for ( iteratorA=transformFile->GetTransformList()->begin(); iteratorA!=transformFile->GetTransformList()->end(); ++iteratorA )
  {
    std::cout << "BSplineToDeformationField: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << std::endl;
    *iteratorA->Print(std::cout);
    std::cout << "BSplineToDeformationField: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << std::endl;
  }
  */

  typedef itk::BSplineDeformableTransform<double, Dimension, Dimension> BSplineDeformableTransformType;

  // first transform
  BSplineDeformableTransformType::Pointer BSplineTransform = BSplineDeformableTransformType::New();
  BSplineTransform =
    dynamic_cast<BSplineDeformableTransformType *>( transformFile->GetTransformList()->front().GetPointer() );

  // second transform
  transformFile->GetTransformList()->pop_front();
  if( transformFile->GetTransformList()->size() == 0 )
    {
    std::cerr << "Error, the second transform does not exist." << std::endl;
    return EXIT_FAILURE;
    }
  if( !dynamic_cast<BSplineDeformableTransformType::BulkTransformType *>( transformFile->GetTransformList()->front().
                                                                          GetPointer() ) )
    {
    std::cerr << "Error, the second transform is not Bulk Transform." << std::endl;
    return EXIT_FAILURE;
    }
  BSplineDeformableTransformType::BulkTransformType* bulkTfm =
    dynamic_cast<BSplineDeformableTransformType::BulkTransformType *>(transformFile->GetTransformList()->front().
                                                                      GetPointer() );
  BSplineTransform->SetBulkTransform(bulkTfm);

  // ///read reference volume
  typedef   short                                  PixelType;
  typedef itk::Image<PixelType, Dimension>         ReferenceImageType;
  typedef itk::ImageFileReader<ReferenceImageType> ReferenceImageReaderType;
  ReferenceImageReaderType::Pointer imagereader =  ReferenceImageReaderType::New();
  imagereader->SetFileName(referenceImageName.c_str() );
  imagereader->Update();

  // ///allocate space for deformation field
  typedef itk::Vector<float, Dimension>     VectorType;
  typedef itk::Image<VectorType, Dimension> DeformationFieldType;
  ReferenceImageType::Pointer    refImage = imagereader->GetOutput();
  DeformationFieldType::Pointer  deformationField = DeformationFieldType::New(); // Filter L's kernnel form tile to same
                                                                                 // size as the input image
  DeformationFieldType::SizeType size = imagereader->GetOutput()->GetBufferedRegion().GetSize();
  deformationField->SetRegions( size );
  deformationField->SetOrigin( refImage->GetOrigin() );
  deformationField->SetSpacing( refImage->GetSpacing() );
  deformationField->SetDirection(refImage->GetDirection() );
  deformationField->Allocate();

  // /convert to dense field
  typedef itk::ImageRegionIterator<DeformationFieldType> DeformationFieldIteratorType;
  DeformationFieldIteratorType fieldit( deformationField, deformationField->GetRequestedRegion() );

  std::cout << "Converting BSpline transform to dense field ... " << std::endl;
  for( fieldit.GoToBegin(); !fieldit.IsAtEnd(); ++fieldit )
    {
    DeformationFieldType::IndexType                 currentPix = fieldit.GetIndex();
    BSplineDeformableTransformType::InputPointType  fixedPoint;
    BSplineDeformableTransformType::OutputPointType movingPoint;
    deformationField->TransformIndexToPhysicalPoint( currentPix, fixedPoint );
    movingPoint = BSplineTransform->TransformPoint( fixedPoint );
    VectorType displacement;
    displacement = movingPoint - fixedPoint;
    fieldit.Set(displacement);
    }
  std::cout << "Converting BSpline transform to dense field successful. " << std::endl;

  typedef itk::ImageFileWriter<DeformationFieldType> DFWriterType;
  DFWriterType::Pointer deformationFieldWriter =  DFWriterType::New();
  deformationFieldWriter->SetFileName(deformationFieldName.c_str() );
  deformationFieldWriter->SetInput(deformationField);
  deformationFieldWriter->SetUseCompression(1);
  deformationFieldWriter->Update();

  return EXIT_SUCCESS;
}
