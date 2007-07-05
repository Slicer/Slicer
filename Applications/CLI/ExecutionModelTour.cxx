#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "ExecutionModelTourCLP.h"

#include "vtkMRMLScene.h"
#include "vtkSmartPointer.h"

int main (int argc, char *argv[])
{
  PARSE_ARGS;

  // tease apart the scene files and the nodes
  std::string::size_type loc;
  std::string transform1Filename, transform2Filename;
  std::string transform1ID, transform2ID;
  
  loc = transform1.find_last_of("#");
  if (loc != std::string::npos)
    {
    transform1Filename = std::string(transform1.begin(),
                                     transform1.begin() + loc);
    loc++;

    transform1ID = std::string(transform1.begin()+loc, transform1.end());
    }

  loc = transform2.find_last_of("#");
  if (loc != std::string::npos)
    {
    transform2Filename = std::string(transform2.begin(),
                                     transform2.begin() + loc);
    loc++;

    transform2ID = std::string(transform2.begin()+loc, transform2.end());
    }

  std::cout << "Transform1 filename: " << transform1Filename << std::endl;
  std::cout << "Transform1 ID: " << transform1ID << std::endl;
  std::cout << "Transform2 filename: " << transform2Filename << std::endl;
  std::cout << "Transform2 ID: " << transform2ID << std::endl;


  if (transform1Filename != transform2Filename)
    {
    std::cerr << "Module only accepts transforms from the same scene. Two scenes were specified: " << transform1Filename << " and " << transform2Filename << std::endl;
    return EXIT_FAILURE;
    }
  
  vtkSmartPointer<vtkMRMLScene> scene = vtkMRMLScene::New();
  scene->SetURL( transform1Filename.c_str() );
  scene->Import();

  vtkMRMLNode *node = scene->GetNodeByID( transform1ID );
  if (node)
    {
    vtkMRMLNode *outNode = scene->GetNodeByID( transform2ID );

    if (outNode)
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

  return EXIT_SUCCESS;
}
