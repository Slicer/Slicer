#include <iostream>
#include "GenericTransformImage.h"

#include "itkAffineTransform.h"
#include "itkBSplineDeformableTransform.h"
#include "itkCenteredAffineTransform.h"
#include "itkCenteredEuler3DTransform.h"
#include "itkCenteredRigid2DTransform.h"
#include "itkCenteredSimilarity2DTransform.h"
#include "itkEuler2DTransform.h"
#include "itkEuler3DTransform.h"
#include "itkFixedCenterOfRotationAffineTransform.h"
#include "itkIdentityTransform.h"
#include "itkQuaternionRigidTransform.h"
#include "itkRigid2DTransform.h"
#include "itkRigid3DPerspectiveTransform.h"
#include "itkRigid3DTransform.h"
#include "itkScalableAffineTransform.h"
#include "itkScaleLogarithmicTransform.h"
#include "itkScaleSkewVersor3DTransform.h"
#include "itkScaleTransform.h"
#include "itkScaleVersor3DTransform.h"
#include "itkSimilarity2DTransform.h"
#include "itkTranslationTransform.h"
#include "itkVersorRigid3DTransform.h"
#include "itkVersorTransform.h"
#include "itkThinPlateR2LogRSplineKernelTransform.h"
#include "itkThinPlateSplineKernelTransform.h"
#include "itkTransformFactory.h"
#include <itksys/SystemTools.hxx>

//#include "itkSimilarity2DTransfor3DPerspectiveTransform.h"

