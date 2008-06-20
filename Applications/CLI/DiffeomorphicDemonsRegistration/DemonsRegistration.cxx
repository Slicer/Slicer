/**
 * This is a small tool that shows how to use the diffeomorphic demons algorithm.
 * The user can choose if diffeomorphic or vanilla demons should be used.
 * The user can also choose the type of demons forces, or other parameters;
 *
 * \author Tom Vercauteren, INRIA & Mauna Kea Technologies
 */

#include "itkMultiResolutionPDEDeformableRegistration2.h"
#include "itkFastSymmetricForcesDemonsRegistrationFilter.h"
#include "itkDiffeomorphicDemonsRegistrationFilter.h"
#include "itkWarpImageFilter.h"
#include "itkCommand.h"
#include "itkWarpJacobianDeterminantFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkWarpSmoothnessCalculator.h"
#include "itkGridForwardWarpImageFilter.h"
#include "itkVectorCentralDifferenceImageFunction.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkHistogramMatchingImageFilter.h"
#include "DemonsRegistrationCLP.h"

#include <iostream>

struct arguments
{
   std::string  fixedImageFile;  /* -f option */
   std::string  movingImageFile; /* -m option */
   std::string  outputImageFile; /* -o option */
   std::string  outputFieldFile; /* -O option */
   std::string  trueFieldFile;   /* -r option */
   unsigned int numLevels;       /* -n option */
   std::vector<unsigned int> numIterations;   /* -i option */
   float sigmaDef;               /* -s option */
   float sigmaUp;                /* -g option */
   float maxStepLength;          /* -l option */
   bool useVanillaDem;           /* -a option */
   unsigned int gradientType;    /* -t option */
   bool useHistogramMatching;    /* -e option */
   unsigned int verbosity;       /* -v option */

   arguments () :
      fixedImageFile(""),
      movingImageFile(""),
      outputImageFile("output.mha"),
      outputFieldFile(""),
      trueFieldFile(""),
      numLevels(3u),
      sigmaDef(3.0f),
      sigmaUp(0.0f),
      maxStepLength(2.0f),
      useVanillaDem(false),
      gradientType(0u),
      useHistogramMatching(false),
      verbosity(0u)
   {
      numIterations = std::vector<unsigned int>(numLevels, 10u);
   }

   friend std::ostream& operator<< (std::ostream& o, const arguments& args)
   {
      std::ostringstream osstr;
      for (unsigned int i=0; i<args.numIterations.size(); ++i)
         osstr<<args.numIterations[i]<<" ";
      std::string iterstr = "[ " + osstr.str() + "]";

      std::string gtypeStr;
      switch (args.gradientType)
      {
      case 0:
         gtypeStr = "symmetrized";
         break;
      case 1:
         gtypeStr = "fixed image";
         break;
      case 2:
         gtypeStr = "moving image";
         break;
      default:
         gtypeStr = "unsuported";
      }
         
      return o
         <<"Arguments structure:"<<std::endl
         <<"  Fixed image file: "<<args.fixedImageFile<<std::endl
         <<"  Moving image file: "<<args.movingImageFile<<std::endl
         <<"  Output image file: "<<args.outputImageFile<<std::endl
         <<"  Output field file: "<<args.outputFieldFile<<std::endl
         <<"  True field file: "<<args.trueFieldFile<<std::endl
         <<"  Number of multiresolution levels: "<<args.numLevels<<std::endl
         <<"  Number of demons iterations: "<<iterstr<<std::endl
         <<"  Deformation field sigma: "<<args.sigmaDef<<std::endl
         <<"  Update field sigma: "<<args.sigmaUp<<std::endl
         <<"  Maximum update step length: "<<args.maxStepLength<<std::endl
         <<"  Use vanilla demons: "<<args.useVanillaDem<<std::endl
         <<"  Type of gradient: "<<gtypeStr<<std::endl
         <<"  Use histogram matching: "<<args.useHistogramMatching<<std::endl
         <<"  Verbosity: "<<args.verbosity;
   }
};


/* Display program usage, and exit.
 */
