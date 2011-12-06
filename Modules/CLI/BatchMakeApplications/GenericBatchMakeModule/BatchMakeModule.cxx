#include <iostream>
#include <fstream>
#include <string>

// BatchMake
#include <bmScriptParser.h>
#include <ApplicationWrapper.h>

#include <itksys/Glob.hxx>
#include <itksys/SystemTools.hxx>
#include <itksys/Process.h>

// SlicerBatchMakeConfig shall include the CLP.h file
// and define the program name for SetAppOption( ... @progName )
#include "BatchMakeModuleConfig.h"

TCLAP::Arg * GetParameter( const std::list<TCLAP::Arg *>& parameters,
                           const std::string& parameterName )
{
  std::list<TCLAP::Arg *>::const_iterator it;
  std::list<TCLAP::Arg *>::const_iterator begin = parameters.begin();
  std::list<TCLAP::Arg *>::const_iterator end = parameters.end();
  // Find the XXXDirectory parameters
  for( it = begin; it != end; ++it )
    {
    if( (*it)->getName() == parameterName )
      {
      return *it;
      }
    }
  return NULL;
}

std::list<TCLAP::Arg *> GetDirectoryMaskParameters(
  const std::list<TCLAP::Arg *>& parameters )
{
  std::list<TCLAP::Arg *>                 res;
  std::list<TCLAP::Arg *>::const_iterator it;
  std::list<TCLAP::Arg *>::const_iterator begin = parameters.begin();
  std::list<TCLAP::Arg *>::const_iterator end = parameters.end();
  // Find the XXX_Directory parameters
  for( it = begin; it != end; ++it )
    {
    std::size_t pos = (*it)->getName().rfind( "_Directory" );
    if( pos == std::string::npos ||    // we found "_Directory"
        pos == 0 ||                    // "XXX" is not empty
        pos != (*it)->getName().size() // "_Directory" is after "XXX"
        - std::string("_Directory").size() )
      { // not a valid XXX_Directory parameter, keep searching
      continue;
      }
    // Get the XXX from XXX_Directory
    std::string paramName = (*it)->getName().substr( 0, pos );
    // Find the matching XXX_Mask parameter
    std::string mask = paramName + "_Mask";
    // TCLAP::Arg::getName returns the flag of the argument and
    // and not the name, usually the difference is in the case of
    // the first char.
    mask[0] = tolower( mask[0] );

    std::list<TCLAP::Arg *>::const_iterator it2;
    for( it2 = begin; it2 != end; ++it2 )
      {
      if( (*it2)->getName() != mask )
        { // not a parameter that match XXX_Mask where XXX is from XXX_Directory
        continue;
        }
      res.push_back( *it );
      res.push_back( *it2 );
      break;
      }
    }
  return res;
}

std::list<TCLAP::Arg *> GetMinMaxStepParameters(
  const std::list<TCLAP::Arg *>& parameters )
{
  std::list<TCLAP::Arg *>                 res;
  std::list<TCLAP::Arg *>::const_iterator it;
  std::list<TCLAP::Arg *>::const_iterator begin = parameters.begin();
  std::list<TCLAP::Arg *>::const_iterator end = parameters.end();
  // Find the XXX_Min parameters
  for( it = begin; it != end; ++it )
    {
    std::size_t pos = (*it)->getName().rfind( "_Min" );
    if( pos == std::string::npos ||    // we found "_Min"
        pos == 0 ||                    // "XXX" is not empty
        pos != (*it)->getName().size() // "_Min" is after "XXX"
        - std::string("_Min").size() )
      { // not a valid XXX_Min parameter, keep searching
      continue;
      }
    // Get the XXX from XXX_Min
    std::string paramName = (*it)->getName().substr( 0, pos );
    // Find the matching XXX_Max and XXX_Step parameter
    std::string                             max = paramName + "_Max";
    std::string                             step = paramName + "_Step";
    std::list<TCLAP::Arg *>::const_iterator it2;
    for( it2 = begin; it2 != end; ++it2 )
      {
      if( (*it2)->getName() != max )
        { // not a parameter that match XXX_Max where XXX is from XXX_Min
        continue;
        }
      std::list<TCLAP::Arg *>::const_iterator it3;
      for( it3 = begin; it3 != end; ++it3 )
        {
        if( (*it3)->getName() != step )
          { // not a parameter that match XXX_Step where XXX is from XXX_Min
          continue;
          }
        // We found XXX_Min, XXX_Max and XXX_Step
        res.push_back( *it );
        res.push_back( *it2 );
        res.push_back( *it3 );
        break;
        }
      break;
      }
    }
  return res;
}

