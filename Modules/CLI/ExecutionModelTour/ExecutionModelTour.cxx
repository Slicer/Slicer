#include "ExecutionModelTourCLP.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkDelimitedTextReader.h>
#include <vtkDelimitedTextWriter.h>
#include <vtkTable.h>

// Markups includes
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLMarkupsFiducialStorageNode.h>

int main(int argc, char *argv[])
{
  PARSE_ARGS;

  // files
  std::cout << "Input file = " << file1 << std::endl;
  std::cout << "Files size = " << files.size() << std::endl;
  std::cout << "Output file = " << outputFile1 << std::endl;

  // booleans
  std::cout << "Boolean 1 = " << boolean1 << std::endl;
  std::cout << "Boolean 2 = " << boolean2 << std::endl;
  std::cout << "Boolean 3 = " << boolean3 << std::endl;

  // tease apart the scene files and the nodes
  std::string::size_type loc;
  std::string            transform1Filename, transform2Filename;
  std::string            transform1ID, transform2ID;

  if( transform1.empty() )
    {
    std::cerr << "transform1 parameter is required" << std::endl;
    return EXIT_FAILURE;
    }

  if( transform2.empty() )
    {
    std::cerr << "transform2 parameter is required" << std::endl;
    return EXIT_FAILURE;
    }

  loc = transform1.find_last_of("#");
  if( loc != std::string::npos )
    {
    transform1Filename = std::string(transform1.begin(),
                                     transform1.begin() + loc);
    loc++;

    transform1ID = std::string(transform1.begin() + loc, transform1.end() );
    }

  loc = transform2.find_last_of("#");
  if( loc != std::string::npos )
    {
    transform2Filename = std::string(transform2.begin(),
                                     transform2.begin() + loc);
    loc++;

    transform2ID = std::string(transform2.begin() + loc, transform2.end() );
    }

  std::cout << "Transform1 filename: " << transform1Filename << std::endl;
  std::cout << "Transform1 ID: " << transform1ID << std::endl;
  std::cout << "Transform2 filename: " << transform2Filename << std::endl;
  std::cout << "Transform2 ID: " << transform2ID << std::endl;

  if( transform1Filename != transform2Filename )
    {
    std::cerr << "Module only accepts transforms from the same scene. Two scenes were specified: "
              << transform1Filename << " and " << transform2Filename << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLScene> scene;
  scene->SetURL( transform1Filename.c_str() );
  scene->Import();

  vtkMRMLNode *node = scene->GetNodeByID( transform1ID );
  if( node )
    {
    vtkMRMLNode *outNode = scene->GetNodeByID( transform2ID );

    if( outNode )
      {
      outNode->Copy( node );
      scene->Commit( transform2Filename.c_str() );
      }
    else
      {
      std::cerr << "No output transform found! Specified transform ID = " << transform2ID << std::endl;
      return EXIT_FAILURE;
      }
    }
  else
    {
    std::cerr << "No input transform found! Specified transform ID = " << transform1ID << std::endl;
    return EXIT_FAILURE;
    }
  // fiducials
  std::cout << "Have an input seed list of size " << seed.size() << std::endl;
  for (unsigned int i = 0; i < seed.size(); ++i)
    {
    std::cout << i << "\t" << seed[i][0] << "\t" << seed[i][1] << "\t" << seed[i][2] << std::endl;
    }
  if (!seedsFile.empty())
    {
    // read the input seeds file
    std::cout << "Have an input seeds file with name " << seedsFile.c_str() << std::endl;
    vtkNew<vtkMRMLMarkupsFiducialNode> fiducialNode;
    vtkNew<vtkMRMLMarkupsFiducialStorageNode> fiducialStorageNode;
    fiducialStorageNode->SetFileName(seedsFile.c_str());
    fiducialStorageNode->ReadData(fiducialNode.GetPointer());
    std::cout << "Number of fids read = " << fiducialNode->GetNumberOfFiducials() << ", coordinate system flag = " << fiducialStorageNode->GetCoordinateSystem() << std::endl;
    for (int i = 0; i < fiducialNode->GetNumberOfFiducials(); ++i)
      {
      double pos[3];
      fiducialNode->GetNthFiducialPosition(i, pos);
      std::cout << i << "\t" << pos[0] << "\t" << pos[1] << "\t" << pos[2] << std::endl;
      }
    }
  // copy the seeds list
  vtkNew<vtkMRMLMarkupsFiducialNode> copiedFiducialNode;
  // set the node name so that fiducials have names that don't just
  // start with -1, -2 etc
  copiedFiducialNode->SetName("seedsCopy");
  for (unsigned int i = 0; i < seed.size(); ++i)
    {
    std::cout << "Copying seed list to output file list: " << seed[i][0] << ", " << seed[i][1] << ", " << seed[i][2] << std::endl;
    copiedFiducialNode->AddFiducial(seed[i][0], seed[i][1], seed[i][2]);
    // toggle some settings
    if (i == 0)
      {
      copiedFiducialNode->SetNthFiducialLocked(i, true);
      copiedFiducialNode->SetNthFiducialSelected(i, false);
      copiedFiducialNode->SetNthFiducialVisibility(i, false);
      }
    }
  // write out the copy
  vtkNew<vtkMRMLMarkupsFiducialStorageNode> outputFiducialStorageNode;
  outputFiducialStorageNode->SetFileName(seedsOutFile.c_str());
  // the .xml file specifies that it expects the output file in LPS
  // coordinate system
  outputFiducialStorageNode->UseLPSOn();
  outputFiducialStorageNode->WriteData(copiedFiducialNode.GetPointer());

  // generic tables

  vtkSmartPointer<vtkTable> table = vtkSmartPointer<vtkTable>::New();

  if (!inputDT.empty())
    {
    vtkNew<vtkDelimitedTextReader> tsvReader;
    tsvReader->SetFileName(inputDT.c_str());
    tsvReader->SetFieldDelimiterCharacters("\t");
    tsvReader->SetHaveHeaders(true);
    tsvReader->SetDetectNumericColumns(true);
    tsvReader->Update();
    table = tsvReader->GetOutput();
    std::cout << "number of rows:" << table->GetNumberOfRows() << std::endl;
    std::cout << "number of cols:" << table->GetNumberOfColumns() << std::endl;
    if (table->GetNumberOfRows()<1)
      {
      table->InsertNextBlankRow();
      }
    table->SetValue(0,0,vtkVariant("Computed first"));
    if (table->GetNumberOfRows()<2)
      {
      table->InsertNextBlankRow();
      }
    table->SetValue(1,0,vtkVariant("Computed second"));
    }

  if (!outputDT.empty())
    {
    vtkNew<vtkDelimitedTextWriter> tsvWriter;
    tsvWriter->SetFileName(outputDT.c_str());
    tsvWriter->SetFieldDelimiter("\t");
    tsvWriter->SetInputData(table);
    tsvWriter->Update();
    }

  // Write out the return parameters in "name = value" form
  std::ofstream rts;
  rts.open(returnParameterFile.c_str() );
  rts << "anintegerreturn =  10" << std::endl;
  rts << "abooleanreturn = true" << std::endl;
  rts << "afloatreturn = 34.2" << std::endl;
  rts << "adoublereturn = 102.7" << std::endl;
  rts << "astringreturn = Good-bye" << std::endl;
  rts << "anintegervectorreturn = 4,5,6,7" << std::endl;
  rts << "astringchoicereturn = Ron" << std::endl;
  rts.close();

  return EXIT_SUCCESS;
}
