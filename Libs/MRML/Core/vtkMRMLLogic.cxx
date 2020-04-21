
// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLDisplayNode.h"

// MRMLLogic includes
#include "vtkMRMLLogic.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <set>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLLogic);

//------------------------------------------------------------------------------
vtkMRMLLogic::vtkMRMLLogic()
{
  this->Scene = nullptr;
}

//------------------------------------------------------------------------------
vtkMRMLLogic::~vtkMRMLLogic() = default;

void vtkMRMLLogic::RemoveUnreferencedStorageNodes()
{
  if (this->Scene == nullptr)
    {
    return;
    }
  std::set<vtkMRMLNode *> referencedNodes;
  std::set<vtkMRMLNode *>::iterator iter;
  std::vector<vtkMRMLNode *> storableNodes;
  std::vector<vtkMRMLNode *> storageNodes;
  this->Scene->GetNodesByClass("vtkMRMLStorableNode", storableNodes);
  this->Scene->GetNodesByClass("vtkMRMLStorageNode", storageNodes);

  vtkMRMLNode *node = nullptr;
  vtkMRMLStorableNode *storableNode = nullptr;
  vtkMRMLStorageNode *storageNode = nullptr;
  unsigned int i;
  for (i=0; i<storableNodes.size(); i++)
    {
    node = storableNodes[i];
    if (node)
      {
      storableNode = vtkMRMLStorableNode::SafeDownCast(node);
      }
    else
      {
      continue;
      }
    storageNode = storableNode->GetStorageNode();
    if (storageNode)
      {
      referencedNodes.insert(storageNode);
      }
    }

  for (i=0; i<storageNodes.size(); i++)
    {
    node = storageNodes[i];
    if (node)
      {
      storageNode = vtkMRMLStorageNode::SafeDownCast(node);
      }
    else
      {
      continue;
      }
    iter = referencedNodes.find(storageNode);
    if (iter == referencedNodes.end())
      {
      this->Scene->RemoveNode(storageNode);
      }
    }
}

void vtkMRMLLogic::RemoveUnreferencedDisplayNodes()
{
  if (this->Scene == nullptr)
    {
    return;
    }
  std::set<vtkMRMLNode *> referencedNodes;
  std::set<vtkMRMLNode *>::iterator iter;
  std::vector<vtkMRMLNode *> displayableNodes;
  std::vector<vtkMRMLNode *> displayNodes;
  this->Scene->GetNodesByClass("vtkMRMLDisplayableNode", displayableNodes);
  this->Scene->GetNodesByClass("vtkMRMLDisplayNode", displayNodes);

  vtkMRMLNode *node = nullptr;
  vtkMRMLDisplayableNode *displayableNode = nullptr;
  vtkMRMLDisplayNode *displayNode = nullptr;
  unsigned int i;
  for (i=0; i<displayableNodes.size(); i++)
    {
    node = displayableNodes[i];
    if (node)
      {
      displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
      }
    else
      {
      continue;
      }
    int numDisplayNodes = displayableNode->GetNumberOfDisplayNodes();
    for (int n=0; n<numDisplayNodes; n++)
      {
      displayNode = displayableNode->GetNthDisplayNode(n);
      if (displayNode)
        {
        referencedNodes.insert(displayNode);
        }
      }
    }

  for (i=0; i<displayNodes.size(); i++)
    {
    node = displayNodes[i];
    if (node)
      {
      displayNode = vtkMRMLDisplayNode::SafeDownCast(node);
      }
    else
      {
      continue;
      }
    iter = referencedNodes.find(displayNode);
    if (iter == referencedNodes.end())
      {
      this->Scene->RemoveNode(displayNode);
      }
    }
}

//----------------------------------------------------------------------------
std::string vtkMRMLLogic::GetApplicationHomeDirectory()
{
  std::string applicationHome;
  if (vtksys::SystemTools::GetEnv(MRML_APPLICATION_HOME_DIR_ENV) != nullptr)
    {
    applicationHome = std::string(vtksys::SystemTools::GetEnv(MRML_APPLICATION_HOME_DIR_ENV));
    }
  else
    {
    if (vtksys::SystemTools::GetEnv("PWD") != nullptr)
      {
      applicationHome =  std::string(vtksys::SystemTools::GetEnv("PWD"));
      }
    else
      {
      applicationHome =  std::string("");
      }
    }
  return applicationHome;
}

//----------------------------------------------------------------------------
std::string vtkMRMLLogic::GetApplicationShareDirectory()
{
  std::string applicationHome = vtkMRMLLogic::GetApplicationHomeDirectory();
  std::vector<std::string> filesVector;
  filesVector.emplace_back(""); // for relative path
  filesVector.push_back(applicationHome);
  filesVector.emplace_back(MRML_APPLICATION_SHARE_SUBDIR);
  std::string applicationShare = vtksys::SystemTools::JoinPath(filesVector);

  return applicationShare;
}