void display_usage( const std::string progname )
{   
   struct arguments defargs = arguments();

   std::ostringstream osstr;
   for (unsigned int i=0; i<defargs.numIterations.size(); ++i)
      osstr<<defargs.numIterations[i]<<" ";
   std::string iterstr = "[ " + osstr.str() + "]";
   
   std::cout<<std::endl;
   std::cout<<progname<<" - register 2 images using demons algorithm"<<std::endl;
   std::cout<<"Usage: "<<progname<<" [OPTION...]"<<std::endl;
   
   std::cout<<"  -f/--fixed_image=STRING    Fixed image filename - mandatory"<<std::endl;
   std::cout<<"  -m/--moving_image=STRING   Moving image filename - mandatory"<<std::endl;
   std::cout<<"  -o/--output_image=STRING   Output image filename - default: "<<defargs.outputImageFile<<std::endl;
   std::cout<<"  -d/--output_field(=STRING) Output field filename - default: OUTPUTIMAGENAME-field.mha"<<std::endl;
   std::cout<<"  -r/--true_field=STRING     True field filename - default: not used"<<std::endl;
   std::cout<<"  -n/--num_levels=UINT       Number of multiresolution levels - default: "<<defargs.numLevels<<std::endl;
   std::cout<<"  -i/--num_iterations=UINTx...xUINT   number of demons iterations - default: "<<iterstr<<std::endl;
   std::cout<<"  -s/--def_field_sigma=FLOAT Smoothing sigma for the deformation field"<<std::endl
            <<"                             at each iteration - default: "<<defargs.sigmaDef<<std::endl;
   std::cout<<"  -g/--up_field_sigma=FLOAT  Smoothing sigma for the update field"<<std::endl
            <<"                             at each iteration - default: "<<defargs.sigmaUp<<std::endl;
   std::cout<<"  -l/--max_step_length=FLOAT Maximum length of an update vector"<<std::endl
            <<"                             (0: no restriction) - default: "<<defargs.maxStepLength<<std::endl;
   std::cout<<"  -a/--use_vanilla_dem       Use the vanilla demons instead of the diffeomorphic one"<<std::endl;
   std::cout<<"  -t/--gradient_type=UINT    Type of gradient used for computing the demons force"<<std::endl
            <<"                             (0 is symmetrized, 1 is fixed image, 2 is moving image) - default: "<<defargs.gradientType<<std::endl;
   std::cout<<"  -e/--use_histogram_matching  Use histogram matching (e.g. for different MRs)"<<std::endl;
   std::cout<<"  -v/--verbose(=UINT)        Verbosity - default: "<<defargs.verbosity<<"; without argurment: 1"<<std::endl;
   std::cout<<"  -h/--help                  Display this message and exit"<<std::endl;

   std::cout<<std::endl;
   std::cout<<"Copyright (c) 2007 Mauna Kea Technologies."<<std::endl;
   std::cout<<"Code: Tom Vercauteren."<<std::endl;
   std::cout<<"Report bugs to <tom.vercauteren@maunakeatech.com>."<<std::endl;
   
    exit( EXIT_FAILURE );
}

//  The following section of code implements a Command observer
//  that will monitor the evolution of the registration process.
//
template <class TPixel=float, unsigned int VImageDimension=3>
class CommandIterationUpdate : public itk::Command 
{
public:
   typedef  CommandIterationUpdate   Self;
   typedef  itk::Command             Superclass;
   typedef  itk::SmartPointer<Self>  Pointer;

   typedef itk::Image< TPixel, VImageDimension > InternalImageType;
   typedef itk::Vector< TPixel, VImageDimension >    VectorPixelType;
   typedef itk::Image<  VectorPixelType, VImageDimension > DeformationFieldType;

   typedef itk::DiffeomorphicDemonsRegistrationFilter<
      InternalImageType,
      InternalImageType,
      DeformationFieldType>   DiffeomorphicDemonsRegistrationFilterType;

   typedef itk::FastSymmetricForcesDemonsRegistrationFilter<
      InternalImageType,
      InternalImageType,
      DeformationFieldType>   FastSymmetricForcesDemonsRegistrationFilterType;

   typedef itk::MultiResolutionPDEDeformableRegistration2<
      InternalImageType, InternalImageType,
      DeformationFieldType, TPixel >   MultiResRegistrationFilterType;

   typedef itk::WarpJacobianDeterminantFilter<
      DeformationFieldType, InternalImageType> JacobianFilterType;
   
   typedef itk::MinimumMaximumImageCalculator<InternalImageType> MinMaxFilterType;

   typedef itk::WarpSmoothnessCalculator<DeformationFieldType>
      SmoothnessCalculatorType;

   typedef itk::VectorCentralDifferenceImageFunction<DeformationFieldType>
      WarpGradientCalculatorType;

