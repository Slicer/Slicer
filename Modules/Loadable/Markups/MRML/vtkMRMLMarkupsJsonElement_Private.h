/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef vtkMRMLMarkupsJsonElement_Private_h
#define vtkMRMLMarkupsJsonElement_Private_h

// STD includes
#include <memory>

// MRML includes
#include "vtkMRMLMarkupsJsonElement.h"

// Relax JSON standard and allow reading/writing of nan and inf
// values. Such values should not normally occur, but if they do then
// it is easier to troubleshoot problems if numerical values are the
// same in memory and files.
// kWriteNanAndInfFlag = 2,        //!< Allow writing of Infinity, -Infinity and NaN.
#define RAPIDJSON_WRITE_DEFAULT_FLAGS 2
// kParseNanAndInfFlag = 256,      //!< Allow parsing NaN, Inf, Infinity, -Inf and -Infinity as doubles.
#define RAPIDJSON_PARSE_DEFAULT_FLAGS 256

#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"

#include <deque>
#include <memory>

//---------------------------------------------------------------------------
class vtkMRMLMarkupsJsonElement::vtkInternal
{
public:
  vtkInternal(vtkMRMLMarkupsJsonElement* external);
  ~vtkInternal();

  struct JsonDocumentContainer
  {
    JsonDocumentContainer() { this->Document = new rapidjson::Document; }
    virtual ~JsonDocumentContainer() { delete this->Document; }
    JsonDocumentContainer(const JsonDocumentContainer&) = delete;
    JsonDocumentContainer& operator=(const JsonDocumentContainer&) = delete;
    rapidjson::Document* Document;
  };

  // Helper methods
  bool ReadVector(rapidjson::Value& item, double* v, int numberOfComponents = 3);

  // Data
  std::shared_ptr<JsonDocumentContainer> JsonRoot;
  rapidjson::Value JsonValue;

protected:
  vtkMRMLMarkupsJsonElement* External;

  friend class vtkMRMLMarkupsJsonReader;
};

//---------------------------------------------------------------------------
class vtkMRMLMarkupsJsonWriter::vtkInternal
{
public:
  vtkInternal(vtkMRMLMarkupsJsonWriter* external);
  ~vtkInternal();

  void WriteVector(double* v, int numberOfComponents = 3);

  std::vector<char> WriteBuffer;
  FILE* WriteFileHandle{ 0 };
  std::unique_ptr<rapidjson::FileWriteStream> FileWriteStream;
  std::unique_ptr<rapidjson::PrettyWriter<rapidjson::FileWriteStream>> Writer;

protected:
  vtkMRMLMarkupsJsonWriter* External;
};

#endif