namespace itk
{
VersorRigid3DTransformType::Pointer ComputeRigidTransformFromGeneric(
  const GenericTransformType::ConstPointer genericTransformToWrite)
{
  typedef VersorRigid3DTransformType VersorRigidTransformType;
  VersorRigidTransformType::Pointer versorRigid = VersorRigidTransformType::New();
  versorRigid->SetIdentity();
  ////////////////////////////////////////////////////////////////////////////
  // ConvertTransforms
  //
  if ( genericTransformToWrite.IsNotNull() )
    {
    try
      {
      const std::string transformFileType = genericTransformToWrite->GetNameOfClass();
      if ( transformFileType == "VersorRigid3DTransform" )
        {
        const VersorRigid3DTransformType::ConstPointer tempInitializerITKTransform =
          dynamic_cast< VersorRigid3DTransformType const *const >( genericTransformToWrite.GetPointer() );
        AssignRigid::ExtractVersorRigid3DTransform(versorRigid, tempInitializerITKTransform);
        }
      else if ( transformFileType == "ScaleVersor3DTransform" )
        {
        const ScaleVersor3DTransformType::ConstPointer tempInitializerITKTransform =
          dynamic_cast< ScaleVersor3DTransformType const *const >( genericTransformToWrite.GetPointer() );
        AssignRigid::ExtractVersorRigid3DTransform(versorRigid, tempInitializerITKTransform);
        }
      else if ( transformFileType == "ScaleSkewVersor3DTransform" )
        {
        const ScaleSkewVersor3DTransformType::ConstPointer tempInitializerITKTransform =
          dynamic_cast< ScaleSkewVersor3DTransformType const *const >( genericTransformToWrite.GetPointer() );
        AssignRigid::ExtractVersorRigid3DTransform(versorRigid, tempInitializerITKTransform);
        }
      else if ( transformFileType == "AffineTransform" )
        {
        const AffineTransformType::ConstPointer tempInitializerITKTransform =
          dynamic_cast< AffineTransformType const *const >( genericTransformToWrite.GetPointer() );
        AssignRigid::ExtractVersorRigid3DTransform(versorRigid, tempInitializerITKTransform);
        }
      /*
        * else if(transformFileType == "BSplineDeformableTransform")
        * {
        * BSplineTransformType::Pointer tempInitializerITKTransform
        * = dynamic_cast<BSplineTransformType *>(
        *    genericTransformToWrite.GetPointer() );
        * //AssignRigid::ExtractVersorRigid3DTransform(versorRigid,
        *    tempInitializerITKTransform->GetBulkTransform());
        * versorRigid=NULL; //NOT: Perhaps it makes sense to extract the rigid
        *    part of the bulk transform.  But that is pretty obscure case.
        * return NULL;
        * }
        */
      else      //  NO SUCH CASE!!
        {
        std::cout
        << "Unsupported initial transform file -- TransformBase first transform typestring, "
        << transformFileType
        << " not equal to any recognized type VersorRigid3DTransform OR "
        << " ScaleVersor3DTransform OR ScaleSkewVersor3DTransform OR AffineTransform"
        << std::endl;
        return NULL;
        }
      }
    catch ( itk::ExceptionObject & excp )
      {
      std::cout << "[FAILED]" << std::endl;
      std::cerr
      << "Error while converting the genericTransformToWrite to Rigid"
      << std::endl;
      std::cerr << excp << std::endl;
      return NULL;
      }
    }
  return versorRigid;
}

int WriteBothTransformsToDisk(const GenericTransformType::ConstPointer genericTransformToWrite,
                              const std::string & outputTransform,
                              const std::string & strippedOutputTransform)
{
  ////////////////////////////////////////////////////////////////////////////
  // Write out tranfoms.
  //
  if ( genericTransformToWrite.IsNull() )
    {
    return 0;
    }
  try
    {
    const std::string transformFileType = genericTransformToWrite->GetNameOfClass();
    if ( transformFileType == "VersorRigid3DTransform" )
      {
      if ( outputTransform.size() > 0 ) // Write out the transform
        {
        itk::WriteTransformToDisk(genericTransformToWrite, outputTransform);
        }
      }
    else if ( transformFileType == "ScaleVersor3DTransform" )
      {
      if ( outputTransform.size() > 0 ) // Write out the transform
        {
        itk::WriteTransformToDisk(genericTransformToWrite, outputTransform);
        }
      }
    else if ( transformFileType == "ScaleSkewVersor3DTransform" )
      {
      if ( outputTransform.size() > 0 ) // Write out the transform
        {
        itk::WriteTransformToDisk(genericTransformToWrite, outputTransform);
        }
      }
    else if ( transformFileType == "AffineTransform" )
      {
      if ( outputTransform.size() > 0 ) // Write out the transform
        {
        itk::WriteTransformToDisk(genericTransformToWrite, outputTransform);
        }
      }
    else if ( transformFileType == "BSplineDeformableTransform" )
      {
      const BSplineTransformType::ConstPointer tempInitializerITKTransform =
        dynamic_cast< BSplineTransformType const *const >( genericTransformToWrite.GetPointer() );
      if ( strippedOutputTransform.size() > 0 )
        {
        std::cout << "ERROR:  The rigid component of a BSpline transform is not supported." << std::endl;
        }
      if ( outputTransform.size() > 0 )
        {
        itk::WriteTransformToDisk(genericTransformToWrite, outputTransform);
        }
      }
    else      //  NO SUCH CASE!!
      {
      std::cout << "Unsupported initial transform file -- TransformBase first transform typestring, "
                << transformFileType
                <<
      " not equal to any recognized type VersorRigid3DTransform OR ScaleVersor3DTransform OR ScaleSkewVersor3DTransform OR AffineTransform"
                << std::endl;
      return -1;
      }
    //Should just write out the rigid transform here.
    if ( strippedOutputTransform.size() > 0  )
      {
      typedef VersorRigid3DTransformType VersorRigidTransformType;
      VersorRigidTransformType::Pointer versorRigid = itk::ComputeRigidTransformFromGeneric(genericTransformToWrite);
      if ( versorRigid.IsNotNull() )
        {
        itk::WriteTransformToDisk(versorRigid.GetPointer(), strippedOutputTransform);
        }
      }
    }
  catch ( itk::ExceptionObject & excp )
    {
    std::cout << "[FAILED]" << std::endl;
    std::cerr << "Error while writing the genericTransformToWrite"
              << std::endl;
    std::cerr << excp << std::endl;
    exit(-1);
    }
  return 0;
}

int WriteStrippedRigidTransformToDisk(const GenericTransformType::ConstPointer genericTransformToWrite,
                                      const std::string & strippedOutputTransform)
{
  return WriteBothTransformsToDisk(genericTransformToWrite, std::string(""), strippedOutputTransform);
}

GenericTransformType::Pointer ReadTransformFromDisk(const std::string initialTransform)
{
  GenericTransformType::Pointer genericTransform = NULL;
  //
  //
  // read in the initial ITKTransform
  //
  //
  TransformReaderType::Pointer      transformListReader =  TransformReaderType::New();
  TransformListType                 currentTransformList;
  TransformListType::const_iterator currentTransformListIterator;

  if ( initialTransform.size() > 0 )
    {
    std::cout
    << "Read ITK transform from text file: "
    << initialTransform << std::endl;

    transformListReader->SetFileName( initialTransform.c_str() );
    try
      {
      transformListReader->Update();
      }
    catch ( itk::ExceptionObject & err )
      {
      std::cerr << "ExceptionObject caught !" << std::endl;
      std::cerr << err << std::endl;
      exit(-1);
      }
    currentTransformList = *( transformListReader->GetTransformList() );
    if ( currentTransformList.size() == 0 )
      {
      std::cout << "Number of currentTransformList = " << currentTransformList.size() << std::endl;
      std::cout << "FATAL ERROR: Failed to read transform" << initialTransform << std::endl;
      exit(-1);
      }
    }
  if ( currentTransformList.size() == 1 ) //Most simple transform types
    {
    //NOTE:  The dynamic casting here circumvents the standard smart pointer
    // behavior.  It is important that
    // by making a new copy and transfering the parameters it is more safe.  Now
    // we only need to ensure
    // that currentTransformList.begin() smart pointer is stable (i.e. not
    // deleted) while the variable
    // temp has a reference to it's internal structure.
    const std::string transformFileType = ( *( currentTransformList.begin() ) )->GetNameOfClass();
    if ( transformFileType == "VersorRigid3DTransform" )
      {
      const VersorRigid3DTransformType::ConstPointer tempInitializerITKTransform =
        dynamic_cast< VersorRigid3DTransformType const *const >( ( *( currentTransformList.begin() ) ).GetPointer() );
      VersorRigid3DTransformType::Pointer tempCopy = VersorRigid3DTransformType::New();
      AssignRigid::AssignConvertedTransform(tempCopy,
                                            tempInitializerITKTransform);
      genericTransform = tempCopy.GetPointer();
      }
    else if ( transformFileType == "ScaleVersor3DTransform" )
      {
      const ScaleVersor3DTransformType::ConstPointer tempInitializerITKTransform =
        dynamic_cast< ScaleVersor3DTransformType const *const >( ( *( currentTransformList.begin() ) ).GetPointer() );
      ScaleVersor3DTransformType::Pointer tempCopy = ScaleVersor3DTransformType::New();
      AssignRigid::AssignConvertedTransform(tempCopy,
                                            tempInitializerITKTransform);
      genericTransform = tempCopy.GetPointer();
      }
    else if ( transformFileType == "ScaleSkewVersor3DTransform" )
      {
      const ScaleSkewVersor3DTransformType::ConstPointer tempInitializerITKTransform =
        dynamic_cast< ScaleSkewVersor3DTransformType const *const >( ( *( currentTransformList.begin() ) ).GetPointer() );
      ScaleSkewVersor3DTransformType::Pointer tempCopy = ScaleSkewVersor3DTransformType::New();
      AssignRigid::AssignConvertedTransform(tempCopy,
                                            tempInitializerITKTransform);
      genericTransform = tempCopy.GetPointer();
      }
    else if ( transformFileType == "AffineTransform" )
      {
      const AffineTransformType::ConstPointer tempInitializerITKTransform =
        dynamic_cast< AffineTransformType const *const >( ( *( currentTransformList.begin() ) ).GetPointer() );
      AffineTransformType::Pointer tempCopy = AffineTransformType::New();
      AssignRigid::AssignConvertedTransform(tempCopy,
                                            tempInitializerITKTransform);
      genericTransform = tempCopy.GetPointer();
      }
    else if ( transformFileType == "ThinPlateR2LogRSplineKernelTransform" )
      {
      const ThinPlateSpline3DTransformType::ConstPointer tempInitializerITKTransform =
      dynamic_cast< ThinPlateSpline3DTransformType const *const >( ( *( currentTransformList.begin() ) ).GetPointer() );
      ThinPlateSpline3DTransformType::Pointer tempCopy = ThinPlateSpline3DTransformType::New();
      tempCopy->SetFixedParameters( tempInitializerITKTransform->GetFixedParameters() );
      tempCopy->SetParametersByValue( tempInitializerITKTransform->GetParameters() );
      tempCopy->ComputeWMatrix();  
      genericTransform = tempCopy.GetPointer();
      }
    }
  else if ( currentTransformList.size() == 2 ) //A special case for
                                               // BSplineTransforms
  //To recombine the bulk and the bSpline transforms.
    {
    // transformListReader->GetTransformList();
    TransformListType::const_iterator initializeTransformsListIterator =
      currentTransformList.begin();

    const GenericTransformType::ConstPointer FirstTransform = dynamic_cast< GenericTransformType const *const >
                                                              ( ( *( initializeTransformsListIterator ) ).GetPointer() );
    const std::string FirstTransformFileType = FirstTransform->GetNameOfClass();

    initializeTransformsListIterator++; //Increment to next iterator

    const GenericTransformType::ConstPointer SecondTransform = dynamic_cast< GenericTransformType const *const >
                                                               ( ( *( initializeTransformsListIterator ) ).GetPointer() );
    const std::string SecondTransformFileType = SecondTransform->GetNameOfClass();

    BSplineTransformType::Pointer outputBSplineTransform =
      BSplineTransformType::New();
    outputBSplineTransform->SetIdentity();

    //Now get the BSpline information
    if ( FirstTransformFileType == "BSplineDeformableTransform" )
      {
      const BSplineTransformType::ConstPointer tempInitializerITKTransform =
        dynamic_cast< BSplineTransformType const *const >
        ( FirstTransform.GetPointer() );
      outputBSplineTransform->SetFixedParameters( tempInitializerITKTransform->GetFixedParameters() );
      outputBSplineTransform->SetParametersByValue( tempInitializerITKTransform->GetParameters() );
      outputBSplineTransform->SetBulkTransform(SecondTransform);
      }
    else if ( SecondTransformFileType == "BSplineDeformableTransform" )
      {
      const BSplineTransformType::ConstPointer tempInitializerITKTransform =
        dynamic_cast< BSplineTransformType const *const >
        ( SecondTransform.GetPointer() );
      outputBSplineTransform->SetFixedParameters( tempInitializerITKTransform->GetFixedParameters() );
      outputBSplineTransform->SetParametersByValue( tempInitializerITKTransform->GetParameters() );
      outputBSplineTransform->SetBulkTransform(FirstTransform);
      }
    else
      {
      std::cout << "[FAILED]" << std::endl;
      std::cerr
      << "Error using the currentTransformList has two elements, but neither of them are a BSplineDeformableTransform/"
      << std::endl;
      std::cerr
      << "There should not be more than two transforms in the transform list."
      << std::endl;
      return NULL;
      }
    genericTransform = outputBSplineTransform.GetPointer();
    }
  else if ( currentTransformList.size() > 2 )
    {
    // Error, too many transforms on transform list.
    std::cout << "[FAILED]" << std::endl;
    std::cerr
    << "Error using the currentTransformList for initializing a BSPlineDeformableTransform:"
    << std::endl;
    std::cerr
    << "There should not be more than two transforms in the transform list."
    << std::endl;
    return NULL;
    }
  return genericTransform;
}

void WriteTransformToDisk(GenericTransformType const *const MyTransform, const std::string TransformFilename)
{
  /*
    *  Convert the transform to the appropriate assumptions and write it out as
    *requested.
    */
    {
    typedef itk::TransformFileWriter TransformWriterType;
    TransformWriterType::Pointer transformWriter =  TransformWriterType::New();
    transformWriter->SetFileName( TransformFilename.c_str() );

    const std::string transformFileType = MyTransform->GetNameOfClass();
    if ( transformFileType == "BSplineDeformableTransform" )
      {
      const BSplineTransformType::ConstPointer tempInitializerITKTransform =
        dynamic_cast< BSplineTransformType const *const >( MyTransform );

      //NOTE: Order was reversed in order to get BSpline first, then Bulk
      // transform in order to
      //try to appease Slicer3.
      transformWriter->AddTransform(tempInitializerITKTransform);
      //Bulk transform is assumed to be second in Slicer3.
      transformWriter->AddTransform( tempInitializerITKTransform->GetBulkTransform() );
      }
    else
      {
      transformWriter->AddTransform(MyTransform);
      }
    try
      {
      transformWriter->Update();
      }
    catch ( itk::ExceptionObject & excp )
      {
      std::cout << "[FAILED]" << std::endl;
      std::cerr << "Error while writing transform file: "
                << TransformFilename << std::endl;
      std::cerr << excp << std::endl;
      return;
      }
    // Test if the file exists.
    if ( !itksys::SystemTools::FileExists( TransformFilename.c_str() ) )
      {
      itk::ExceptionObject e(__FILE__, __LINE__, "Failed to write file", "WriteTransformToDisk");
      std::ostringstream        msg;
      msg << "The file was not successfully created. "
          << std::endl << "Filename = " << TransformFilename
          << std::endl;
      e.SetDescription( msg.str().c_str() );
      throw e;
      exit(-1);
      return;
      }
    }
}

//Adding a single new transform require registering all the transform types.
void AddExtraTransformRegister(void)
{
  // This is needed in order to read and write ScaleVersor3D TransformTypes.
  // Hopefully in ITK-3-19 this will become part of the non-review transform
  // types.
  itk::TransformFactory< itk::ScaleVersor3DTransform< float > >::RegisterDefaultTransforms();
  itk::TransformFactory< itk::ScaleVersor3DTransform< double > >::RegisterTransform ();
  itk::TransformFactory< itk::ScaleVersor3DTransform< float > >::RegisterTransform ();

  itk::TransformFactory< itk::AffineTransform< double, 2 > >::RegisterTransform ();
  itk::TransformFactory< itk::AffineTransform< double, 3 > >::RegisterTransform ();
  itk::TransformFactory< itk::BSplineDeformableTransform< double, 2, 2 > >::RegisterTransform ();
  itk::TransformFactory< itk::BSplineDeformableTransform< double, 3, 3 > >::RegisterTransform ();
  itk::TransformFactory< itk::CenteredAffineTransform< double, 2 > >::RegisterTransform ();
  itk::TransformFactory< itk::CenteredAffineTransform< double, 3 > >::RegisterTransform ();
  itk::TransformFactory< itk::CenteredEuler3DTransform< double > >::RegisterTransform ();
  itk::TransformFactory< itk::CenteredRigid2DTransform< double > >::RegisterTransform();
  itk::TransformFactory< itk::CenteredSimilarity2DTransform< double > >::RegisterTransform ();
  itk::TransformFactory< itk::Similarity2DTransform< double > >::RegisterTransform ();
  itk::TransformFactory< itk::Euler2DTransform< double > >::RegisterTransform ();
  itk::TransformFactory< itk::Euler3DTransform< double > >::RegisterTransform ();
  itk::TransformFactory< itk::FixedCenterOfRotationAffineTransform< double > >::RegisterTransform ();
  itk::TransformFactory< itk::IdentityTransform< double, 2 > >::RegisterTransform ();
  itk::TransformFactory< itk::IdentityTransform< double, 3 > >::RegisterTransform ();
  itk::TransformFactory< itk::QuaternionRigidTransform< double > >::RegisterTransform ();
  itk::TransformFactory< itk::Rigid2DTransform< double > >::RegisterTransform ();
  itk::TransformFactory< itk::Rigid3DPerspectiveTransform< double > >::RegisterTransform ();
  itk::TransformFactory< itk::ScalableAffineTransform< double > >::RegisterTransform ();
  itk::TransformFactory< itk::ScaleLogarithmicTransform< double > >::RegisterTransform ();
  itk::TransformFactory< itk::ScaleSkewVersor3DTransform< double > >::RegisterTransform ();
  itk::TransformFactory< itk::ScaleTransform< double, 2 > >::RegisterTransform ();
  itk::TransformFactory< itk::ScaleTransform< double, 2 > >::RegisterTransform ();
  itk::TransformFactory< itk::ScaleTransform< double, 3 > >::RegisterTransform ();
  itk::TransformFactory< itk::TranslationTransform< double > >::RegisterTransform ();
  itk::TransformFactory< itk::VersorRigid3DTransform< double > >::RegisterTransform ();
  itk::TransformFactory< itk::VersorTransform< double > >::RegisterTransform ();
  itk::TransformFactory< itk::AffineTransform< float, 2 > >::RegisterTransform ();
  itk::TransformFactory< itk::AffineTransform< float, 3 > >::RegisterTransform ();
  itk::TransformFactory< itk::BSplineDeformableTransform< float, 2, 2 > >::RegisterTransform ();
  itk::TransformFactory< itk::BSplineDeformableTransform< float, 3, 3 > >::RegisterTransform ();
  itk::TransformFactory< itk::CenteredAffineTransform< float, 2 > >::RegisterTransform ();
  itk::TransformFactory< itk::CenteredAffineTransform< float, 3 > >::RegisterTransform ();
  itk::TransformFactory< itk::CenteredEuler3DTransform< float > >::RegisterTransform ();
  itk::TransformFactory< itk::CenteredRigid2DTransform< float > >::RegisterTransform();
  itk::TransformFactory< itk::CenteredSimilarity2DTransform< float > >::RegisterTransform ();
  itk::TransformFactory< itk::Similarity2DTransform< float > >::RegisterTransform ();
  itk::TransformFactory< itk::Euler2DTransform< float > >::RegisterTransform ();
  itk::TransformFactory< itk::Euler3DTransform< float > >::RegisterTransform ();
  itk::TransformFactory< itk::FixedCenterOfRotationAffineTransform< float > >::RegisterTransform ();
  itk::TransformFactory< itk::IdentityTransform< float, 2 > >::RegisterTransform ();
  itk::TransformFactory< itk::IdentityTransform< float, 3 > >::RegisterTransform ();
  itk::TransformFactory< itk::QuaternionRigidTransform< float > >::RegisterTransform ();
  itk::TransformFactory< itk::Rigid2DTransform< float > >::RegisterTransform ();
  itk::TransformFactory< itk::Rigid3DPerspectiveTransform< float > >::RegisterTransform ();
  itk::TransformFactory< itk::ScalableAffineTransform< float > >::RegisterTransform ();
  itk::TransformFactory< itk::ScaleLogarithmicTransform< float > >::RegisterTransform ();
  itk::TransformFactory< itk::ScaleSkewVersor3DTransform< float > >::RegisterTransform ();
  itk::TransformFactory< itk::ScaleTransform< float, 2 > >::RegisterTransform ();
  itk::TransformFactory< itk::ScaleTransform< float, 2 > >::RegisterTransform ();
  itk::TransformFactory< itk::ScaleTransform< float, 3 > >::RegisterTransform ();
  itk::TransformFactory< itk::TranslationTransform< float > >::RegisterTransform ();
  itk::TransformFactory< itk::VersorRigid3DTransform< float > >::RegisterTransform ();
  itk::TransformFactory< itk::VersorTransform< float > >::RegisterTransform ();
  itk::TransformFactory< itk::VersorTransform< float > >::RegisterTransform ();
  itk::TransformFactory< itk::ThinPlateR2LogRSplineKernelTransform< float, 2 > >::RegisterTransform ();
  itk::TransformFactory< itk::ThinPlateR2LogRSplineKernelTransform< float, 3 > >::RegisterTransform ();
  itk::TransformFactory< itk::ThinPlateSplineKernelTransform< float, 2 > >::RegisterTransform ();
  itk::TransformFactory< itk::ThinPlateSplineKernelTransform< float, 3 > >::RegisterTransform ();
  itk::TransformFactory< itk::ThinPlateR2LogRSplineKernelTransform< double, 2 > >::RegisterTransform ();
  itk::TransformFactory< itk::ThinPlateR2LogRSplineKernelTransform< double, 3 > >::RegisterTransform ();
  itk::TransformFactory< itk::ThinPlateSplineKernelTransform< double, 2 > >::RegisterTransform ();
  itk::TransformFactory< itk::ThinPlateSplineKernelTransform< double, 3 > >::RegisterTransform ();
}
} //end namespace itk
