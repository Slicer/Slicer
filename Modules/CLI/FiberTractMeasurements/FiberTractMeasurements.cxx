// vtkTeem includes
#include <vtkDiffusionTensorMathematics.h>
#include <vtkNRRDReader.h>
#include <vtkNRRDWriter.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyDataTensorToColor.h>
#include <vtkPolyDataReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkGlobFileNames.h>
#include <vtkStringArray.h>
#include <vtkAssignAttribute.h>

// MRML
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelStorageNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLFiberBundleNode.h>
#include <vtkMRMLFiberBundleLineDisplayNode.h>
#include <vtkMRMLFiberBundleTubeDisplayNode.h>
#include <vtkMRMLFiberBundleGlyphDisplayNode.h>
#include <vtkMRMLFiberBundleStorageNode.h>
#include <vtkMRMLSceneViewNode.h>
#include <vtkMRMLSceneViewStorageNode.h>
#include <vtkMRMLCommandLineModuleNode.h>

// VTKsys includes
#include <vtksys/SystemTools.hxx>

// ITK includes
#include <itkFloatingPointExceptions.h>

#include "FiberTractMeasurementsCLP.h"

void computeFiberStats(vtkPolyData *poly,
                       std::string &id);

void computeScalarMeasurements(vtkPolyData *poly,
                               std::string &id,
                               std::string &operation);

int computeTensorMeasurement(vtkPolyDataTensorToColor *math,
                             vtkAlgorithmOutput *input,
                             std::string &id,
                             std::string &operation);

void getPathFromParentToChild(vtkMRMLHierarchyNode *parent,
                              vtkMRMLHierarchyNode *child,
                              std::string &path);

bool setTensors(vtkPolyData *poly);

void printTable(std::ofstream &ofs, bool printHeader,
                std::map< std::string, std::map<std::string, double> > &output);

std::string getNthTensorName(int n, vtkPolyData *poly);

bool isInCluster(const std::string &id, const std::string &clusterName);

int getNumberOfTensors(vtkPolyData *poly);

int addClusters();

void printFlat(std::ofstream &ofs);

void printCluster(const std::string &id,
                  std::map< std::string, std::map<std::string, double> > &output,
                  std::map<std::string, std::string> &names,
                  std::stringstream &ids,
                  std::stringstream &measureNames,
                  std::stringstream &measureValues);

std::map< std::string, std::map<std::string, double> > OutTable;
std::map< std::string, std::string> ClusterNames;
std::map< std::string, std::map<std::string, double> > Clusters;

#define INVALID_NUMBER -999999
#define INVALID_NUMBER_PRINT std::string("#NAN")

