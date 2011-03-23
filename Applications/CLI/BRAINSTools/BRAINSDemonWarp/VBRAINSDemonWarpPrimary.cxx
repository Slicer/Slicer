#include "BRAINSDemonWarpTemplates.h"
#include "VBRAINSDemonWarpPrimaryCLP.h"

#ifdef USE_DEBUG_IMAGE_VIEWER
/*************************
  * Have a global variable to
  * add debugging information.
  */
DebugImageViewerClient DebugImageDisplaySender;
#endif

#ifdef WIN32
#define MODULE_EXPORT __declspec(dllexport)
#else
#define MODULE_EXPORT
#endif

extern "C" MODULE_EXPORT int VBRAINSDemonWarpPrimary(int, char* []);


int VBRAINSDemonWarpPrimary(int argc, char *argv[])
{
  struct BRAINSDemonWarpAppParameters command;

    {
    PARSE_ARGS;
#ifdef USE_DEBUG_IMAGE_VIEWER
    DebugImageDisplaySender.SetEnabled(UseDebugImageViewer);
#endif

    command.registrationFilterType = registrationFilterType;
    command.vectorMovingVolume = movingVolume;
    command.vectorFixedVolume = fixedVolume;
    command.outputVolume = outputVolume;
    command.outputDeformationFieldVolume = outputDeformationFieldVolume;
    command.inputPixelType = inputPixelType;
    command.outputPixelType = outputPixelType;
    command.outputDisplacementFieldPrefix = outputDisplacementFieldPrefix;
    command.outputCheckerboardVolume = outputCheckerboardVolume;
    command.outputNormalized = outputNormalized;
    command.outputDebug = outputDebug;
    command.makeBOBF = makeBOBF;
    command.fixedBinaryVolume = fixedBinaryVolume;
    command.movingBinaryVolume = movingBinaryVolume;
    command.lowerThresholdForBOBF = lowerThresholdForBOBF;
    command.upperThresholdForBOBF = upperThresholdForBOBF;
    command.backgroundFillValue = backgroundFillValue;
    /* NOT YET IMPLEMENTED
      * command.forceCoronalZeroOrigin = forceCoronalZeroOrigin;
      * command.movingLandmarks = movingLandmarks;
      * command.fixedLandmarks = fixedLandmarks;
      * command.initializeWithFourier = initializeWithFourier;
      */
    command.initializeWithDeformationField = initializeWithDeformationField;
    command.initializeWithTransform = initializeWithTransform;

    command.histogramMatch = histogramMatch;
    command.numberOfHistogramLevels = numberOfHistogramBins;
    command.numberOfMatchPoints = numberOfMatchPoints;
    command.numberOfLevels = numberOfPyramidLevels;
    command.numberOfIterations.SetSize(numberOfPyramidLevels);
    command.weightFactors.SetSize( movingVolume.size() );

    command.maxStepLength = maxStepLength;
    command.gradientType = gradientType;
    command.smoothDeformationFieldSigma = smoothDeformationFieldSigma;
    command.smoothingUp = smoothingUp;
    command.interpolationMode = interpolationMode;

    for ( int i = 0; i < numberOfPyramidLevels; i++ )
      {
      command.numberOfIterations[i] = arrayOfPyramidLevelIterations[i];
      }

    for ( unsigned int i = 0; i < movingVolume.size(); i++ )
      {
      command.weightFactors[i] = weightFactors[i];
      if ( command.weightFactors[i] == 0.0 )
        {
        command.vectorMovingVolume.erase(command.vectorMovingVolume.begin() + i);
        command.vectorFixedVolume.erase(command.vectorFixedVolume.begin() + i);
        }
      }

    for ( int i = 0; i < 3; i++ )
      {
      command.theMovingImageShrinkFactors[i] = minimumMovingPyramid[i];
      command.theFixedImageShrinkFactors[i] = minimumFixedPyramid[i];
      }

    for ( int i = 0; i < 3; i++ )
      {
      command.checkerboardPatternSubdivisions[i] =
        checkerboardPatternSubdivisions[i];
      command.seedForBOBF[i] = seedForBOBF[i];
      command.neighborhoodForBOBF[i] = neighborhoodForBOBF[i];
      command.medianFilterSize[i] = medianFilterSize[i];
      }
    }
  //  bool debug=true;
  if ( command.outputDebug )
    {
    for ( unsigned int i = 0; i < command.vectorMovingVolume.size(); ++i )
      {
      std::cout << "                   movingVolume: "
                << command.vectorMovingVolume[i] << std::endl
                << "                   weightFactor: "
                << command.weightFactors[i] << std::endl;
      }
    for ( unsigned int i = 0; i < command.vectorFixedVolume.size(); ++i )
      {
      std::cout << "                    fixedVolume: "
                << command.vectorFixedVolume[i] << std::endl;
      }
    std::cout
    << "   outputDeformationFieldVolume: "
    << command.outputDeformationFieldVolume << std::endl
    << "                 inputPixelType: " << command.inputPixelType
    << std::endl
    << "                outputPixelType: " << command.outputPixelType
    << std::endl
    << "  outputDisplacementFieldPrefix: "
    << command.outputDisplacementFieldPrefix << std::endl
    << "       outputCheckerboardVolume: "
    << command.outputCheckerboardVolume << std::endl
    << "checkerboardPatternSubdivisions: "
    << command.checkerboardPatternSubdivisions << std::endl
    << "               outputNormalized: " << command.outputNormalized
    << std::endl
    << "                    outputDebug: " << command.outputDebug
    << std::endl
    << "                       makeBOBF: " << command.makeBOBF << std::endl
    << "              fixedBinaryVolume: " << command.fixedBinaryVolume
    << std::endl
    << "             movingBinaryVolume: " << command.movingBinaryVolume
    << std::endl
    << "          lowerThresholdForBOBF: "
    << command.lowerThresholdForBOBF << std::endl
    << "          upperThresholdForBOBF: "
    << command.upperThresholdForBOBF << std::endl
    << "            backgroundFillValue: " << command.backgroundFillValue
    << std::endl
    << "                    seedForBOBF: " << command.seedForBOBF
    << std::endl
    << "            neighborhoodForBOBF: " << command.neighborhoodForBOBF
    << std::endl
    << "               medianFilterSize: " << command.medianFilterSize
    << std::endl
    /* NOT YET IMPLEMENTED
      * << "        movingLandmarks: " << command.movingLandmarks << std::endl
      * << "         fixedLandmarks: " << command.fixedLandmarks << std::endl
      * << "     initializeWithFourier: " << command.initializeWithFourier
      */
    << "  initializeWithDeformationField: "
    << command.initializeWithDeformationField  << std::endl
    << std::endl
    << "       initializeWithTransform: "
    << command.initializeWithTransform << std::endl
    << "                    gradientType: " << command.gradientType
    << std::endl
    << "                   maxStepLength: " << command.maxStepLength
    << std::endl
    << "     smoothDeformationFieldSigma: "
    << command.smoothDeformationFieldSigma << std::endl
    << "                     smoothingUp: " << command.smoothingUp
    << std::endl
    << "                   histogramMatch: " << command.histogramMatch
    << std::endl
    << "                histogram levels: "
    << command.numberOfHistogramLevels << std::endl
    << "                 matching points: " << command.numberOfMatchPoints
    << std::endl
    ;
    }

  bool violated = false;
  if ( command.vectorMovingVolume.size() == 0 )
    {
    violated = true; std::cout << "  --movingVolume Required! "  << std::endl;
    }
  if ( command.vectorFixedVolume.size() == 0 )
    {
    violated = true; std::cout << "  --fixedVolume Required! "  << std::endl;
    }
  if ( command.vectorMovingVolume.size() != command.vectorFixedVolume.size() )
    {
    violated = true; std::cout
    << "Volumes of fixed images and moving images are not matched!" << std::endl;
    }

  if ( ( command.checkerboardPatternSubdivisions[0] == 0 )
       || ( command.checkerboardPatternSubdivisions[1] == 0 )
       || ( command.checkerboardPatternSubdivisions[2] == 0 ) )
    {
    std::cout
    <<
    "Invalid Patameters. The value of checkboardPatternSubdivisions should not be zero!"
    << std::endl;
    exit(-1);
    }

  // if (outputVolume.size() == 0) { violated = true; std::cout << "
  //  --outputVolume Required! "  << std::endl; }
  // if (outputDeformationFieldVolume.size() == 0) { violated = true; std::cout
  // << "  --outputDeformationFieldVolume Required! "  << std::endl; }
  // if (registrationParameters.size() == 0) { violated = true; std::cout << "
  //  --registrationParameters Required! "  << std::endl; }
  // if (inputPixelType.size() == 0) { violated = true; std::cout << "
  //  --inputPixelType Required! "  << std::endl; }
  if ( violated )
    {
    exit(1);
    }

  // Test if the input data type is valid
  if ( command.inputPixelType != "" )
    {
    // check to see if valid type
    if ( ( CompareNoCase( command.inputPixelType.c_str(), std::string("uchar") ) )
         && ( CompareNoCase( command.inputPixelType.c_str(), std::string("short") ) )
         && ( CompareNoCase( command.inputPixelType.c_str(),
                             std::string("ushort") ) )
         && ( CompareNoCase( command.inputPixelType.c_str(), std::string("int") ) )
         && ( CompareNoCase( command.inputPixelType.c_str(), std::string("float") ) )
#ifdef _USE_UNCOMMON_TYPES  // This is commented out because it causes too many
                            // segments in one object file for the intel
                            // compiler
         &&
         ( CompareNoCase( command.inputPixelType.c_str(), std::string("uint") ) )
         && ( CompareNoCase( command.inputPixelType.c_str(),
                             std::string("double") ) )
#endif
          )
      {
      std::cout
      << "Error. Invalid data type string specified with --inputPixelType!"
      << std::endl;
      std::cout << "Use one of the following:" << std::endl;
      PrintDataTypeStrings();
      exit(-1);
      }
    }

  if ( command.outputPixelType != "" )
    {
    // check to see if valid type
    if ( ( CompareNoCase( command.outputPixelType.c_str(),
                          std::string("uchar") ) )
         &&            ( CompareNoCase( command.outputPixelType.c_str(),
                                        std::string("SHORT") ) )
         && ( CompareNoCase( command.outputPixelType.c_str(),
                             std::string("ushort") ) )
         && ( CompareNoCase( command.outputPixelType.c_str(), std::string("int") ) )
         && ( CompareNoCase( command.outputPixelType.c_str(),
                             std::string("float") ) )
#ifdef _USE_UNCOMMON_TYPES  // This is commented out because it causes too many
                            // segments in one object file for the intel
                            // compiler
         &&
         ( CompareNoCase( command.outputPixelType.c_str(), std::string("uint") ) )
         && ( CompareNoCase( command.outputPixelType.c_str(),
                             std::string("double") ) )
#endif
          )
      {
      std::cout
      << "Error. Invalid data type string specified with --outputPixelType!"
      << std::endl;
      std::cout << "Use one of the following:" << std::endl;
      PrintDataTypeStrings();
      exit(-1);
      }
    }

  // Call the process output data type function based on the input data type.

  if ( CompareNoCase ( command.inputPixelType, std::string ("uchar") ) == 0 )
    {
    VectorProcssOutputType< unsigned char >(command);
    }
  else if ( CompareNoCase ( command.inputPixelType,
                            std::string ("short") ) == 0 )
    {
    VectorProcssOutputType< short >(command);
    }
  else if ( CompareNoCase ( command.inputPixelType,
                            std::string ("ushort") ) == 0 )
    {
    VectorProcssOutputType< unsigned short >(command);
    }
  else if ( CompareNoCase ( command.inputPixelType, std::string ("int") ) == 0 )
    {
    VectorProcssOutputType< int >(command);
    }
  else if ( CompareNoCase ( command.inputPixelType,
                            std::string ("float") ) == 0 )
    {
    VectorProcssOutputType< float >(command);
    }
#ifdef _USE_UNCOMMON_TYPES  // This is commented out because it causes too many
                            // segments in one object file for the intel
                            // compiler
  else if ( CompareNoCase ( command.inputPixelType, std::string ("uint") ) == 0 )
    {
    VectorProcssOutputType< unsigned int >(command);
    }
  else if ( CompareNoCase ( command.inputPixelType,
                            std::string ("double") ) == 0 )
    {
    VectorProcssOutputType< double >(command);
    }
#endif
  else
    {
    std::cout
    << "Error. Invalid data type for --inputPixelType!  Use one of these:"
    << std::endl;
    PrintDataTypeStrings ();
    exit (-1);
    }
  return 0;
}
