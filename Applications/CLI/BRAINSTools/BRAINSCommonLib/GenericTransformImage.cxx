#include <iostream>
#include "GenericTransformImage.h"


namespace itk {

  VersorRigid3DTransformType::Pointer ComputeRigidTransformFromGeneric(const GenericTransformType::ConstPointer genericTransformToWrite)
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
          const VersorRigid3DTransformType::ConstPointer tempInitializerITKTransform
            = dynamic_cast<VersorRigid3DTransformType const * const>( genericTransformToWrite.GetPointer() );
          AssignRigid::ExtractVersorRigid3DTransform(versorRigid, tempInitializerITKTransform);
          }
        else if ( transformFileType == "ScaleVersor3DTransform" )
          {
          const ScaleVersor3DTransformType::ConstPointer tempInitializerITKTransform
            = dynamic_cast<ScaleVersor3DTransformType const * const>( genericTransformToWrite.GetPointer() );
          AssignRigid::ExtractVersorRigid3DTransform(versorRigid, tempInitializerITKTransform);
          }
        else if ( transformFileType == "ScaleSkewVersor3DTransform" )
          {
          const ScaleSkewVersor3DTransformType::ConstPointer tempInitializerITKTransform
            = dynamic_cast<ScaleSkewVersor3DTransformType const * const>( genericTransformToWrite.GetPointer() );
          AssignRigid::ExtractVersorRigid3DTransform(versorRigid, tempInitializerITKTransform);
          }
        else if ( transformFileType == "AffineTransform" )
          {
          const AffineTransformType::ConstPointer tempInitializerITKTransform
            = dynamic_cast<AffineTransformType const * const>( genericTransformToWrite.GetPointer() );
          AssignRigid::ExtractVersorRigid3DTransform(versorRigid, tempInitializerITKTransform);
          }
        /*
        else if(transformFileType == "BSplineDeformableTransform")
        {
        BSplineTransformType::Pointer tempInitializerITKTransform
        = dynamic_cast<BSplineTransformType *>( genericTransformToWrite.GetPointer() );
        //AssignRigid::ExtractVersorRigid3DTransform(versorRigid, tempInitializerITKTransform->GetBulkTransform());
        versorRigid=NULL; //NOT: Perhaps it makes sense to extract the rigid part of the bulk transform.  But that is pretty obscure case.
        return NULL;
        }
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
  int WriteBothTransformsToDisk(const GenericTransformType::ConstPointer genericTransformToWrite, const std::string & outputTransform, const std::string & strippedOutputTransform)
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
        if (outputTransform.size() > 0) // Write out the transform
          {
          itk::WriteTransformToDisk(genericTransformToWrite,outputTransform);
          }
        }
      else if ( transformFileType == "ScaleVersor3DTransform" )
        {
        if (outputTransform.size() > 0) // Write out the transform
          {
          itk::WriteTransformToDisk(genericTransformToWrite,outputTransform);
          }
        }
      else if ( transformFileType == "ScaleSkewVersor3DTransform" )
        {
        if (outputTransform.size() > 0) // Write out the transform
          {
          itk::WriteTransformToDisk(genericTransformToWrite,outputTransform);
          }
        }
      else if ( transformFileType == "AffineTransform" )
        {
        if (outputTransform.size() > 0) // Write out the transform
          {
          itk::WriteTransformToDisk(genericTransformToWrite,outputTransform);
          }
        }
      else if(transformFileType == "BSplineDeformableTransform")
        {
        const BSplineTransformType::ConstPointer tempInitializerITKTransform
          = dynamic_cast<BSplineTransformType const * const>( genericTransformToWrite.GetPointer() );
        if (strippedOutputTransform.size() > 0)
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
          << transformFileType <<
          " not equal to any recognized type VersorRigid3DTransform OR ScaleVersor3DTransform OR ScaleSkewVersor3DTransform OR AffineTransform"
          << std::endl;
        return -1;
        }
      }
    catch ( itk::ExceptionObject & excp )
      {
      std::cout << "[FAILED]" << std::endl;
      std::cerr << "Error while reading the genericTransformToWrite"
        << std::endl;
      std::cerr << excp << std::endl;
      return -1;
      }
    //Should just write out the rigid transform here.
    if (strippedOutputTransform.size() > 0  )
      {
      typedef VersorRigid3DTransformType VersorRigidTransformType;
      VersorRigidTransformType::Pointer versorRigid = itk::ComputeRigidTransformFromGeneric(genericTransformToWrite);
      if(versorRigid.IsNotNull() )
        {
        itk::WriteTransformToDisk(versorRigid.GetPointer(),strippedOutputTransform);
        }
      }
    return 0;
    }

  int WriteStrippedRigidTransformToDisk(const GenericTransformType::ConstPointer genericTransformToWrite, const std::string & strippedOutputTransform)
    {
    return WriteBothTransformsToDisk(genericTransformToWrite, std::string(""), strippedOutputTransform);
    }



  GenericTransformType::Pointer ReadTransformFromDisk(const std::string initialTransform)
    {
    GenericTransformType::Pointer genericTransform=NULL;
    //
    //
    // read in the initial ITKTransform
    //
    //
    TransformReaderType::Pointer transformListReader =  TransformReaderType::New();
    TransformListType currentTransformList;
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
    if (currentTransformList.size() == 1) //Most simple transform types
      {

      //NOTE:  The dynamic casting here circumvents the standard smart pointer behavior.  It is important that
      // by making a new copy and transfering the parameters it is more safe.  Now we only need to ensure
      // that currentTransformList.begin() smart pointer is stable (i.e. not deleted) while the variable
      // temp has a reference to it's internal structure.
      const std::string transformFileType = (*(currentTransformList.begin()))->GetNameOfClass();
      if ( transformFileType == "VersorRigid3DTransform" )
        {
        const VersorRigid3DTransformType::ConstPointer tempInitializerITKTransform
          = dynamic_cast<VersorRigid3DTransformType const * const>( (*(currentTransformList.begin())).GetPointer() );
        VersorRigid3DTransformType::Pointer tempCopy=VersorRigid3DTransformType::New();
        AssignRigid::AssignConvertedTransform(tempCopy,
          tempInitializerITKTransform);
        genericTransform=tempCopy.GetPointer();
        }
      else if ( transformFileType == "ScaleVersor3DTransform" )
        {
        const ScaleVersor3DTransformType::ConstPointer tempInitializerITKTransform
          = dynamic_cast<ScaleVersor3DTransformType const * const>( (*(currentTransformList.begin())).GetPointer() );
        ScaleVersor3DTransformType::Pointer tempCopy=ScaleVersor3DTransformType::New();
        AssignRigid::AssignConvertedTransform(tempCopy,
          tempInitializerITKTransform);
        genericTransform=tempCopy.GetPointer();
        }
      else if ( transformFileType == "ScaleSkewVersor3DTransform" )
        {
        const ScaleSkewVersor3DTransformType::ConstPointer tempInitializerITKTransform
          = dynamic_cast<ScaleSkewVersor3DTransformType const * const>( (*(currentTransformList.begin())).GetPointer() );
        ScaleSkewVersor3DTransformType::Pointer tempCopy=ScaleSkewVersor3DTransformType::New();
        AssignRigid::AssignConvertedTransform(tempCopy,
          tempInitializerITKTransform);
        genericTransform=tempCopy.GetPointer();
        }
      else if ( transformFileType == "AffineTransform" )
        {
        const AffineTransformType::ConstPointer tempInitializerITKTransform
          = dynamic_cast<AffineTransformType const * const>( (*(currentTransformList.begin())).GetPointer() );
        AffineTransformType::Pointer tempCopy=AffineTransformType::New();
        AssignRigid::AssignConvertedTransform(tempCopy,
          tempInitializerITKTransform);
        genericTransform=tempCopy.GetPointer();
        }
      }
    else if (currentTransformList.size() == 2) //A special case for BSplineTransforms
      //To recombine the bulk and the bSpline transforms.
      {
      // transformListReader->GetTransformList();
      TransformListType::const_iterator initializeTransformsListIterator
        = currentTransformList.begin();

      const GenericTransformType::ConstPointer FirstTransform=dynamic_cast<GenericTransformType const * const>
        ((*(initializeTransformsListIterator)).GetPointer() );
      const std::string FirstTransformFileType = FirstTransform->GetNameOfClass();

      initializeTransformsListIterator++;//Increment to next iterator

      const GenericTransformType::ConstPointer SecondTransform=dynamic_cast<GenericTransformType const * const>
        ((*(initializeTransformsListIterator)).GetPointer() );
      const std::string SecondTransformFileType = SecondTransform->GetNameOfClass();

      BSplineTransformType::Pointer outputBSplineTransform
        = BSplineTransformType::New();
      outputBSplineTransform->SetIdentity();

      //Now get the BSpline information
      if ( FirstTransformFileType == "BSplineDeformableTransform" )
        {
        const BSplineTransformType::ConstPointer tempInitializerITKTransform
          = dynamic_cast<BSplineTransformType const * const>
          ( FirstTransform.GetPointer() );
        outputBSplineTransform->SetFixedParameters( tempInitializerITKTransform->GetFixedParameters() );
        outputBSplineTransform->SetParametersByValue( tempInitializerITKTransform->GetParameters() );
        outputBSplineTransform->SetBulkTransform( SecondTransform );
        }
      else if ( SecondTransformFileType == "BSplineDeformableTransform" )
        {
        const BSplineTransformType::ConstPointer tempInitializerITKTransform
          = dynamic_cast<BSplineTransformType const * const>
          ( SecondTransform.GetPointer() );
        outputBSplineTransform->SetFixedParameters( tempInitializerITKTransform->GetFixedParameters() );
        outputBSplineTransform->SetParametersByValue( tempInitializerITKTransform->GetParameters() );
        outputBSplineTransform->SetBulkTransform( FirstTransform );
        }
      else
        {
        std::cout << "[FAILED]" << std::endl;
        std::cerr <<
          "Error using the currentTransformList has two elements, but neither of them are a BSplineDeformableTransform/"
          << std::endl;
        std::cerr <<
          "There should not be more than two transforms in the transform list."
          << std::endl;
        return NULL;
        }
      genericTransform=outputBSplineTransform.GetPointer();
      }
    else if ( currentTransformList.size() > 2 )
      {
      // Error, too many transforms on transform list.
      std::cout << "[FAILED]" << std::endl;
      std::cerr <<
        "Error using the currentTransformList for initializing a BSPlineDeformableTransform:"
        << std::endl;
      std::cerr <<
        "There should not be more than two transforms in the transform list."
        << std::endl;
      return NULL;
      }
    return genericTransform;
    }

  void WriteTransformToDisk( GenericTransformType const * const MyTransform, const std::string TransformFilename )
    {
    /*
     *  Convert the transform to the appropriate assumptions and write it out as requested.
     */
      {
      typedef itk::TransformFileWriter TransformWriterType;
      TransformWriterType::Pointer transformWriter =  TransformWriterType::New();
      transformWriter->SetFileName( TransformFilename.c_str() );

      const std::string transformFileType = MyTransform->GetNameOfClass();
      if ( transformFileType == "BSplineDeformableTransform" )
        {
        const BSplineTransformType::ConstPointer tempInitializerITKTransform
          = dynamic_cast<BSplineTransformType const * const>( MyTransform );

        //NOTE: Order was reversed in order to get BSpline first, then Bulk transform in order to
        //try to appease Slicer3.
        transformWriter->AddTransform( tempInitializerITKTransform );
        //Bulk transform is assumed to be second in Slicer3.
        transformWriter->AddTransform( tempInitializerITKTransform->GetBulkTransform() );
        }
      else
        {
        transformWriter->AddTransform( MyTransform );
        }
      transformWriter->Update();
      //std::cout << "Appended ITK transform to text file: " << TransformFilename.c_str() << std::endl;
      }
    }
}//end namespace itk

