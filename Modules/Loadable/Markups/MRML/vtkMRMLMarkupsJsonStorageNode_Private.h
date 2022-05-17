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

// STD includes
#include <memory>

// MRML includes
#include "vtkMRMLMarkupsJsonStorageNode.h"

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

//---------------------------------------------------------------------------
class vtkMRMLMarkupsJsonStorageNode::vtkInternal
{
public:
  vtkInternal(vtkMRMLMarkupsJsonStorageNode* external);
  ~vtkInternal();

  // Reader
  std::unique_ptr<rapidjson::Document> CreateJsonDocumentFromFile(const char* filePath);
  std::string GetMarkupsClassNameFromMarkupsType(std::string markupsType);
  std::string GetMarkupsClassNameFromJsonValue(rapidjson::Value& markupObject);
  virtual bool UpdateMarkupsNodeFromJsonValue (vtkMRMLMarkupsNode* markupsNode, rapidjson::Value& markupObject);
  bool UpdateMarkupsDisplayNodeFromJsonValue(vtkMRMLMarkupsDisplayNode* displayNode, rapidjson::Value& markupObject);
  bool ReadVector(rapidjson::Value& item, double* v, int numberOfComponents=3);
  bool ReadControlPoints(rapidjson::Value& item, int coordinateSystem, vtkMRMLMarkupsNode* markupsNode);
  bool ReadMeasurements(rapidjson::Value& item, vtkMRMLMarkupsNode* markupsNode);

  // Writer
  virtual bool WriteMarkup(rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer, vtkMRMLMarkupsNode* markupsNode);
  virtual bool WriteBasicProperties(rapidjson::PrettyWriter<rapidjson::FileWriteStream> &writer, vtkMRMLMarkupsNode* markupsNode);
  virtual bool WriteControlPoints(rapidjson::PrettyWriter<rapidjson::FileWriteStream> &writer, vtkMRMLMarkupsNode* markupsNode);
  virtual bool WriteMeasurements(rapidjson::PrettyWriter<rapidjson::FileWriteStream> &writer, vtkMRMLMarkupsNode* markupsNode);
  virtual bool WriteDisplayProperties(rapidjson::PrettyWriter<rapidjson::FileWriteStream> &writer, vtkMRMLMarkupsDisplayNode* markupsDisplayNode);
  void WriteVector(rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer, double* v, int numberOfComponents = 3);

  std::string GetCoordinateUnitsFromSceneAsString(vtkMRMLMarkupsNode* markupsNode);

protected:
  vtkMRMLMarkupsJsonStorageNode* External;
};
