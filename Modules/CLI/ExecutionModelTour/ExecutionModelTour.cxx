#include "ExecutionModelTourCLP.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

// STD includes

int main(int argc, char *argv[])
{
  PARSE_ARGS;

  // files
  std::cout << "Files size = " << files.size() << std::endl;

  // booleans
  std::cout << "Boolean 1 = " << boolean1 << std::endl;
  std::cout << "Boolean 2 = " << boolean2 << std::endl;
  std::cout << "Boolean 3 = " << boolean3 << std::endl;

  // tease apart the scene files and the nodes
  std::string::size_type loc;
  std::string            transform1Filename, transform2Filename;
  std::string            transform1ID, transform2ID;

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

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
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
  scene = 0;

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