   typedef typename WarpGradientCalculatorType::OutputType WarpGradientType;
   
   itkNewMacro( Self );

private:
   std::ofstream m_Fid;
   bool m_headerwritten;
   typename JacobianFilterType::Pointer m_JacobianFilter;
   typename MinMaxFilterType::Pointer m_Minmaxfilter;
   typename SmoothnessCalculatorType::Pointer m_SmothnessCalculator;
   typename DeformationFieldType::ConstPointer m_TrueField;
   typename WarpGradientCalculatorType::Pointer m_TrueWarpGradientCalculator;
   typename WarpGradientCalculatorType::Pointer m_CompWarpGradientCalculator;

public:
   void SetTrueField(const DeformationFieldType * truefield)
   {
      m_TrueField = truefield;

      m_TrueWarpGradientCalculator = WarpGradientCalculatorType::New();
      m_TrueWarpGradientCalculator->SetInputImage( m_TrueField );

      m_CompWarpGradientCalculator =  WarpGradientCalculatorType::New();
   }
   
   void Execute(itk::Object *caller, const itk::EventObject & event)
   {
      Execute( (const itk::Object *)caller, event);
   }

   void Execute(const itk::Object * object, const itk::EventObject & event)
   {
      if( !(itk::IterationEvent().CheckEvent( &event )) )
      {
         return;
      }

      typename DeformationFieldType::ConstPointer deffield = 0;
      unsigned int iter = -1;
      double metricbefore = -1.0;
      
      if ( const DiffeomorphicDemonsRegistrationFilterType * filter = 
           dynamic_cast< const DiffeomorphicDemonsRegistrationFilterType * >( object ) )
      {
         iter = filter->GetElapsedIterations() - 1;
         metricbefore = filter->GetMetric();
         deffield = const_cast<DiffeomorphicDemonsRegistrationFilterType *>
            (filter)->GetDeformationField();
      }
      else if ( const FastSymmetricForcesDemonsRegistrationFilterType * filter = 
           dynamic_cast< const FastSymmetricForcesDemonsRegistrationFilterType * >( object ) )
      {
         iter = filter->GetElapsedIterations() - 1;
         metricbefore = filter->GetMetric();
         deffield = const_cast<FastSymmetricForcesDemonsRegistrationFilterType *>
            (filter)->GetDeformationField();
      }
      else if ( const MultiResRegistrationFilterType * multiresfilter = 
           dynamic_cast< const MultiResRegistrationFilterType * >( object ) )
      {
         std::cout<<"Finished Multi-resolution iteration :"<<multiresfilter->GetCurrentLevel()-1<<std::endl;
         std::cout<<"=============================="<<std::endl<<std::endl;
      }
      else
      {
         return;
      }

      if (deffield)
      {
         std::cout<<iter<<": MSE "<<metricbefore<<" - ";

         double fieldDist = -1.0;
         double fieldGradDist = -1.0;
         double tmp;
         if (m_TrueField)
         {
            typedef itk::ImageRegionConstIteratorWithIndex<DeformationFieldType>
               FieldIteratorType;
            FieldIteratorType currIter(
               deffield, deffield->GetLargestPossibleRegion() );
            FieldIteratorType trueIter(
               m_TrueField, deffield->GetLargestPossibleRegion() );

            m_CompWarpGradientCalculator->SetInputImage( deffield );

            fieldDist = 0.0;
            fieldGradDist = 0.0;
            for ( currIter.GoToBegin(), trueIter.GoToBegin();
                  not currIter.IsAtEnd(); ++currIter, ++trueIter )
            {
               fieldDist += (currIter.Value() - trueIter.Value()).GetSquaredNorm();

               // No need to add Id matrix here as we do a substraction
               tmp = (
                  ( m_CompWarpGradientCalculator->EvaluateAtIndex(currIter.GetIndex())
                    -m_TrueWarpGradientCalculator->EvaluateAtIndex(trueIter.GetIndex())
                     ).GetVnlMatrix() ).frobenius_norm();
               fieldGradDist += tmp*tmp;
            }
            fieldDist = sqrt( fieldDist/ (double)(
                     deffield->GetLargestPossibleRegion().GetNumberOfPixels()) );
            fieldGradDist = sqrt( fieldGradDist/ (double)(
                     deffield->GetLargestPossibleRegion().GetNumberOfPixels()) );
            
            std::cout<<"d(.,true) "<<fieldDist<<" - ";
            std::cout<<"d(.,Jac(true)) "<<fieldGradDist<<" - ";
         }
         
         m_SmothnessCalculator->SetImage( deffield );
         m_SmothnessCalculator->Compute();
         const double harmonicEnergy = m_SmothnessCalculator->GetSmoothness();
         std::cout<<"harmo. "<<harmonicEnergy<<" - ";

         
         m_JacobianFilter->SetInput( deffield );
         m_JacobianFilter->UpdateLargestPossibleRegion();

        
         const unsigned int numPix = m_JacobianFilter->
            GetOutput()->GetLargestPossibleRegion().GetNumberOfPixels();
         
         TPixel* pix_start = m_JacobianFilter->GetOutput()->GetBufferPointer();
         TPixel* pix_end = pix_start + numPix;

         TPixel* jac_ptr;

         // Get percentage of det(Jac) below 0
         unsigned int jacBelowZero(0u);
         for (jac_ptr=pix_start; jac_ptr!=pix_end; ++jac_ptr)
         {
            if ( *jac_ptr<=0.0 ) ++jacBelowZero;
         }
         const double jacBelowZeroPrc = static_cast<double>(jacBelowZero)
            / static_cast<double>(numPix);
         

         // Get min an max jac
         /*
         std::pair<TPixel*, TPixel*> minmax_res =
            boost::minmax_element(pix_start, pix_end);
         */

         //const double minJac = *(minmax_res.first);
         //const double maxJac = *(minmax_res.second);

         const double minJac = *(std::min_element (pix_start, pix_end));
         const double maxJac = *(std::max_element (pix_start, pix_end));

         // Get some quantiles
         // We don't need the jacobian image
         // we can modify/sort it in place
         jac_ptr = pix_start + static_cast<unsigned int>(0.002*numPix);
         std::nth_element(pix_start, jac_ptr, pix_end);
         const double Q002 = *jac_ptr;

         jac_ptr = pix_start + static_cast<unsigned int>(0.01*numPix);
         std::nth_element(pix_start, jac_ptr, pix_end);
         const double Q01 = *jac_ptr;

         jac_ptr = pix_start + static_cast<unsigned int>(0.99*numPix);
         std::nth_element(pix_start, jac_ptr, pix_end);
         const double Q99 = *jac_ptr;

         jac_ptr = pix_start + static_cast<unsigned int>(0.998*numPix);
         std::nth_element(pix_start, jac_ptr, pix_end);
         const double Q998 = *jac_ptr;
         

         std::cout<<"max|Jac| "<<maxJac<<" - "
                  <<"min|Jac| "<<minJac<<" - "
                  <<"ratio(|Jac|<=0) "<<jacBelowZeroPrc<<std::endl;
         
         

         if (this->m_Fid.is_open())
         {
            if (not m_headerwritten)
            {
               this->m_Fid<<"Iteration"
                          <<", MSE before"
                          <<", Harmonic energy"
                          <<", min|Jac|"
                          <<", 0.2% |Jac|"
                          <<", 01% |Jac|"
                          <<", 99% |Jac|"
                          <<", 99.8% |Jac|"
                          <<", max|Jac|"
                          <<", ratio(|Jac|<=0)";
               
               if (m_TrueField)
               {
                  this->m_Fid<<", dist(warp,true warp)"
                             <<", dist(Jac,true Jac)";
               }
               
               this->m_Fid<<std::endl;
               
               m_headerwritten = true;
            }
            
            this->m_Fid<<iter
                       <<", "<<metricbefore
                       <<", "<<harmonicEnergy
                       <<", "<<minJac
                       <<", "<<Q002
                       <<", "<<Q01
                       <<", "<<Q99
                       <<", "<<Q998
                       <<", "<<maxJac
                       <<", "<<jacBelowZeroPrc;

            if (m_TrueField)
            {
               this->m_Fid<<", "<<fieldDist
                          <<", "<<fieldGradDist;
            }
            
            this->m_Fid<<std::endl;
         }
      }
   }
   
protected:   
   CommandIterationUpdate() :
      m_Fid( "metricvalues.csv" ),
      m_headerwritten(false)
   {
      m_JacobianFilter = JacobianFilterType::New();
      m_JacobianFilter->SetUseImageSpacing( true );
      m_JacobianFilter->ReleaseDataFlagOn();
      
      m_Minmaxfilter = MinMaxFilterType::New();

      m_SmothnessCalculator = SmoothnessCalculatorType::New();

      m_TrueField = 0;
      m_TrueWarpGradientCalculator = 0;
      m_CompWarpGradientCalculator = 0;
   };

