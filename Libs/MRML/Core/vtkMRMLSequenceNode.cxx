/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MRMLSequence includes
#include "vtkMRMLLinearTransformSequenceStorageNode.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLSequenceStorageNode.h"
#include "vtkMRMLStorableNode.h"
#include "vtkMRMLVolumeSequenceStorageNode.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkTimerLog.h>

// STD includes
#include <sstream>

#define SAFE_CHAR_POINTER(unsafeString) ( unsafeString==nullptr?"":unsafeString )

// This macro sets a member variable and sets both this node and the storage node as modified.
// This macro can be used for properties that are stored in both the scene and in the stored file.
#define vtkCxxSetVariableInDataAndStorageNodeMacro(name, type) \
  void vtkMRMLSequenceNode::Set##name(type arg) \
  { \
    if (arg == this->name) { return; } \
    this->name = arg; \
    this->StorableModifiedTime.Modified(); \
    this->Modified(); \
  }

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSequenceNode);
vtkCxxSetVariableInDataAndStorageNodeMacro(IndexName, const std::string&);
vtkCxxSetVariableInDataAndStorageNodeMacro(IndexUnit, const std::string&);
vtkCxxSetVariableInDataAndStorageNodeMacro(IndexType, int);
vtkCxxSetVariableInDataAndStorageNodeMacro(NumericIndexValueTolerance, double);