int main( int argc, char * argv[] )
{
  itk::FloatingPointExceptions::Disable();

  PARSE_ARGS;

  std::ofstream ofs(outputFile.c_str());
  if (ofs.fail())
    {
    std::cerr << "Output file doesn't exist: " <<  outputFile << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkPolyDataTensorToColor> math;
  std::vector<std::string> operations;
  operations.push_back(std::string("Trace"));
  operations.push_back(std::string("RelativeAnisotropy"));
  operations.push_back(std::string("FractionalAnisotropy"));
  operations.push_back(std::string("LinearMeasurement"));
  operations.push_back(std::string("PlanarMeasurement"));
  operations.push_back(std::string("SphericalMeasurement"));
  operations.push_back(std::string("MinEigenvalue"));
  operations.push_back(std::string("MidEigenvalue"));
  operations.push_back(std::string("MaxEigenvalue"));
  std::vector<std::string> emptyOperationVector;
  emptyOperationVector.push_back(std::string(""));

  if (inputType == std::string("Fibers_Hierarchy") ||
      inputType == std::string("All_Fiber_Nodes") )
    {
    // get the model hierarchy id from the scene file
    std::string::size_type loc;
    std::string            inputFilename;
    std::string            inputNodeID;

    if (inputType == std::string("Fibers_Hierarchy"))
      {
      std::string sceneFilename;
      std::string filename = FiberHierarchyNode[0];
      loc = filename.find_last_of("#");
      if (loc != std::string::npos)
        {
        sceneFilename = std::string(filename.begin(),
                                    filename.begin() + loc);
        loc++;

        inputNodeID = std::string(filename.begin() + loc, filename.end());
        }
      }

    // check for the model mrml file
    if (FibersMRMLFile.empty())
      {
      std::cout << "No MRML scene file specified." << std::endl;
      return EXIT_FAILURE;
      }

    // get the directory of the scene file
    std::string rootDir
      = vtksys::SystemTools::GetParentDirectory(FibersMRMLFile.c_str());

    vtkNew<vtkMRMLScene> modelScene;

    // load the scene that Slicer will re-read
    modelScene->SetURL(FibersMRMLFile.c_str());

    modelScene->RegisterNodeClass(vtkNew<vtkMRMLSceneViewNode>().GetPointer());
    modelScene->RegisterNodeClass(vtkNew<vtkMRMLSceneViewStorageNode>().GetPointer());
    modelScene->RegisterNodeClass(vtkNew<vtkMRMLCommandLineModuleNode>().GetPointer());
    modelScene->RegisterNodeClass(vtkNew<vtkMRMLFiberBundleNode>().GetPointer());
    modelScene->RegisterNodeClass(vtkNew<vtkMRMLFiberBundleLineDisplayNode>().GetPointer());
    modelScene->RegisterNodeClass(vtkNew<vtkMRMLFiberBundleTubeDisplayNode>().GetPointer());
    modelScene->RegisterNodeClass(vtkNew<vtkMRMLFiberBundleGlyphDisplayNode>().GetPointer());
    modelScene->RegisterNodeClass(vtkNew<vtkMRMLFiberBundleStorageNode>().GetPointer());

    // only try importing if the scene file exists
    if (vtksys::SystemTools::FileExists(FibersMRMLFile.c_str()))
      {
      modelScene->Import();
      }
    else
      {
      std::cerr << "Model scene file doesn't exist: " <<  FibersMRMLFile.c_str() << std::endl;
      }

    if (inputType == std::string("Fibers_Hierarchy"))
      {
      // make sure we have a model hierarchy node
      vtkMRMLNode *node = modelScene->GetNodeByID(inputNodeID);
      vtkSmartPointer<vtkMRMLModelHierarchyNode> topHierNode =
         vtkMRMLModelHierarchyNode::SafeDownCast(node);
      if (!topHierNode)
        {
        std::cerr << "Model hierachy node doesn't exist: " <<  inputNodeID.c_str() << std::endl;
        return EXIT_FAILURE;
        }

      // get all the children nodes
      std::vector< vtkMRMLHierarchyNode *> allChildren;
      topHierNode->GetAllChildrenNodes(allChildren);

      // and loop over them
      for (unsigned int i = 0; i < allChildren.size(); ++i)
        {
        vtkMRMLDisplayableHierarchyNode *dispHierarchyNode = vtkMRMLDisplayableHierarchyNode::SafeDownCast(allChildren[i]);
        if (dispHierarchyNode)
          {
          // get any associated node
          vtkMRMLFiberBundleNode *fiberNode = vtkMRMLFiberBundleNode::SafeDownCast(
              dispHierarchyNode->GetAssociatedNode());

          if (fiberNode)
            {
            std::string id = std::string(fiberNode->GetName());

            // concat path to parent
            getPathFromParentToChild(topHierNode, dispHierarchyNode, id);

            computeFiberStats(fiberNode->GetPolyData(), id);

            computeScalarMeasurements(fiberNode->GetPolyData(), id, emptyOperationVector[0]);

            for (int o=0; o<operations.size(); o++)
              {
              computeTensorMeasurement(math.GetPointer(),
                                       fiberNode->GetPolyDataConnection(),
                                       id,
                                       operations[o]);
              } // for (int o=0; o<operations.size(); o++)
            } // if (fiberNode)
          } // if (dispHierarchyNode)
        } // for (unsigned int i = 0; i < allChildren.size(); ++i)
      } // if (inputType == std::string("Fibers_Hierarchy"))
    else if (inputType == std::string("All_Fiber_Nodes"))
      {
      std::vector< vtkMRMLNode *> fiberNodes;
      modelScene->GetNodesByClass("vtkMRMLFiberBundleNode", fiberNodes);
      for (int i=0; i<fiberNodes.size(); i++)
        {
        vtkMRMLFiberBundleNode *fiberNode = vtkMRMLFiberBundleNode::SafeDownCast(fiberNodes[i]);
        std::string id = std::string(fiberNode->GetName());

        computeFiberStats(fiberNode->GetPolyData(), id);

        computeScalarMeasurements(fiberNode->GetPolyData(), id, emptyOperationVector[0]);

        for (int o=0; o<operations.size(); o++)
          {
          computeTensorMeasurement(math.GetPointer(),
                                   fiberNode->GetPolyDataConnection(),
                                   id,
                                   operations[o]);
          } // for (int o=0; o<operations.size(); o++)
        } // for (int i=0; i<fiberNodes.size(); i++)
      } // else if inputType == std::string("All_Fiber_Nodes"))
    } //if (inputType == ... || ... )
  else if (inputType == std::string("Fibers_File_Folder") )
    {
    // File based
    if (InputDirectory.size() == 0)
      {
      std::cerr << "Input directory doesn't exist: " << std::endl;
      return EXIT_FAILURE;
      }

    vtkNew<vtkGlobFileNames> gfnVTK;
    gfnVTK->SetDirectory(InputDirectory.c_str());
    gfnVTK->AddFileNames("*.vtk");
    vtkStringArray *fileNamesVTK = gfnVTK->GetFileNames();

    vtkNew<vtkGlobFileNames> gfnVTP;
    gfnVTP->SetDirectory(InputDirectory.c_str());
    gfnVTP->AddFileNames("*.vtp");
    vtkStringArray *fileNamesVTP = gfnVTP->GetFileNames();

    // Loop over polydatas
    for (int i=0; i<fileNamesVTP->GetNumberOfValues(); i++)
      {
      vtkNew<vtkXMLPolyDataReader> readerVTP;
      vtkStdString fileName = fileNamesVTP->GetValue(i);
      readerVTP->SetFileName(fileName.c_str());
      readerVTP->Update();
      std::string id = fileName;

      computeFiberStats(readerVTP->GetOutput(), id);

      computeScalarMeasurements(readerVTP->GetOutput(), id, emptyOperationVector[0]);

      if( !setTensors(readerVTP->GetOutput()) )
        {
        std::cerr << argv[0] << ": No tensor data for file " << fileName << std::endl;
        continue;
        }

      for (int o=0; o<operations.size(); o++)
        {
        computeTensorMeasurement(math.GetPointer(),
                                 readerVTP.GetPointer()->GetOutputPort(),
                                 id,
                                 operations[o]);
        }
      }
    for (int i=0; i<fileNamesVTK->GetNumberOfValues(); i++)
      {
      vtkNew<vtkPolyDataReader> readerVTK;
      vtkStdString fileName = fileNamesVTK->GetValue(i);
      readerVTK->SetFileName(fileName.c_str());
      readerVTK->Update();
      std::string id = fileName;

      computeFiberStats(readerVTK->GetOutput(), id);

      computeScalarMeasurements(readerVTK->GetOutput(), id, emptyOperationVector[0]);

      if( !setTensors(readerVTK->GetOutput()) )
        {
        std::cerr << argv[0] << ": No tensor data for file " << fileName << std::endl;
        continue;
        }

      for (int o=0; o<operations.size(); o++)
        {
        computeTensorMeasurement(math.GetPointer(),
                                 readerVTK.GetPointer()->GetOutputPort(),
                                 id,
                                 operations[o]);
        }
      }
    } //if (inputType == std::string("Fibers File Folder") )

  if (addClusters() == 0)
    {
    return EXIT_FAILURE;
    }

  if (outputFormat == std::string("Row_Hierarchy"))
    {
    printFlat(ofs);
    }
  else
    {
    printTable(ofs, true, OutTable);
    printTable(ofs, false, Clusters);
    }

  ofs.flush();
  ofs.close();

  return EXIT_SUCCESS;
}

void getPathFromParentToChild(vtkMRMLHierarchyNode *parent,
                              vtkMRMLHierarchyNode *child,
                              std::string &path)
{
  vtkMRMLHierarchyNode *immediateParent = child->GetParentNode();
  if (immediateParent)
    {
    std::string parentName = immediateParent->GetName() ? immediateParent->GetName() : immediateParent->GetID();
    std::string childName = child->GetName() ? child->GetName() : child->GetID();
    std::map<std::string, std::string>::iterator it = ClusterNames.find(childName);
    if (it != ClusterNames.end())
      {
      ClusterNames[childName] = parentName + std::string(":") + it->second;
      }
    path = parentName + std::string(":") + path;
    ClusterNames[parentName] = parentName;
    if (strcmp(immediateParent->GetID(), parent->GetID()) != 0)
      {
      getPathFromParentToChild(parent, immediateParent, path);
      }
    }
}

void computeFiberStats(vtkPolyData *poly,
                       std::string &id)
{
  int npoints = poly->GetNumberOfPoints();
  int npolys = poly->GetNumberOfCells();

  //if (npoints > 0 && npolys > 0)
  //  {
    std::map< std::string, std::map<std::string, double> >::iterator it = OutTable.find(id);
    if (it == OutTable.end())
      {
      OutTable[id] = std::map<std::string, double>();
      it = OutTable.find(id);
      }
    it->second[std::string("Num_Points")] = npoints;
    it->second[std::string("Num_Fibers")] = npolys;
   // }
}

void computeScalarMeasurements(vtkPolyData *poly,
                               std::string &id,
                               std::string &operation)
{
  int npoints = poly->GetNumberOfPoints();
  int npolys = poly->GetNumberOfCells();

  if (npoints == 0 || npolys == 0)
    {
    //return;
    }

  // averagre measurement for each scalar array
  for (int i=0; i<poly->GetPointData()->GetNumberOfArrays(); i++)
    {
    vtkDataArray *arr = poly->GetPointData()->GetArray(i);
    if (arr->GetNumberOfComponents() > 1)
      {
      continue;
      }

    std::string name = operation;
    if (arr->GetName())
      {
      name = std::string(arr->GetName());
      }

    double val;
    double sum = 0;
    for (int n=0; n<npoints; n++)
      {
      arr->GetTuple(n, &val);
      sum += val;
      }
    if (npoints)
      {
      sum /= npoints;
      }
    else
      {
      sum = INVALID_NUMBER;
      }

    std::cout << " : " << name << " = " << sum << std::endl;
    //ofs << id << " : " << name << " = " << sum << std::endl;

    std::map< std::string, std::map<std::string, double> >::iterator it = OutTable.find(id);
    if (it == OutTable.end())
      {
      OutTable[id] = std::map<std::string, double>();
      it = OutTable.find(id);
      }
      it->second[name] = sum;
    } //for (int i=0; i<poly->GetPointData()->GetNumberOfArrays(); i++)
}

int computeTensorMeasurement(vtkPolyDataTensorToColor *math,
                             vtkAlgorithmOutput *input,
                             std::string &id,
                             std::string &operation)
{
  //TODO loop over all tensors, use ExtractTensor
  vtkNew<vtkAssignAttribute> assignAttribute;

#if (VTK_MAJOR_VERSION <= 5)
  assignAttribute->SetInput(0, input );
  math->SetInput(0, assignAttribute->GetOutput() );
#else
  assignAttribute->SetInputConnection(0, input );
  math->SetInputConnection(0, assignAttribute->GetOutputPort() );
#endif

  vtkPolyData *poly = vtkPolyData::SafeDownCast(assignAttribute->GetInput());

  if (poly == 0)
    {
    std::cerr << "no polydata found" << std::endl;
    return EXIT_FAILURE;
    }

  int numTensors = getNumberOfTensors(poly);

  for (int i=0; i<numTensors; i++)
    {
    std::string name = getNthTensorName(i, poly);

    assignAttribute->Assign(
      name.c_str(),
      name.c_str() ? vtkDataSetAttributes::TENSORS : -1,
      vtkAssignAttribute::POINT_DATA);

    assignAttribute->Update();

    if( operation == std::string("Trace") )
      {
      math->ColorGlyphsByTrace();
      }
    else if( operation == std::string("RelativeAnisotropy") )
      {
      math->ColorGlyphsByRelativeAnisotropy();
      }
    else if( operation == std::string("FractionalAnisotropy") )
      {
      math->ColorGlyphsByFractionalAnisotropy();
      }
    else if( operation == std::string("LinearMeasurement") || operation == std::string("LinearMeasure") )
      {
      math->ColorGlyphsByLinearMeasure();
      }
    else if( operation == std::string("PlanarMeasurement") || operation == std::string("PlanarMeasure") )
      {
      math->ColorGlyphsByPlanarMeasure();
      }
    else if( operation == std::string("SphericalMeasurement") || operation == std::string("SphericalMeasure") )
      {
      math->ColorGlyphsBySphericalMeasure();
      }
    else if( operation == std::string("MinEigenvalue") )
      {
      math->ColorGlyphsByMinEigenvalue();
      }
    else if( operation == std::string("MidEigenvalue") )
      {
      math->ColorGlyphsByMidEigenvalue();
      }
    else if( operation == std::string("MaxEigenvalue") )
      {
      math->ColorGlyphsByMaxEigenvalue();
      }
    else
      {
      std::cerr << operation << ": Operation " << operation << "not supported" << std::endl;
      return EXIT_FAILURE;
      }

    math->Update();

    if (!math->GetOutput()->GetPointData() || !math->GetOutput()->GetPointData()->GetScalars())
      {
      std::cerr << "no scalars computed" << std::endl;
      }
    std::string scalarName = name + std::string(".") + operation;
    computeScalarMeasurements(math->GetOutput(), id, scalarName);
    }

  return EXIT_SUCCESS;
}

bool setTensors(vtkPolyData *poly)
{
  bool hasTensors = false;
  if (poly)
    {
    if (poly->GetPointData()->GetTensors())
      {
      hasTensors = true;
      }
    else
      {
      for (int i=0; i<poly->GetPointData()->GetNumberOfArrays(); i++)
        {
        vtkDataArray *arr = poly->GetPointData()->GetArray(i);
        if (arr->GetNumberOfComponents() == 9)
          {
          poly->GetPointData()->SetTensors(arr);
          hasTensors = true;
          }
        }
      }
    }
  return hasTensors;
}

int getNumberOfTensors(vtkPolyData *poly)
{
  int count = 0;
  for (int i=0; i<poly->GetPointData()->GetNumberOfArrays(); i++)
    {
    vtkDataArray *arr = poly->GetPointData()->GetArray(i);
    if (arr->GetNumberOfComponents() == 9)
      {
        count++;
      }
    }
  return count;
}

std::string getNthTensorName(int n, vtkPolyData *poly)
{
  int count = 0;
  for (int i=0; i<poly->GetPointData()->GetNumberOfArrays(); i++)
    {
    vtkDataArray *arr = poly->GetPointData()->GetArray(i);
    if (arr->GetNumberOfComponents() == 9)
      {
      if (count == n)
        {
          return arr->GetName() ? std::string(arr->GetName()) : std::string();
        }
      count++;
      }
    }
  return std::string();
}

std::map<std::string, std::string> getMeasureNames()
{
  std::map<std::string, std::string> names;
  std::map< std::string, std::map<std::string, double> >::iterator it;
  std::map<std::string, double>::iterator it1;

  for(it = OutTable.begin(); it != OutTable.end(); it++)
    {
    for (it1 = it->second.begin(); it1 != it->second.end(); it1++)
      {
      names[it1->first] = it1->first;
      }
    }
  return names;
}

void printTable(std::ofstream &ofs, bool printHeader,
                std::map< std::string, std::map<std::string, double> > &output)
{
  std::map<std::string, std::string> names = getMeasureNames();

  std::map< std::string, std::map<std::string, double> >::iterator it;
  std::map<std::string, double>::iterator it1;
  std::map<std::string, std::string>::iterator it2;

  if (printHeader)
    {
    std::cout << "Name";
    ofs << "Name";

    for (it2 = names.begin(); it2 != names.end(); it2++)
      {
      std::cout << " , " << it2->second;
      ofs << " , " << it2->second;
      }
    std::cout << std::endl;
    ofs << std::endl;
    }

  for(it = output.begin(); it != output.end(); it++)
    {
    std::cout << it->first;
    ofs << it->first;

    for (it2 = names.begin(); it2 != names.end(); it2++)
      {
      std::cout << " , ";
      ofs << " , ";
      it1 = it->second.find(it2->second);
      if (it1 != it->second.end())
        {
        if (it1->second == INVALID_NUMBER)
          {
          std::cout << INVALID_NUMBER_PRINT;
          ofs << INVALID_NUMBER_PRINT;
          }
        else
          {
          std::cout << std::fixed << it1->second;
          ofs << std::fixed << it1->second;
          }
        }
      }
    std::cout << std::endl;
    ofs << std::endl;
    }
}

bool isInCluster(const std::string &id, const std::string &clusterName)
{
  std::string s = id;
  std::string delimiter = ":";
  size_t pos = 0;
  std::string token;
  while ((pos = s.find(delimiter)) != std::string::npos)
    {
    token = s.substr(0, pos);
    if (clusterName == token)
      {
      return true;
      }
    s.erase(0, pos + delimiter.length());
    }
  return false;
}

int addClusters()
{
  std::map< std::string, std::map<std::string, double> >::iterator itOutput;
  std::map<std::string, double>::iterator itValues;
  std::map<std::string, double>::iterator itClusterValues;
  std::map<std::string, std::string>::iterator itClusterNames;
  std::map<std::string, std::string>::iterator itNames;

  std::map<std::string, std::string> names = getMeasureNames();

  for (itClusterNames = ClusterNames.begin(); itClusterNames!= ClusterNames.end(); itClusterNames++)
    {
    Clusters[itClusterNames->second] = std::map<std::string, double>();
    std::map< std::string, std::map<std::string, double> >::iterator itCluster = Clusters.find(itClusterNames->second);

    int npoints = 0;
    int npointsCluster = 0;
    for(itOutput = OutTable.begin(); itOutput != OutTable.end(); itOutput++)
      {
      if (isInCluster(itOutput->first, itClusterNames->first))
        {
        itValues = itOutput->second.find(std::string("Num_Points"));
        if (itValues != itOutput->second.end())
          {
          npoints = itValues->second;
          }
        npointsCluster += npoints;

        for (itNames = names.begin(); itNames != names.end(); itNames++)
          {
          itValues = itOutput->second.find(itNames->second);
          if (itValues == itOutput->second.end())
            {
            std::cerr << "Fibers contain different number of scalars" << std::endl;
            return 0;
            }

          itClusterValues = itCluster->second.find(itNames->second);
          if (itClusterValues == itCluster->second.end())
            {
            itCluster->second[itNames->second] = 0;
            itClusterValues = itCluster->second.find(itNames->second);
            }
          double clusterValue = itClusterValues->second;
          if (itValues != itOutput->second.end() && itNames->second != std::string("Num_Points") &&
              itNames->second != std::string("Num_Fibers") )
            {
            if (itValues->second != INVALID_NUMBER)
              {
              clusterValue += npoints * itValues->second;
              }
            }
          else
            {
            clusterValue += itValues->second;
            }

          itCluster->second[itNames->second] = clusterValue;
          } ////for (itNames = names.begin(); itNames != names.end(); itNames++)
        } // if (isInCluster(itOutput->first, itClusterNames->first)
      } // for(itOutput = OutTable.begin(); itOutput != OutTable.end(); itOutput++)

      // second pass divide by npoints
      for (itNames = names.begin(); itNames != names.end(); itNames++)
        {
        itClusterValues = itCluster->second.find(itNames->second);
        if (itClusterValues != itCluster->second.end() && itNames->second != std::string("Num_Points") &&
              itNames->second != std::string("Num_Fibers") && npointsCluster )
          {
          double clusterValue = itClusterValues->second;
          itCluster->second[itNames->second] = clusterValue/npointsCluster;
          }
        }
    } //  for (itClusterNames = ClusterNames.begin(); itClusterNames!= ClusterNames.end(); itClusterNames++)
  return 1;
}

void printFlat(std::ofstream &ofs)
{
  std::stringstream ids;
  std::stringstream measureNames;
  std::stringstream measureValues;
  std::map<std::string, std::string>::iterator itClusterNames;

  std::map<std::string, std::string> names = getMeasureNames();

  for (itClusterNames = ClusterNames.begin(); itClusterNames!= ClusterNames.end(); itClusterNames++)
    {
    // find if this cluster in any other cluster
    bool topCluster = true;
    std::map<std::string, std::string>::iterator itClusterNames1;
    for (itClusterNames1 = ClusterNames.begin(); itClusterNames1!= ClusterNames.end(); itClusterNames1++)
      {
      if (isInCluster(itClusterNames->second, itClusterNames1->first) )
        {
        topCluster = false;
        break;
        }
      }

    if (topCluster)
    {
      // print it
      printCluster(itClusterNames->first, Clusters, names,
                   ids, measureNames, measureValues);

      // print all children clusters
      for (itClusterNames1 = ClusterNames.begin(); itClusterNames1!= ClusterNames.end(); itClusterNames1++)
        {
        if (isInCluster(itClusterNames1->second, itClusterNames->first) )
          {
          printCluster(itClusterNames1->first, Clusters, names,
                       ids, measureNames, measureValues);
          // print all fibers in this clusters
          std::map< std::string, std::map<std::string, double> >::iterator it;
          for(it = OutTable.begin(); it != OutTable.end(); it++)
            {
            if (isInCluster(it->first, itClusterNames1->first) )
              {
              printCluster(it->first, OutTable, names,
                           ids, measureNames, measureValues);
              }
            } //for(it = OutTable.begin(); it != OutTable.end(); it++)
          }
        }
      } // if (topCluster)
    } //   for (itClusterNames = ClusterNames.begin(); itClusterNames!= ClusterNames.end(); itClusterNames++)

  // if no clusters print fibers
  //if (ClusterNames.empty())
  //  {
    std::map< std::string, std::map<std::string, double> >::iterator it;
    for(it = OutTable.begin(); it != OutTable.end(); it++)
      {
      printCluster(it->first, OutTable, names,
                   ids, measureNames, measureValues);
      }
  //  }

  if (!ids.str().empty())
    {
    ofs << ids.str() << std::endl;
    ofs << measureNames.str() << std::endl;
    ofs << measureValues.str() << std::endl;
    }
}

void printCluster(const std::string &id,
                  std::map< std::string, std::map<std::string, double> > &output,
                  std::map<std::string, std::string> &names,
                  std::stringstream &ids,
                  std::stringstream &measureNames,
                  std::stringstream &measureValues)
{
  std::map< std::string, std::map<std::string, double> >::iterator it;
  std::map<std::string, double>::iterator it1;
  std::map<std::string, std::string>::iterator it2;

  it = output.find(id);
  if (it != output.end())
    {
    for (it2 = names.begin(); it2 != names.end(); it2++)
      {
      it1 = it->second.find(it2->second);
      if (it1 != it->second.end())
        {
        if (!ids.str().empty())
          {
          ids << ",";
          measureNames << ",";
          measureValues << ",";
          }
        ids << id;
        measureNames << it2->second;

        if (it1->second == INVALID_NUMBER)
          {
          measureValues << INVALID_NUMBER_PRINT;
          }
        else
          {
          measureValues << std::fixed << it1->second;
          }
        }
      }
    }
}