   ~CommandIterationUpdate()
   {
      this->m_Fid.close();
   }
};



template <unsigned int Dimension>
void DemonsRegistrationFunction( arguments args )
{
   // Declare the types of the images (float or double only)
   typedef float PixelType;

   typedef itk::Image< PixelType, Dimension >  ImageType;

   // Images we use
   typename ImageType::Pointer fixedImage = 0;
   typename ImageType::Pointer movingImage = 0;

   {//for mem allocations
   
   // Set up the file readers
   typedef typename itk::ImageFileReader< ImageType > FixedImageReaderType;
   typedef typename itk::ImageFileReader< ImageType > MovingImageReaderType;

   typename FixedImageReaderType::Pointer fixedImageReader   = FixedImageReaderType::New();
   typename MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();
   
   fixedImageReader->SetFileName( args.fixedImageFile.c_str() );
   movingImageReader->SetFileName( args.movingImageFile.c_str() );


   // Update the reader
   try
   {
      fixedImageReader->Update();
      movingImageReader->Update();
   }
   catch( itk::ExceptionObject& err )
   {
      std::cout << "Could not read one of the input images." << std::endl;
      std::cout << err << std::endl;
      exit( EXIT_FAILURE );
   }

   

   if (!args.useHistogramMatching)
   {
      fixedImage = fixedImageReader->GetOutput();
      fixedImage->DisconnectPipeline();
      movingImage = movingImageReader->GetOutput();
      movingImage->DisconnectPipeline();
   }
   else
   {   
      typedef typename itk::HistogramMatchingImageFilter
         <ImageType, ImageType> MatchingFilterType;
      typename MatchingFilterType::Pointer matcher = MatchingFilterType::New();

      matcher->SetInput( movingImageReader->GetOutput() );
      matcher->SetReferenceImage( fixedImageReader->GetOutput() );

      matcher->SetNumberOfHistogramLevels( 1024 );
      matcher->SetNumberOfMatchPoints( 7 );
      matcher->ThresholdAtMeanIntensityOn();

      // Update the matcher
      try
      {
         matcher->Update();
      }
      catch( itk::ExceptionObject& err )
      {
         std::cout << "Could not match the input images." << std::endl;
         std::cout << err << std::endl;
         exit( EXIT_FAILURE );
      }

      movingImage = matcher->GetOutput();
      movingImage->DisconnectPipeline();
      
      fixedImage = fixedImageReader->GetOutput();
      fixedImage->DisconnectPipeline();
   }

   }//end for mem allocations


   // Set up the demons filter output
   typedef itk::Vector< PixelType, Dimension >    VectorPixelType;
   typedef typename itk::Image<  VectorPixelType, Dimension > DeformationFieldType;

   typename DeformationFieldType::Pointer defField = 0;

   {//for mem allocations
   
   // Set up the demons filter
   typedef typename itk::PDEDeformableRegistrationFilter
      < ImageType, ImageType, DeformationFieldType>   BaseRegistrationFilterType;
   typename BaseRegistrationFilterType::Pointer filter;
   
   if ( !args.useVanillaDem )
   {
      typedef typename itk::DiffeomorphicDemonsRegistrationFilter
         < ImageType, ImageType, DeformationFieldType>   ActualRegistrationFilterType;
      typedef typename ActualRegistrationFilterType::GradientType GradientType;
      
      typename ActualRegistrationFilterType::Pointer actualfilter = ActualRegistrationFilterType::New();

      actualfilter->SetMaximumUpdateStepLength( args.maxStepLength );
      actualfilter->SetUseGradientType( static_cast<GradientType>(args.gradientType) );
      filter = actualfilter;
   }
   else
   {
      typedef typename itk::FastSymmetricForcesDemonsRegistrationFilter
         < ImageType, ImageType, DeformationFieldType>   ActualRegistrationFilterType;
      typedef typename ActualRegistrationFilterType::GradientType GradientType;
      
      typename ActualRegistrationFilterType::Pointer actualfilter = ActualRegistrationFilterType::New();
      
      actualfilter->SetMaximumUpdateStepLength( args.maxStepLength );
      actualfilter->SetUseGradientType( static_cast<GradientType>(args.gradientType) );
      filter = actualfilter;
   }

   if ( args.sigmaDef > 0.1 )
   {
      filter->SmoothDeformationFieldOn();
      filter->SetStandardDeviations( args.sigmaDef );
   }
   else
      filter->SmoothDeformationFieldOff();

   if ( args.sigmaUp > 0.1 )
   {
      filter->SmoothUpdateFieldOn();
      filter->SetUpdateFieldStandardDeviations( args.sigmaUp );
   }
   else
      filter->SmoothUpdateFieldOff();

   //filter->SetIntensityDifferenceThreshold( 0.001 );

   if ( args.verbosity > 0 )
   {      
      // Create the Command observer and register it with the registration filter.
      typename CommandIterationUpdate<PixelType, Dimension>::Pointer observer =
         CommandIterationUpdate<PixelType, Dimension>::New();

      if (!args.trueFieldFile.empty())
      {
         if (args.numLevels>1)
         {
            std::cout << "You cannot compare the results with a true filed in a multiresolution setting yet." << std::endl;
            exit( EXIT_FAILURE );
         }
         
         // Set up the file readers
         typedef itk::ImageFileReader< DeformationFieldType > FieldReaderType;
         typename FieldReaderType::Pointer fieldReader = FieldReaderType::New();
         fieldReader->SetFileName(  args.trueFieldFile.c_str() );

         // Update the reader
         try
         {
            fieldReader->Update();
         }
         catch( itk::ExceptionObject& err )
         {
            std::cout << "Could not read the true field." << std::endl;
            std::cout << err << std::endl;
            exit( EXIT_FAILURE );
         }

         observer->SetTrueField( fieldReader->GetOutput() );
      }
      
      filter->AddObserver( itk::IterationEvent(), observer );
   }

   // Set up the multi-resolution filter
   typedef typename itk::MultiResolutionPDEDeformableRegistration2<
      ImageType, ImageType, DeformationFieldType, PixelType >   MultiResRegistrationFilterType;
   typename MultiResRegistrationFilterType::Pointer multires = MultiResRegistrationFilterType::New();

   multires->SetRegistrationFilter( filter );
   multires->SetNumberOfLevels( args.numLevels );
   
   multires->SetNumberOfIterations( &args.numIterations[0] );

   multires->SetFixedImage( fixedImage );
   multires->SetMovingImage( movingImage );


   if ( args.verbosity > 0 )
   {
      // Create the Command observer and register it with the registration filter.
      typename CommandIterationUpdate<PixelType, Dimension>::Pointer multiresobserver =
         CommandIterationUpdate<PixelType, Dimension>::New();
      multires->AddObserver( itk::IterationEvent(), multiresobserver );
   }
   

   
   // Compute the deformation field
   try
   {
      multires->UpdateLargestPossibleRegion();
   }
   catch( itk::ExceptionObject& err )
   {
      std::cout << "Unexpected error." << std::endl;
      std::cout << err << std::endl;
      exit( EXIT_FAILURE );
   }


   // The outputs
   defField = multires->GetOutput();
   defField->DisconnectPipeline();

   }//end for mem allocations

   
   // warp the result
   typedef itk::WarpImageFilter
      < ImageType, ImageType, DeformationFieldType >  WarperType;
   typename WarperType::Pointer warper = WarperType::New();
   warper->SetInput( movingImage );
   warper->SetOutputSpacing( fixedImage->GetSpacing() );
   warper->SetOutputOrigin( fixedImage->GetOrigin() );
   warper->SetDeformationField( defField );

   
   // Write warped image out to file
   typedef PixelType OutputPixelType;
   typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
   typedef itk::CastImageFilter
      < ImageType, OutputImageType > CastFilterType;
   typedef itk::ImageFileWriter< OutputImageType >  WriterType;
   
   typename WriterType::Pointer      writer =  WriterType::New();
   typename CastFilterType::Pointer  caster =  CastFilterType::New();
   writer->SetFileName( args.outputImageFile.c_str() );
   caster->SetInput( warper->GetOutput() );
   writer->SetInput( caster->GetOutput()   );
   writer->SetUseCompression( true );
   
   try
   {
      writer->Update();
   }
   catch( itk::ExceptionObject& err )
   {
      std::cout << "Unexpected error." << std::endl;
      std::cout << err << std::endl;
      exit( EXIT_FAILURE );
   }
   
   
   // Write deformation field
   if (!args.outputFieldFile.empty())
   {
      // Write the deformation field as an image of vectors.
      // Note that the file format used for writing the deformation field must be
      // capable of representing multiple components per pixel. This is the case
      // for the MetaImage and VTK file formats for example.
      typedef itk::ImageFileWriter< DeformationFieldType > FieldWriterType;
      typename FieldWriterType::Pointer fieldWriter = FieldWriterType::New();
      fieldWriter->SetFileName(  args.outputFieldFile.c_str() );
      fieldWriter->SetInput( defField );
      fieldWriter->SetUseCompression( true );
      
      try
      {
         fieldWriter->Update();
      }
      catch( itk::ExceptionObject& err )
      {
         std::cout << "Unexpected error." << std::endl;
         std::cout << err << std::endl;
         exit( EXIT_FAILURE );
      }
   }


   
   // Create and write warped grid image
   if ( args.verbosity > 0 )
   {
      typedef itk::Image< unsigned char, Dimension > GridImageType;
      typename GridImageType::Pointer gridImage = GridImageType::New();
      gridImage->SetRegions( movingImage->GetRequestedRegion() );
      gridImage->SetOrigin( movingImage->GetOrigin() );
      gridImage->SetSpacing( movingImage->GetSpacing() );
      gridImage->Allocate();
      gridImage->FillBuffer(0);
      
      typedef itk::ImageRegionIteratorWithIndex<GridImageType> GridImageIteratorWithIndex;
      GridImageIteratorWithIndex itergrid = GridImageIteratorWithIndex(
         gridImage, gridImage->GetRequestedRegion() );

      const int gridspacing(8);
      for (itergrid.GoToBegin(); !itergrid.IsAtEnd(); ++itergrid)
      {
         itk::Index<Dimension> index = itergrid.GetIndex();

         if (Dimension==2 or Dimension==3)
         {
            // produce an xy grid for all z
            if ( (index[0]%gridspacing)==0 or
                 (index[1]%gridspacing)==0 )
            {
               itergrid.Set( itk::NumericTraits<unsigned char>::max() );
            }
         }
         else
         {
            unsigned int numGridIntersect = 0;
            for( unsigned int dim = 0; dim < Dimension; dim++ )
               numGridIntersect += ( (index[dim]%gridspacing)==0 );
            if (numGridIntersect >= (Dimension-1))
               itergrid.Set( itk::NumericTraits<unsigned char>::max() );
         }
      }

      typedef itk::WarpImageFilter
         < GridImageType, GridImageType, DeformationFieldType >  GridWarperType;
      typename GridWarperType::Pointer gridwarper = GridWarperType::New();
      gridwarper->SetInput( gridImage );
      gridwarper->SetOutputSpacing( fixedImage->GetSpacing() );
      gridwarper->SetOutputOrigin( fixedImage->GetOrigin() );
      gridwarper->SetDeformationField( defField );
      
      // Write warped grid to file
      typedef itk::ImageFileWriter< GridImageType >  GridWriterType;
   
      typename GridWriterType::Pointer      gridwriter =  GridWriterType::New();
      gridwriter->SetFileName( "WarpedGridImage.mha" );
      gridwriter->SetInput( gridwarper->GetOutput()   );
      gridwriter->SetUseCompression( true );
   
      try
      {
         gridwriter->Update();
      }
      catch( itk::ExceptionObject& err )
      {
         std::cout << "Unexpected error." << std::endl;
         std::cout << err << std::endl;
         exit( EXIT_FAILURE );
      }
   }


   // Create and write forewardwarped grid image
   if ( args.verbosity > 0 )
   {
      typedef itk::Image< unsigned char, Dimension > GridImageType;
      typedef itk::GridForwardWarpImageFilter<DeformationFieldType, GridImageType> GridForwardWarperType;

      typename GridForwardWarperType::Pointer fwWarper = GridForwardWarperType::New();
      fwWarper->SetInput(defField);
      fwWarper->SetForegroundValue( itk::NumericTraits<unsigned char>::max() );

      // Write warped grid to file
      typedef itk::ImageFileWriter< GridImageType >  GridWriterType;
   
      typename GridWriterType::Pointer      gridwriter =  GridWriterType::New();
      gridwriter->SetFileName( "ForwardWarpedGridImage.mha" );
      gridwriter->SetInput( fwWarper->GetOutput()   );
      gridwriter->SetUseCompression( true );
   
      try
      {
         gridwriter->Update();
      }
      catch( itk::ExceptionObject& err )
      {
         std::cout << "Unexpected error." << std::endl;
         std::cout << err << std::endl;
         exit( EXIT_FAILURE );
      }
   }



 
   // compute final metric
   if ( args.verbosity > 0 )
   {
      double finalSSD = 0.0;
      typedef itk::ImageRegionConstIterator<ImageType> ImageConstIterator;

      ImageConstIterator iterfix = ImageConstIterator(
         fixedImage, fixedImage->GetRequestedRegion() );
      
      ImageConstIterator itermovwarp = ImageConstIterator(
         warper->GetOutput(), fixedImage->GetRequestedRegion() );
      
      for (iterfix.GoToBegin(), itermovwarp.GoToBegin(); !iterfix.IsAtEnd(); ++iterfix, ++itermovwarp)
      {
         finalSSD += vnl_math_sqr( iterfix.Get() - itermovwarp.Get() );
      }

      const double finalMSE = finalSSD / static_cast<double>(
         fixedImage->GetRequestedRegion().GetNumberOfPixels() );
      std::cout<<"MSE fixed image vs. warped moving image: "<<finalMSE<<std::endl;
   }


   
   // Create and write jacobian of the deformation field
   if ( args.verbosity > 0 )
   {
      typedef itk::WarpJacobianDeterminantFilter
         <DeformationFieldType, ImageType> JacobianFilterType;
      typename JacobianFilterType::Pointer jacobianFilter = JacobianFilterType::New();
      jacobianFilter->SetInput( defField );
      jacobianFilter->SetUseImageSpacing( true );

      writer->SetFileName( "TransformJacobianDeteminant.mha" );
      caster->SetInput( jacobianFilter->GetOutput() );
      writer->SetInput( caster->GetOutput()   );
      writer->SetUseCompression( true );
      
      try
      {
         writer->Update();
      }
      catch( itk::ExceptionObject& err )
      {
         std::cout << "Unexpected error." << std::endl;
         std::cout << err << std::endl;
         exit( EXIT_FAILURE );
      }

      typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxFilterType;
      typename MinMaxFilterType::Pointer minmaxfilter = MinMaxFilterType::New();
      minmaxfilter->SetImage( jacobianFilter->GetOutput() );
      minmaxfilter->Compute();
      std::cout<<"Minimum of the determinant of the Jacobian of the warp: "
               <<minmaxfilter->GetMinimum()<<std::endl;
      std::cout<<"Maximum of the determinant of the Jacobian of the warp: "
               <<minmaxfilter->GetMaximum()<<std::endl;
   }

   
}