void WriteArgInScript( std::stringstream& script,
                       const std::string& indent,
                       const TCLAP::Arg* arg,
                       const std::string& name,
                       const std::string& value,
                       bool quote = true)
{
  if( !arg || !arg->isSet() )
    {
    return;
    }
  // Write a script line that set the correct parameter to the
  // application to execute: i.e. SetAppFlag( app.origin.origin 0,0,0 )
  // Only required parameters have their name set correctly
  // by the TCLAP parser. The names of the non required parameters
  // are in fact flag names.
  if( arg->isRequired() )
    {
    script << indent << "SetAppOption( app.";
    }
  else
    {
    script << indent << "SetAppFlag( app.";
    }
  script << name;
  if( arg->isValueRequired() )
    {
    script << "." << name;
    }
  script << " " << (quote ? "'" : "")
         << value
         << (quote ? "'" : "" ) << " )" << std::endl;
}

int main(int argc, char* argv[])
{
  PARSE_ARGS;

  // Generate the program .bmm file
  ApplicationWrapper applicationWrapper;
  applicationWrapper.AutomaticCommandLineParsingSlicer(
    BatchMake_APPLICATION_PATH );
  applicationWrapper.Save(
    std::string( BatchMake_WRAPPED_APPLICATION_DIR )
    + std::string("/") + applicationWrapper.GetName()  + std::string(".bmm") );
  bm::ScriptParser batchMakeParser;
  batchMakeParser.LoadWrappedApplication( BatchMake_WRAPPED_APPLICATION_DIR );
  batchMakeParser.SetBatchMakeBinaryPath( BatchMake_WRAPPED_APPLICATION_DIR );

  // Write the script
  std::stringstream script;
  script << "echo('Starting BatchMake Script')" << std::endl;
  // Use the module title to extract the application .BMM name.
  script << "setapp( app @'" << applicationWrapper.GetName()
         << "' )" << std::endl;

  // List of the arguments of the command line
  const std::list<TCLAP::Arg *>& parameters = commandLine.getArgList();
  // iterators to go through the argument list
  std::list<TCLAP::Arg *>::const_iterator it;
  std::list<TCLAP::Arg *>::const_iterator begin = parameters.begin();
  std::list<TCLAP::Arg *>::const_iterator end = parameters.end();

  // for( it = begin; it != end; ++it )
//     {
//     std::cout << "********************" << std::endl;
//     std::cout << "name: " << (*it)->getName() << std::endl;
//     std::cout << "flag: " << (*it)->getFlag() << std::endl;
//     std::cout << "required: " << (*it)->isRequired() << std::endl;
//     std::cout << "val requ: " << (*it)->isValueRequired() << std::endl;
//     std::cout << "set: " << (*it)->isSet() << std::endl;
//     }

  TCLAP::Arg* gridRunningMode = GetParameter( parameters, "runningMode_Grid" );
  TCLAP::Arg* gridInputDir = GetParameter( parameters, "inputDirectory_Grid" );
  TCLAP::Arg* gridOutputDir = GetParameter( parameters, "outputDirectory_Grid" );
  TCLAP::Arg* gridExecutableDir =
    GetParameter( parameters, "executableDirectory_Grid" );
  TCLAP::Arg* gridWorkingDir =
    GetParameter( parameters, "workingDirectory_Grid" );
  TCLAP::Arg* gridTransferFile = GetParameter( parameters, "transferFile_Grid" );

  // Get the pairs of "XXX_Directory" "XXX_Mask" parameters
  std::list<TCLAP::Arg *> directoryMaskParameters =
    GetDirectoryMaskParameters( parameters );
  // Get the trios of "XXX_Min" "XXX_Max" and "XXX_Step" parameters
  std::list<TCLAP::Arg *> minMaxStepParameters =
    GetMinMaxStepParameters( parameters );
  // Gather all the parameters that need special handling.
  std::list<TCLAP::Arg *> skipParameters;
  skipParameters.insert( skipParameters.end(), directoryMaskParameters.begin(),
                         directoryMaskParameters.end() );
  skipParameters.insert( skipParameters.end(), minMaxStepParameters.begin(),
                         minMaxStepParameters.end() );
  skipParameters.insert( skipParameters.end(), gridRunningMode );
  skipParameters.insert( skipParameters.end(), gridInputDir );
  skipParameters.insert( skipParameters.end(), gridOutputDir );
  skipParameters.insert( skipParameters.end(), gridExecutableDir );
  skipParameters.insert( skipParameters.end(), gridWorkingDir );
  skipParameters.insert( skipParameters.end(), gridTransferFile );
  // just a spacing string to indent correctly the generated script
  std::string indent = "";
  // Write in the scripts the application options that that are not
  // iterated over.
  // Write something like:
  // SetAppOption( app.threshold.threshold 10 )
  // SetAppOption( app.numberOfThreads.numberOfThreads 2 )
  // ...
  for( it = begin; it != end; ++it )
    {
    // make sure it is not a parameter that is handled later
    std::list<TCLAP::Arg *>::const_iterator it2 =
      std::find( skipParameters.begin(),
                 skipParameters.end(), *it );
    if( it2 != skipParameters.end() )
      {
      continue;
      }
    WriteArgInScript( script, indent, *it,
                      (*it)->getName(), (*it)->getValueAsString() );
    /*
    if( !(*it)->isSet() )
      {
      continue;
      }
    // Write a script line that set the correct parameter to the
    // application to execute: i.e. SetAppFlag( app.origin.origin 0,0,0 )
    // Only required parameters have their name set correctly
    // by the TCLAP parser. The names of the non required parameters
    // are in fact flag names.
    if( (*it)->isRequired() )
      {
      script << indent << "SetAppOption( app." ;
      }
    else
      {
      script << indent << "SetAppFlag( app." ;
      }
    script << (*it)->getName() ;
    if( (*it)->isValueRequired() )
      {
      script << "." << (*it)->getName();
      }
    script << " '" << (*it)->getValueAsString() << "' )" << std::endl;
    */
    }

  // Directory + Mask for INPUTS
  // By setting variable names in your .xml file, you can iterate your
  // BatchMake script over many files in a directory.
  // For instance you can execute the RegisterImages module for 1 fixed
  // image and many moving images by setting in xml:
  // <file><name>fixedImage</name>...</file>
  // <directory><name>movingImage_Directory</name>...</directory>
  // <string><name>movingImage_Mask</name>...</string>
  // movingImage_Directory and movingImage_Mask will be
  // process as if you want to iterate through many moving files.
  // This is possible because movingImage_Directory and movingImage_Mask
  // have the same name before _Directory and _Mask ie movingImage.
  begin = directoryMaskParameters.begin();
  end = directoryMaskParameters.end();
  for( it = begin; it != end; )
    {
    // The true name of the parameter XXX_Directory is XXX
    std::string paramName = (*it)->getName();
    paramName.erase( paramName.rfind("_Directory") );
    // Get the directory and mask parameters
    TCLAP::Arg* directoryArg = *(it++);
    TCLAP::Arg* maskArg = *(it++);
    std::string directory = directoryArg->getValueAsString();
    std::string mask = maskArg->getValueAsString();
    // There are 2 kinds of masks, for input and for output.
    // input masks generally contains a *. A ForEach() loop is used in this
    // case.
    // output masks generally contain XXX${param1}YYY${param2}.ext that
    // describes the output file names. In this example, param1 and param2
    // are varying ( using _Directory/Mask or _Min/_Max_Step ).
    if( mask.find('*') == std::string::npos )
      {
      // Output masks don't necessitate a ForEach() loop
      continue;
      }
    // If directoryArg name is InputVolume_Directory and maskArg name is
    // InputVolume_Mask with values respectively /home/user/inputs and *.mha
    // Set( InputVolume_Directory '/home/user/inputs' )
    // ListFileInDir( InputVolumeFiles ${InputVolume_Directory} *.mha )
    // ForEach( InputVolume ${InputVolumeFiles} )
    //   SetAppOption( app.InputVolume.InputVolume
    //                 ${InputVolume_Directory}/${InputVolume} )
    //   ...
    //   Run( output ${app} )
    // EndForEach( InputVolume )
    script << indent << "Set( " << paramName << "_Directory"
           << " '" << directory << "' )" << std::endl;
    script << indent << "ListFileInDir( " << paramName << "Files "
           << " ${" << paramName << "_Directory}"
           << " " << mask << " )" << std::endl;
    script << indent << "ForEach( " << paramName
           << " ${" << paramName << "Files} )" << std::endl;
    indent += "  ";
    std::string value = std::string("${") + paramName + "_Directory}"
      + "/${" + paramName + "}";
    WriteArgInScript( script, indent, directoryArg, paramName, value, false );
    /*
    // Write a script line that set the correct parameter to the
    // application to execute: i.e. SetAppFlag( app.origin.origin 0,0,0 )
    // Only required parameters have their name set correctly
    // by the TCLAP parser. The names of the non required parameters
    // are in fact flag names.
    if( directoryArg->isRequired() )
      {
      script << indent << "SetAppOption( app.";
      }
    else
      {
      script << indent << "SetAppFlag( app.";
      }
    script << paramName << "." << paramName
           << " ${" << paramName << "_Directory}"
           << "/${" << paramName << "} )" << std::endl;
    */
    }
  // Min + Max + Step
  // The same principe as _Directory/_Mask is applied for parameters that
  // end by _Min, _Max and _Step.
  // If you have in your .xml file 3 parameters that have the same name
  // with a suffix _Min, _Max and _Step, then the BatchMake script will
  // iterate over the parameter varying from min to max with a given step.
  // <integer><name>iterations_Min</name>...</integer>
  // <integer><name>iterations_Max</name>...</integer>
  // <integer><name>iterations_Step</name>...</integer>
  // If the values in the command line are: 0, 10, 2 for respectively
  // iterations_Min, iterations_Max and iterations_Step, this will result
  // in a script like:
  // SetApp( app @myApp )
  // ...
  // Sequence( iterations 0 10 2 )
  // ForEach( iteration ${iterations} )
  //   SetAppOption( app.iteration.iteration ${iteration} )
  //   ...
  //   Run( output ${app} )
  // EndForEach( iteration )
  begin = minMaxStepParameters.begin();
  end = minMaxStepParameters.end();
  for( it = begin; it != end; )
    {
    // Get the true name of the param: the XXX in XXX_Min, XXX_Max and XXX_Step
    std::string paramName = (*it)->getName();
    paramName.erase( paramName.rfind("_Min") );
    std::string min, max, step;
    TCLAP::Arg* minArg = *(it++);
    TCLAP::Arg* maxArg = *(it++);
    TCLAP::Arg* stepArg = *(it++);
    min = minArg->getValueAsString();
    max = maxArg->getValueAsString();
    step = stepArg->getValueAsString();
    script << indent << "Sequence( " << paramName << "s"
           << " " << min << " " << max << " " << step << " )" << std::endl;
    script << indent << "ForEach( " << paramName
           << " ${" << paramName << "s} )" << std::endl;
    indent += "  ";
    std::string value = std::string("${") + paramName + "}";
    WriteArgInScript( script, indent, minArg, paramName, value, false );
    /*
    // Write a script line that set the correct parameter to the
    // application to execute: i.e. SetAppFlag( app.origin.origin 0,0,0 )
    // Only required parameters have their name set correctly
    // by the TCLAP parser. The names of the non required parameters
    // are in fact flag names.
    if( minArg->isRequired() )
      {
      script << indent << "SetAppOption( app." ;
      }
    else
      {
      script << indent << "SetAppFlag( app." ;
      }
    script << paramName << "." << paramName
           << " ${" << paramName << "} )" << std::endl;
    */
    }

  // Directory + Mask for OUTPUTS
  // As we were iterating the script through many files,
  // meaning that we were executing many times the application
  // if the application generates an output, we probably want to
  // create a unique file each time the application is run.
  // The easiest way to do it is by defining a name depending on the
  // variables values.
  // If the script is iterated over many files and varies parameters like:
  // ForEach( InputVolume ${InputVolumes} )
  //   ...
  //   ForEach( variance ${variances} )
  //     ...
  //     SetAppOption( app.outputFile.outputFile **changing name** )
  //     Run( output ${app} )
  //   EndForEach( variance )
  // EndForEach( InputVolume )
  // You probably want to have a changing name looking like:
  //   ${outputFile_Directory}/${InputVolume}-processed-${variance}.mha
  // To have this behavior you have to define in your .xml files, some
  // parameters like:
  // <directory><name>outputFile_Directory</name>...</directory>
  // <string><name>outputFile_Mask</name>...</directory>
  // Where outputFile_Directory could be set to /home/user/outputs
  // and outputFile_Mask could be set to ${InputVolume}-processed-${variance}.mha
  begin = directoryMaskParameters.begin();
  end = directoryMaskParameters.end();
  for( it = begin; it != end; )
    {
    std::string paramName = (*it)->getName();
    paramName.erase( paramName.rfind("_Directory") );
    TCLAP::Arg* directoryArg = *(it++);
    TCLAP::Arg* maskArg = *(it++);
    std::string directory = directoryArg->getValueAsString();
    std::string mask = maskArg->getValueAsString();
    // There are 2 kinds of masks, for input and for output.
    // input masks generally contains a * and a forEach loop is used.
    // output masks generally contain XXX${...}YYY that describe the
    // output file name.
    if( mask.find('*') != std::string::npos )
      {
      continue;
      }
    // Example of generated script:
    // ForEach( InputVolume ${InputVolumes} )
    //   ...
    //   ForEach( variance ${variances} )
    //     ...
    //     Set( outputFile_Directory '/home/user/outputs' )
    //     SetAppOption( app.outputFile.outputFile
    //       ${outputFile_Directory}/${InputVolume}-processed-${variance}.mha )
    //     Run( output ${app} )
    //   EndForEach( variance )
    // EndForEach( InputVolume )
    script << indent << "Set( " << paramName << "_Directory"
           << " '" << directory << "' )" << std::endl;
    std::string value = std::string("${") + paramName + "_Directory}"
      + "/" + mask;
    WriteArgInScript( script, indent, directoryArg, paramName, value, false );
    /*
    // Write a script line that set the correct parameter to the
    // application to execute: i.e. SetAppFlag( app.origin.origin 0,0,0 )
    // Only required parameters have their name set correctly
    // by the TCLAP parser. The names of the non required parameters
    // are in fact flag names.
    if( directoryArg->isRequired() )
      {
      script << indent << "SetAppOption(app." ;
      }
    else
      {
      script << indent << "SetAppFlag(app." ;
      }
    script << paramName << "." << paramName
           << " ${" << paramName << "_Directory}/" << mask << " )" << std::endl;
    */
    }
  // Finally run the script
  script << indent << "Run( runOutput ${app} )" << std::endl;
  // Now generate the EndForEach() script lines.
  // For each ForEach() loop must correspond an EndForEach command
  // Min, Max and Step
  begin = minMaxStepParameters.begin();
  end = minMaxStepParameters.end();
  for( it = begin; it != end; ++it )
    {
    indent.erase( 0, 2 );
    std::string paramName = (*it)->getName();
    paramName.erase( paramName.rfind("_Min") );
    script << indent << "EndForEach( " << paramName << " )" << std::endl;
    // go to max
    ++it;
    // go to step
    ++it;
    }
  // Directory + Mask
  begin = directoryMaskParameters.begin();
  end = directoryMaskParameters.end();
  for( it = begin; it != end; ++it )
    {
    std::string paramName = (*it)->getName();
    paramName.erase( paramName.rfind("_Directory") );
    std::string directory =
      dynamic_cast<TCLAP::ValueArg<std::string> *>( *it )->getValue();
    ++it;
    std::string mask =
      dynamic_cast<TCLAP::ValueArg<std::string> *>( *it )->getValue();
    // There are 2 kinds of masks, for input and for output.
    // input masks generally contains a * and a forEach loop is used.
    // output masks generally contain XXX${...}YYY that describe the
    // output file name.
    if( mask.find('*') != std::string::npos )
      {
      continue;
      }
    indent.erase( 0, 2 );
    script << indent << "EndForEach( " << paramName << " )" << std::endl;
    }

  std::cout << script.str();

  if( !gridRunningMode || gridRunningMode->getValueAsString() != "condor" )
    {
    batchMakeParser.ParseBuffer(script.str() );
    }
  else
    {
    if( gridInputDir )
      {
      script << "InputDirectory('" << gridInputDir->getValueAsString()
             << "')" << std::endl;
      }
    if( gridOutputDir )
      {
      script << "OutputDirectory( '"  << gridOutputDir->getValueAsString()
             << "')" << std::endl;
      }
    if( gridExecutableDir )
      {
      script << "ExecutableDirectory( '"
             << gridExecutableDir->getValueAsString() << "' )" << std::endl;
      }
    if( gridWorkingDir )
      {
      script << "WorkingDirectory( '"
             << gridWorkingDir->getValueAsString() << "' )" << std::endl;
      }
    if( gridTransferFile )
      {
      script << "GridTransferFile( " << gridTransferFile->getValueAsString()
             << " )" << std::endl;
      }
    script << "GridOwner( condor )" << std::endl;

    // generate the condor script and condor_submit them
    batchMakeParser.RunCondor( script.str() );
    }

  return EXIT_SUCCESS;
}
