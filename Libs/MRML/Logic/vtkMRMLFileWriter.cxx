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

#include "vtkMRMLFileWriter.h"

#include "vtkMRMLIOProperties.h"

#include <vtkObjectFactory.h>

#include <algorithm>

vtkStandardNewMacro(vtkMRMLFileWriter);

//----------------------------------------------------------------------------
vtkMRMLFileWriter::vtkMRMLFileWriter() = default;

//----------------------------------------------------------------------------
vtkMRMLFileWriter::~vtkMRMLFileWriter() = default;

//----------------------------------------------------------------------------
void vtkMRMLFileWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "NodeClassNames:";
  for (const std::string& className : this->NodeClassNames)
  {
    os << " " << className;
  }
  os << "\n";
  os << indent << "ConfidenceForMatchingClass: " << this->ConfidenceForMatchingClass << "\n";
  os << indent << "WrittenNodeIDs:";
  for (const std::string& nodeID : this->WrittenNodeIDs)
  {
    os << " " << nodeID;
  }
  os << "\n";
}

//----------------------------------------------------------------------------
double vtkMRMLFileWriter::CanWriteObjectConfidence(vtkObject* object)
{
  return this->CanWriteObject(object) ? this->ConfidenceForMatchingClass : 0.0;
}

//----------------------------------------------------------------------------
bool vtkMRMLFileWriter::CanWriteObject(vtkObject* object)
{
  if (!object)
  {
    return false;
  }
  for (const std::string& className : this->NodeClassNames)
  {
    if (object->IsA(className.c_str()))
    {
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLFileWriter::GetNameFiltersForObject(vtkObject* vtkNotUsed(object))
{
  return this->GetNameFilters();
}

//----------------------------------------------------------------------------
bool vtkMRMLFileWriter::Write(vtkMRMLIOProperties* vtkNotUsed(properties))
{
  // A writer that does its writing elsewhere (in Python, say) is called through its owner.
  this->ClearWrittenNodeIDs();
  return false;
}

//----------------------------------------------------------------------------
const std::vector<std::string>& vtkMRMLFileWriter::GetWrittenNodeIDs() const
{
  return this->WrittenNodeIDs;
}

//----------------------------------------------------------------------------
void vtkMRMLFileWriter::SetWrittenNodeIDs(const std::vector<std::string>& nodeIDs)
{
  if (this->WrittenNodeIDs == nodeIDs)
  {
    return;
  }
  this->WrittenNodeIDs = nodeIDs;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLFileWriter::AddWrittenNodeID(const std::string& nodeID)
{
  if (nodeID.empty())
  {
    return;
  }
  this->WrittenNodeIDs.push_back(nodeID);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLFileWriter::ClearWrittenNodeIDs()
{
  if (this->WrittenNodeIDs.empty())
  {
    return;
  }
  this->WrittenNodeIDs.clear();
  this->Modified();
}

//----------------------------------------------------------------------------
std::string vtkMRMLFileWriter::GetNodeClassName() const
{
  return this->NodeClassNames.empty() ? std::string() : this->NodeClassNames[0];
}

//----------------------------------------------------------------------------
void vtkMRMLFileWriter::SetNodeClassName(const std::string& className)
{
  std::vector<std::string> classNames;
  if (!className.empty())
  {
    classNames.push_back(className);
  }
  this->SetNodeClassNames(classNames);
}

//----------------------------------------------------------------------------
void vtkMRMLFileWriter::SetNodeClassNames(const std::vector<std::string>& classNames)
{
  if (this->NodeClassNames == classNames)
  {
    return;
  }
  this->NodeClassNames = classNames;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLFileWriter::AddNodeClassName(const std::string& className)
{
  if (className.empty())
  {
    return;
  }
  if (std::find(this->NodeClassNames.begin(), this->NodeClassNames.end(), className) != this->NodeClassNames.end())
  {
    return;
  }
  this->NodeClassNames.push_back(className);
  this->Modified();
}