//----------------------------------------------------------------------------
vtkMRMLSequenceNode::vtkMRMLSequenceNode()
{
  this->SetIndexName("time");
  this->SetIndexUnit("s");
  this->HideFromEditorsOff();
  // sequence scene cannot be created here because vtkMRMLScene instantiates this node
  // in its constructor, which would lead to infinite loop
  this->SequenceScene = nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLSequenceNode::~vtkMRMLSequenceNode()
{
  if (this->SequenceScene)
    {
    this->SequenceScene->Delete();
    this->SequenceScene = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSequenceNode::RemoveAllDataNodes()
{
  this->IndexEntries.clear();
  if (!this->SequenceScene)
    {
    return;
    }
  this->SequenceScene->Delete();
  this->SequenceScene = nullptr;
  this->Modified();
  this->StorableModifiedTime.Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLSequenceNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkIndent indent(nIndent);

  of << indent << " indexName=\"" << this->IndexName << "\"";
  of << indent << " indexUnit=\"" << this->IndexUnit << "\"";

  std::string indexTypeString=GetIndexTypeAsString();
  of << indent << " indexType=\"" << indexTypeString << "\"";

  of << indent << " numericIndexValueTolerance=\"" << this->NumericIndexValueTolerance << "\"";

  of << indent << " indexValues=\"";
  for(std::deque< IndexEntryType >::iterator indexIt=this->IndexEntries.begin(); indexIt!=this->IndexEntries.end(); ++indexIt)
    {
    if (indexIt!=this->IndexEntries.begin())
      {
      // not the first index, add a separator before adding values
      of << ";";
      }
    if (indexIt->DataNode==nullptr)
      {
      // If we have a data node ID then store that, it is the most we know about the node that should be there
      if (!indexIt->DataNodeID.empty())
        {
        // this is normal when sequence node is in scene view
        of << indexIt->DataNodeID << ":" << indexIt->IndexValue;
        }
      else
        {
        vtkErrorMacro("Error while writing node "<<(this->GetID()?this->GetID():"(unknown)")
          << " to XML: data node is invalid at index value "<<indexIt->IndexValue);
        }
      }
    else
      {
      of << indexIt->DataNode->GetID() << ":" << indexIt->IndexValue;
      }
    }
  of << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLSequenceNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "indexName"))
      {
      this->SetIndexName(attValue);
      }
    else if (!strcmp(attName, "indexUnit"))
      {
      this->SetIndexUnit(attValue);
      }
    else if (!strcmp(attName, "indexType"))
      {
      int indexType=GetIndexTypeFromString(attValue);
      if (indexType<0 || indexType>=vtkMRMLSequenceNode::NumberOfIndexTypes)
        {
        vtkErrorMacro("Invalid index type: "<<(attValue?attValue:"(empty). Assuming TextIndex."));
        indexType=vtkMRMLSequenceNode::TextIndex;
        }
      SetIndexType(indexType);
      }
    else if (!strcmp(attName, "numericIndexValueTolerance"))
      {
      std::stringstream ss;
      ss << attValue;
      double numericIndexValueTolerance = 0.001;
      ss >> numericIndexValueTolerance;
      this->SetNumericIndexValueTolerance(numericIndexValueTolerance);
      }
    else if (!strcmp(attName, "indexValues"))
      {
      ReadIndexValues(attValue);
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSequenceNode::ReadIndexValues(const std::string& indexText)
{
  bool modified = false;

  if (!this->IndexEntries.empty())
    {
    this->IndexEntries.clear();
    modified = true;
    }

  std::stringstream ss(indexText);
  std::string nodeId_indexValue;
  while (std::getline(ss, nodeId_indexValue, ';'))
    {
    std::size_t indexValueSeparatorPos = nodeId_indexValue.find_first_of(':');
    if (indexValueSeparatorPos>0 && indexValueSeparatorPos != std::string::npos)
      {
      std::string nodeId = nodeId_indexValue.substr(0, indexValueSeparatorPos);
      std::string indexValue = nodeId_indexValue.substr(indexValueSeparatorPos+1, nodeId_indexValue.size()-indexValueSeparatorPos-1);

      IndexEntryType indexEntry;
      indexEntry.IndexValue=indexValue;
      // The nodes are not read yet, so we can only store the node ID and get the pointer to the node later (in UpdateScene())
      indexEntry.DataNodeID=nodeId;
      indexEntry.DataNode=nullptr;
      this->IndexEntries.push_back(indexEntry);
      modified = true;
      }
    }

  if (modified)
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLSequenceNode::Copy(vtkMRMLNode *anode)
{
  int wasModified = this->StartModify();
  Superclass::Copy(anode);

  vtkMRMLSequenceNode *snode = (vtkMRMLSequenceNode *) anode;
  if (!snode)
    {
    vtkErrorMacro("vtkMRMLSequenceNode::Copy failed: invalid input node");
    return;
    }

  this->SetIndexName(snode->GetIndexName());
  this->SetIndexUnit(snode->GetIndexUnit());
  this->SetIndexType(snode->GetIndexType());
  this->SetNumericIndexValueTolerance(snode->GetNumericIndexValueTolerance());

  // Clear nodes: RemoveAllNodes is not a public method, so it's simpler to just delete and recreate the scene
  if (this->SequenceScene)
    {
    this->SequenceScene->Delete();
    }
  this->SequenceScene=vtkMRMLScene::New();

  if (snode->SequenceScene)
    {
    for (int n = 0; n < snode->SequenceScene->GetNodes()->GetNumberOfItems(); n++)
      {
      vtkMRMLNode* node = (vtkMRMLNode*)snode->SequenceScene->GetNodes()->GetItemAsObject(n);
      if (node == nullptr)
        {
        vtkErrorMacro("Invalid node in vtkMRMLSequenceNode");
        continue;
        }
      this->DeepCopyNodeToScene(node, this->SequenceScene);
      }
    }

  this->IndexEntries.clear();
  for(std::deque< IndexEntryType >::iterator sourceIndexIt=snode->IndexEntries.begin(); sourceIndexIt!=snode->IndexEntries.end(); ++sourceIndexIt)
    {
    IndexEntryType seqItem;
    seqItem.IndexValue=sourceIndexIt->IndexValue;
    seqItem.DataNode = nullptr;
    if (sourceIndexIt->DataNode!=nullptr)
      {
      seqItem.DataNode=this->SequenceScene->GetNodeByID(sourceIndexIt->DataNode->GetID());
      seqItem.DataNodeID.clear();
      }
    if (seqItem.DataNode==nullptr)
      {
      // data node was not found, at least copy its ID
      seqItem.DataNodeID=sourceIndexIt->DataNodeID;
      if (seqItem.DataNodeID.empty())
        {
        vtkWarningMacro("vtkMRMLSequenceNode::Copy: node was not found at index value "<<seqItem.IndexValue);
        }
      }
    this->IndexEntries.push_back(seqItem);
    }
  this->Modified();
  this->StorableModifiedTime.Modified();

  this->EndModify(wasModified);
}

//----------------------------------------------------------------------------
void vtkMRMLSequenceNode::CopySequenceIndex(vtkMRMLNode *anode)
{
  int wasModified = this->StartModify();
  vtkMRMLSequenceNode *snode = (vtkMRMLSequenceNode *)anode;
  this->SetIndexName(snode->GetIndexName());
  this->SetIndexUnit(snode->GetIndexUnit());
  this->SetIndexType(snode->GetIndexType());
  this->SetNumericIndexValueTolerance(snode->GetNumericIndexValueTolerance());
  if (this->IndexEntries.size() > 0 || snode->IndexEntries.size() > 0)
    {
    this->IndexEntries.clear();
    for (std::deque< IndexEntryType >::iterator sourceIndexIt = snode->IndexEntries.begin(); sourceIndexIt != snode->IndexEntries.end(); ++sourceIndexIt)
      {
      IndexEntryType seqItem;
      seqItem.IndexValue = sourceIndexIt->IndexValue;
      if (sourceIndexIt->DataNode != nullptr)
        {
        seqItem.DataNodeID = sourceIndexIt->DataNode->GetID();
        }
      else
        {
        seqItem.DataNodeID = sourceIndexIt->DataNodeID;
        }
      seqItem.DataNode = nullptr;
      this->IndexEntries.push_back(seqItem);
      }
    this->Modified();
    }
  this->EndModify(wasModified);
}

//----------------------------------------------------------------------------
void vtkMRMLSequenceNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);
  os << indent << "indexName: " << this->IndexName + "\n";
  os << indent << "indexUnit: " << this->IndexUnit + "\n";

  std::string indexTypeString = GetIndexTypeAsString();
  os << indent << "indexType: " << indexTypeString << "\n";

  os << indent << "numericIndexValueTolerance: " << this->NumericIndexValueTolerance << "\n";

  os << indent << "indexValues: ";
  if (this->IndexEntries.empty())
    {
    os << "(none)";
    }
  else
    {
    os << this->IndexEntries[0].IndexValue;
    if (this->IndexEntries.size() > 1)
      {
      os << " ... " << this->IndexEntries[this->IndexEntries.size()-1].IndexValue;
      os << " (" << this->IndexEntries.size() << " items)";
      }
    }
  os << "\n";
}

//----------------------------------------------------------------------------
bool vtkMRMLSequenceNode::UpdateDataNodeAtValue(vtkMRMLNode* node, const std::string& indexValue, bool shallowCopy /* = false */)
{
  if (node==nullptr)
    {
    vtkErrorMacro("vtkMRMLSequenceNode::UpdateDataNodeAtValue failed, invalid node");
    return false;
    }
  vtkMRMLNode* nodeToBeUpdated = this->GetDataNodeAtValue(indexValue);
  if (!nodeToBeUpdated)
    {
    vtkDebugMacro("vtkMRMLSequenceNode::UpdateDataNodeAtValue failed, indexValue not found");
    return false;
    }
  nodeToBeUpdated->CopyContent(node, !shallowCopy);
  this->Modified();
  this->StorableModifiedTime.Modified();
  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLSequenceNode::GetInsertPosition(const std::string& indexValue)
{
  int insertPosition = this->IndexEntries.size();
  if (this->IndexType == vtkMRMLSequenceNode::NumericIndex && !this->IndexEntries.empty())
    {
    int itemNumber = this->GetItemNumberFromIndexValue(indexValue, false);
    double numericIndexValue = atof(indexValue.c_str());
    double foundNumericIndexValue = atof(this->IndexEntries[itemNumber].IndexValue.c_str());
    if (numericIndexValue < foundNumericIndexValue) // Deals with case of index value being smaller than any in the sequence and numeric tolerances
      {
      insertPosition = itemNumber;
      }
    else
      {
      insertPosition = itemNumber + 1;
      }
    }
  return insertPosition;
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLSequenceNode::SetDataNodeAtValue(vtkMRMLNode* node, const std::string& indexValue)
{
  if (node == nullptr)
    {
    vtkErrorMacro("vtkMRMLSequenceNode::SetDataNodeAtValue failed, invalid node");
    return nullptr;
    }
  // Make sure the sequence scene is created
  this->GetSequenceScene();
  // Add a copy of the node to the sequence's scene
  vtkMRMLNode* newNode = this->DeepCopyNodeToScene(node, this->SequenceScene);
  int seqItemIndex = this->GetItemNumberFromIndexValue(indexValue);
  if (seqItemIndex<0)
    {
    // The sequence item doesn't exist yet
    seqItemIndex = GetInsertPosition(indexValue);
    // Create new item
    IndexEntryType seqItem;
    seqItem.IndexValue = indexValue;
    this->IndexEntries.insert(this->IndexEntries.begin() + seqItemIndex, seqItem);
    }
  this->IndexEntries[seqItemIndex].DataNode = newNode;
  this->IndexEntries[seqItemIndex].DataNodeID.clear();
  this->Modified();
  this->StorableModifiedTime.Modified();
  return newNode;
}

//----------------------------------------------------------------------------
void vtkMRMLSequenceNode::RemoveDataNodeAtValue(const std::string& indexValue)
{
  int seqItemIndex = GetItemNumberFromIndexValue(indexValue);
  if (seqItemIndex<0)
    {
    vtkWarningMacro("vtkMRMLSequenceNode::RemoveDataNodeAtValue: node was not found at index value "<<indexValue);
    return;
    }
  if (!this->SequenceScene)
    {
    vtkWarningMacro("vtkMRMLSequenceNode::RemoveDataNodeAtValue: internal scene is already empty");
    return;
    }
  // TODO: remove associated nodes as well (such as storage node)?
  this->SequenceScene->RemoveNode(this->IndexEntries[seqItemIndex].DataNode);
  this->IndexEntries.erase(this->IndexEntries.begin()+seqItemIndex);
  this->Modified();
  this->StorableModifiedTime.Modified();
}

//---------------------------------------------------------------------------
int vtkMRMLSequenceNode::GetItemNumberFromIndexValue(const std::string& indexValue, bool exactMatchRequired /* =true */)
{
  int numberOfSeqItems=this->IndexEntries.size();
  if (numberOfSeqItems == 0)
    {
    return -1;
    }

  // Binary search will be faster for numeric index
  if (this->IndexType == NumericIndex)
    {
    int lowerBound = 0;
    int upperBound = numberOfSeqItems-1;

    // Deal with index values not within the range of index values in the Sequence
    double numericIndexValue = atof(indexValue.c_str());
    double lowerNumericIndexValue = atof(this->IndexEntries[lowerBound].IndexValue.c_str());
    double upperNumericIndexValue = atof(this->IndexEntries[upperBound].IndexValue.c_str());
    if (numericIndexValue <= lowerNumericIndexValue + this->NumericIndexValueTolerance)
      {
      if (numericIndexValue < lowerNumericIndexValue - this->NumericIndexValueTolerance && exactMatchRequired)
        {
        return -1;
        }
      else
        {
        return lowerBound;
        }
      }
    if (numericIndexValue >= upperNumericIndexValue - this->NumericIndexValueTolerance)
      {
      if (numericIndexValue > upperNumericIndexValue + this->NumericIndexValueTolerance && exactMatchRequired)
        {
        return -1;
        }
      else
        {
        return upperBound;
        }
      }

    while (upperBound - lowerBound > 1)
      {
      // Note that if middle is equal to either lowerBound or upperBound then upperBound - lowerBound <= 1
      int middle = int((lowerBound + upperBound)/2);
      double middleNumericIndexValue = atof(this->IndexEntries[middle].IndexValue.c_str());
      if (fabs(numericIndexValue - middleNumericIndexValue) <= this->NumericIndexValueTolerance)
        {
        return middle;
        }
      if (numericIndexValue > middleNumericIndexValue)
        {
        lowerBound = middle;
        }
      if (numericIndexValue < middleNumericIndexValue)
        {
        upperBound = middle;
        }
      }
    if (!exactMatchRequired)
      {
      return lowerBound;
      }
    }

  // Need linear search for non-numeric index
  for (int i=0; i<numberOfSeqItems; i++)
    {
    if (this->IndexEntries[i].IndexValue.compare(indexValue)==0)
      {
      return i;
      }
    }

  return -1;
}

//---------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLSequenceNode::GetDataNodeAtValue(const std::string& indexValue, bool exactMatchRequired /* =true */)
{
  if (!this->SequenceScene)
    {
    // no data nodes are stored
    return nullptr;
    }
  int seqItemIndex = this->GetItemNumberFromIndexValue(indexValue, exactMatchRequired);
  if (seqItemIndex < 0)
    {
    // not found
    return nullptr;
    }
  return this->IndexEntries[seqItemIndex].DataNode;
}

//---------------------------------------------------------------------------
std::string vtkMRMLSequenceNode::GetNthIndexValue(int seqItemIndex)
{
  if (seqItemIndex<0 || seqItemIndex>=static_cast<int>(this->IndexEntries.size()))
    {
    vtkErrorMacro("vtkMRMLSequenceNode::GetNthIndexValue failed, invalid seqItemIndex value: "<<seqItemIndex);
    return "";
    }
  return this->IndexEntries[seqItemIndex].IndexValue;
}

//-----------------------------------------------------------------------------
int vtkMRMLSequenceNode::GetNumberOfDataNodes()
{
  return this->IndexEntries.size();
}

//-----------------------------------------------------------------------------
bool vtkMRMLSequenceNode::UpdateIndexValue(const std::string& oldIndexValue, const std::string& newIndexValue)
{
  if (oldIndexValue == newIndexValue)
    {
    // no change
    return true;
    }
  int oldSeqItemIndex = GetItemNumberFromIndexValue(oldIndexValue);
  if (oldSeqItemIndex<0)
    {
    vtkErrorMacro("vtkMRMLSequenceNode::UpdateIndexValue failed, no data node found with index value "<<oldIndexValue);
    return false;
    }
  if (this->GetItemNumberFromIndexValue(newIndexValue) >= 0)
    {
    vtkErrorMacro("vtkMRMLSequenceNode::UpdateIndexValue failed, data node is already defined at index value " << newIndexValue);
    return false;
    }
  // Update the index value
  this->IndexEntries[oldSeqItemIndex].IndexValue = newIndexValue;
  if (this->IndexType == vtkMRMLSequenceNode::NumericIndex)
    {
    IndexEntryType movingEntry = this->IndexEntries[oldSeqItemIndex];
    // Remove from current position
    this->IndexEntries.erase(this->IndexEntries.begin() + oldSeqItemIndex);
    // Insert into new position
    int insertPosition = this->GetInsertPosition(newIndexValue);
    this->IndexEntries.insert(this->IndexEntries.begin() + insertPosition, movingEntry);
    }
  this->Modified();
  this->StorableModifiedTime.Modified();
  return true;
}

//-----------------------------------------------------------------------------
std::string vtkMRMLSequenceNode::GetDataNodeClassName()
{
  if (this->IndexEntries.empty())
    {
    return "";
    }
  // All the nodes should be of the same class, so just get the class from the first one
  vtkMRMLNode* node=this->IndexEntries[0].DataNode;
  if (node==nullptr)
    {
    vtkErrorMacro("vtkMRMLSequenceNode::GetDataNodeClassName node is invalid");
    return "";
    }
  const char* className=node->GetClassName();
  return SAFE_CHAR_POINTER(className);
}

//-----------------------------------------------------------------------------
std::string vtkMRMLSequenceNode::GetDataNodeTagName()
{
  std::string undefinedReturn="undefined";
  if (this->IndexEntries.empty())
    {
    return undefinedReturn;
    }
  // All the nodes should be of the same class, so just get the class from the first one
  vtkMRMLNode* node=this->IndexEntries[0].DataNode;
  if (node==nullptr)
    {
    vtkErrorMacro("vtkMRMLSequenceNode::GetDataNodeClassName node is invalid");
    return undefinedReturn;
    }
  const char* tagName=node->GetNodeTagName();
  if (tagName==nullptr)
    {
    return undefinedReturn;
    }
  return tagName;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLSequenceNode::GetNthDataNode(int itemNumber)
{
  if (static_cast<int>(this->IndexEntries.size())<=itemNumber)
    {
    vtkErrorMacro("vtkMRMLSequenceNode::GetNthDataNode failed: itemNumber "<<itemNumber<<" is out of range");
    return nullptr;
    }
  return this->IndexEntries[itemNumber].DataNode;
}

//-----------------------------------------------------------------------------
vtkMRMLScene* vtkMRMLSequenceNode::GetSequenceScene(bool autoCreate/*=true*/)
{
  if (!this->SequenceScene && autoCreate)
    {
    this->SequenceScene = vtkMRMLScene::New();
    }
  return this->SequenceScene;
}

//-----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLSequenceNode::CreateDefaultStorageNode()
{
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkDebugMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }

  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass(this->GetDefaultStorageNodeClassName().c_str()));
}

//-----------------------------------------------------------
std::string vtkMRMLSequenceNode::GetDefaultStorageNodeClassName(const char* filename /* =nullptr */)
{
  // No need to create storage node if there are no nodes to store
  if (this->GetSequenceScene() == nullptr || this->GetSequenceScene()->GetNumberOfNodes() == 0)
    {
    return "";
    }

  // Use specific sequence storage node, if possible
  vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetNthDataNode(0));
  if (storableNode)
    {
    vtkSmartPointer<vtkMRMLStorageNode> storageNode = vtkSmartPointer<vtkMRMLStorageNode>::Take(
      storableNode->CreateDefaultSequenceStorageNode());
    if (storageNode)
      {
      // Filename is not specified or it is specified and there is a supported file extension
      if (!filename || (filename && !storageNode->GetSupportedFileExtension(filename, false, true).empty()))
        {
        if (storageNode->CanWriteFromReferenceNode(this))
          {
          return storageNode->GetClassName();
          }
        }
      }
    }

  // Use generic storage node
  return "vtkMRMLSequenceStorageNode";
}

//-----------------------------------------------------------
void vtkMRMLSequenceNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);

  // By now the storage node imported the sequence scene, so we can get the pointers to the data nodes
  this->UpdateSequenceIndex();
}