int main( int argc, char *argv[] )
{  
   PARSE_ARGS;

   // FIXME uncomment for debug only
   // itk::MultiThreader::SetGlobalDefaultNumberOfThreads(1);

   // Get the image dimension
   
   itk::ImageIOBase::Pointer imageIO = itk::ImageIOFactory::CreateImageIO(
   demonsTargetVolume.c_str(), itk::ImageIOFactory::ReadMode);
   imageIO->SetFileName(demonsTargetVolume.c_str());
   imageIO->ReadImageInformation();

   // set up the args structure from the parsed input
   arguments args;
   args.fixedImageFile = demonsTargetVolume;
   args.movingImageFile = demonsMovingVolume;
   args.outputImageFile = demonsResampledVolume;
   args.outputFieldFile = demonsDeformationVolume;
   // not used
//   args.trueFieldFile = ; 
   args.numLevels = levels;
   for (unsigned int i = 0; i < iteration.size(); i++)
     {
     args.numIterations.push_back(iteration[i]);
     }
   args.sigmaDef = smoothing;
   args.sigmaUp = smoothingUp;
   args.maxStepLength = maxStepLength;
   args.useVanillaDem = turnOffDiffeomorph;
   args.gradientType = gradientType;
   args.useHistogramMatching = normalization;
   args.verbosity = verbose;
   
   switch ( imageIO->GetNumberOfDimensions() )
   {
   case 2:
      DemonsRegistrationFunction<2>(args);
      break;
   case 3:
      DemonsRegistrationFunction<3>(args);
      break;
   default:
      std::cerr << "Unsuported dimension" << std::endl;
      exit( EXIT_FAILURE );
   }
    
   return EXIT_SUCCESS;
}
