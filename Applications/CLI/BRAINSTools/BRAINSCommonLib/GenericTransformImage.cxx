#include <iostream>
#include "GenericTransformImage.h"


AffineTransformType::Pointer
ReadLinearTransform(const std::string & filename)
{
  itk::TransformFileReader::Pointer affineReader
    = itk::TransformFileReader::New();

  affineReader->SetFileName( filename.c_str() );
  affineReader->Update();
  AffineTransformType::Pointer returnval
    = AffineTransformType::New();

  //  This interpretation of a Transform List file
  //  as a single AffineTransform's Parameters
  //  is ridiculously restrictive, but there it is.
  //
  itk::TransformFileReader::TransformPointer transformFromFile
    = *( affineReader->GetTransformList()->begin() );

  // START: Copied out of BRAINSFit
  //
  // Process the initialITKTransform
  //
  AffineTransformType::Pointer initialITKTransform = AffineTransformType::New();
  initialITKTransform->SetIdentity();
    {
    try
      {
      typedef itk::TransformFileReader::TransformListType *TransformListType;
      TransformListType transforms = affineReader->GetTransformList();
      // std::cout << "Number of transforms = " << transforms->size() <<
      // std::endl;

      itk::TransformFileReader::TransformListType::const_iterator it
        = transforms->begin();
      const std::string
      transformFileType = ( *it )->GetNameOfClass();
      if ( transformFileType == "VersorRigid3DTransform" )
        {
        VersorRigid3DTransformType::Pointer tempInitializerITKTransform
          = static_cast<VersorRigid3DTransformType *>( ( *it ).GetPointer() );
        AssignRigid::AssignConvertedTransform(initialITKTransform,
          tempInitializerITKTransform);
        }
      else if ( transformFileType == "ScaleVersor3DTransform" )
        {
        ScaleVersor3DTransformType::Pointer tempInitializerITKTransform
          = static_cast<ScaleVersor3DTransformType *>( ( *it ).GetPointer() );
        AssignRigid::AssignConvertedTransform(initialITKTransform,
          tempInitializerITKTransform);
        }
      else if ( transformFileType == "ScaleSkewVersor3DTransform" )
        {
        ScaleSkewVersor3DTransformType::Pointer tempInitializerITKTransform
          = static_cast<ScaleSkewVersor3DTransformType *>( ( *it ).GetPointer() );
        AssignRigid::AssignConvertedTransform(initialITKTransform,
          tempInitializerITKTransform);
        }
      else if ( transformFileType == "AffineTransform" )
        {
        AffineTransformType::Pointer tempInitializerITKTransform
          = static_cast<AffineTransformType *>( ( *it ).GetPointer() );
        AssignRigid::AssignConvertedTransform(initialITKTransform,
          tempInitializerITKTransform);
        }
      else    //  NO SUCH CASE!!
        {
        std::cout
       <<
        "Unsupported initial transform file -- TransformBase first transform typestring, "
       << transformFileType
       <<
        " not equal to any recognized type VersorRigid3DTransform OR ScaleVersor3DTransform OR ScaleSkewVersor3DTransform OR AffineTransform"
       << std::endl;
        return NULL;
        }
      }
    catch ( itk::ExceptionObject & excp )
      {
      std::cout << "[FAILED]" << std::endl;
      std::cerr << "Error while reading the transforms" << std::endl;
      std::cerr << excp << std::endl;
      return NULL;
      }
    }
  // END Copied out of BRAINSFit

  return initialITKTransform;
}





void ReadDotMatTransformFile(std::string RegistrationFilename,
  BSplineTransformType::Pointer &itkBSplineTransform,
  AffineTransformType::Pointer &ITKAffineTransform,
  bool invertTransform)
{
  AffineTransformType::Pointer BulkTransform;
  bool definitelyBSpline = false;

  //Read *mat Transformation
  itk::TransformFileReader::Pointer transformReader
    = itk::TransformFileReader::New();

/*  std::cout<<"Read Transformation File of "
           <<RegistrationFilename.c_str()
           <<std::endl;
           */
  transformReader->SetFileName( RegistrationFilename.c_str() );
  try
    {
      transformReader->Update();
    }
  catch ( itk::ExceptionObject & excp )
    {
      std::cout << "[FAILED]" << std::endl;
      std::cerr << "Error while reading the transform:"
                << RegistrationFilename << std::endl;
      std::cerr << excp << std::endl;
      return;
    }
  catch ( ... )
    {
      std::cerr << "UNIDENTIFIED Error while reading the transform:"
                << RegistrationFilename << std::endl;
      return;
    }

  typedef itk::TransformFileReader::TransformListType *TransformListType;
  TransformListType transforms = transformReader->GetTransformList();
  std::cout << "Number of transforms = " << transforms->size() << std::endl;

  itk::TransformFileReader::TransformListType::const_iterator it
    = transforms->begin();

  if (transforms->size() == 1) // There is no bulk transform.
    {
    BulkTransform = AffineTransformType::New();
    BulkTransform->SetIdentity();
    const std::string firstNameOfClass = ( *it )->GetNameOfClass();
    std::cout << "FIRST (and only) NameOfClass = " << firstNameOfClass << std::endl;
    definitelyBSpline = (firstNameOfClass == "BSplineDeformableTransform");
    }
  else // Pick up what we presume was the bulk transform.
    {
    BulkTransform = static_cast<AffineTransformType *>( ( *it ).GetPointer() );
    const std::string firstNameOfClass = ( *it )->GetNameOfClass();
    std::cout << "First (Bulk) NameOfClass = " << firstNameOfClass << std::endl;
    it++;
    const std::string secondNameOfClass = ( *it )->GetNameOfClass();
    std::cout << "SECOND NameOfClass = " << secondNameOfClass << std::endl;
    definitelyBSpline = (secondNameOfClass == "BSplineDeformableTransform");
    }

  if (definitelyBSpline)
    {
    itkBSplineTransform = static_cast<BSplineTransformType *>( ( *it ).GetPointer() );
    itkBSplineTransform->SetBulkTransform( BulkTransform );
    std::cout << "warpTransform recognized as a BSpline." << std::endl;
    }
  else
    {
    ITKAffineTransform = ReadLinearTransform( RegistrationFilename.c_str() );
    std::cout << "warpTransform recognized as one of the linear transforms." << std::endl;

    if ( invertTransform )
      {
      AffineTransformType::Pointer ITKAffineTempTransform
        = AffineTransformType::New( );
      ITKAffineTempTransform->SetIdentity();

      ITKAffineTempTransform->SetFixedParameters(
        ITKAffineTransform->GetFixedParameters() );
      ITKAffineTempTransform->SetParameters( 
        ITKAffineTransform->GetParameters() );
      ITKAffineTempTransform->GetInverse( ITKAffineTransform );
      }
    }
}