//-----------------------------------------------------------
void vtkMRMLSequenceNode::UpdateSequenceIndex()
{
  if (!this->SequenceScene)
    {
    // nothing to update
    return;
    }
  for (std::deque< IndexEntryType >::iterator indexIt = this->IndexEntries.begin(); indexIt != this->IndexEntries.end(); ++indexIt)
    {
    if (indexIt->DataNode == nullptr)
      {
      indexIt->DataNode = this->SequenceScene->GetNodeByID(indexIt->DataNodeID);
      if (indexIt->DataNode != nullptr)
        {
        // clear the ID to remove redundancy in the data
        indexIt->DataNodeID.clear();
        }
      }
    }
}

//-----------------------------------------------------------
void vtkMRMLSequenceNode::SetIndexTypeFromString(const char *indexTypeString)
{
  int indexType=GetIndexTypeFromString(indexTypeString);
  this->SetIndexType(indexType);
}

//-----------------------------------------------------------
std::string vtkMRMLSequenceNode::GetIndexTypeAsString()
{
  return vtkMRMLSequenceNode::GetIndexTypeAsString(this->IndexType);
}

//-----------------------------------------------------------
std::string vtkMRMLSequenceNode::GetIndexTypeAsString(int indexType)
{
  switch (indexType)
    {
    case vtkMRMLSequenceNode::NumericIndex: return "numeric";
    case vtkMRMLSequenceNode::TextIndex: return "text";
    default:
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLSequenceNode::GetIndexTypeFromString(const std::string& indexTypeString)
{
  for (int i=0; i<vtkMRMLSequenceNode::NumberOfIndexTypes; i++)
    {
    if (indexTypeString == GetIndexTypeAsString(i))
      {
      // found it
      return i;
      }
    }
  return -1;
}

vtkMRMLNode* vtkMRMLSequenceNode::DeepCopyNodeToScene(vtkMRMLNode* source, vtkMRMLScene* scene)
{
  if (source == nullptr)
    {
    vtkGenericWarningMacro("NodeSequencer::CopyNode failed, invalid node");
    return nullptr;
    }
  std::string baseName = "Data";
  if (source->GetAttribute("Sequences.BaseName") != 0)
    {
    baseName = source->GetAttribute("Sequences.BaseName");
    }
  else if (source->GetName() != 0)
    {
    baseName = source->GetName();
    }
  std::string newNodeName = baseName;

  vtkSmartPointer<vtkMRMLNode> target = vtkSmartPointer<vtkMRMLNode>::Take(source->CreateNodeInstance());
  target->CopyContent(source); // deep-copy

  // Generating unique node names is slow, and makes adding many nodes to a sequence too slow
  // We will instead ensure that all file names for storable nodes are unique when saving
  target->SetName(newNodeName.c_str());
  target->SetAttribute("Sequences.BaseName", baseName.c_str());

  vtkMRMLNode* addedTargetNode = scene->AddNode(target);
  return addedTargetNode;
}
