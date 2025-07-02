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

#ifndef vtkMRMLJsonElement_Private_h
#define vtkMRMLJsonElement_Private_h

// STD includes
#include <memory>

// MRML includes
#include "vtkMRMLJsonElement.h"

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
class vtkMRMLJsonElement::vtkInternal
{
public:
  vtkInternal(vtkMRMLJsonElement* external);
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
  vtkMRMLJsonElement::Type GetValueType(rapidjson::Value& item);

  // Data
  std::shared_ptr<JsonDocumentContainer> JsonRoot;
  rapidjson::Value JsonValue;


protected:
  vtkMRMLJsonElement* External;

  friend class vtkMRMLJsonReader;
};

// Base class for writers
class BaseWriter
{
public:
  virtual ~BaseWriter() = default;
  virtual void SetFormatOptions(rapidjson::PrettyFormatOptions options) = 0;
  virtual void StartArray() = 0;
  virtual void EndArray() = 0;
  virtual void StartObject() = 0;
  virtual void EndObject() = 0;
  virtual void Key(const char*) = 0;
  virtual void String(const char*) = 0;
  virtual void Bool(bool) = 0;
  virtual void Int(int) = 0;
  virtual void Double(double) = 0;
};

// Derived class for FileWriter
class FileWriter : public BaseWriter
{
public:
  FileWriter(std::unique_ptr<rapidjson::PrettyWriter<rapidjson::FileWriteStream>> writer)
    : writer_(std::move(writer))
  {}

  void SetFormatOptions(rapidjson::PrettyFormatOptions options) override { writer_->SetFormatOptions(options); }

  void StartArray() override { writer_->StartArray(); }

  void EndArray() override { writer_->EndArray(); }

  void StartObject() override { writer_->StartObject(); }

  void EndObject() override { writer_->EndObject(); }

  void Key(const char* d) override { writer_->Key(d); }

  void String(const char* d) override { writer_->String(d); }

  void Bool(bool d) override { writer_->Bool(d); }

  void Int(int d) override { writer_->Int(d); }

  void Double(double d) override { writer_->Double(d); }

private:
  std::unique_ptr<rapidjson::PrettyWriter<rapidjson::FileWriteStream>> writer_;
};

// Derived class for StringWriter
class StringWriter : public BaseWriter
{
public:
  StringWriter(std::unique_ptr<rapidjson::PrettyWriter<rapidjson::StringBuffer>> writer)
    : writer_(std::move(writer))
  {}

  void SetFormatOptions(rapidjson::PrettyFormatOptions options) override { writer_->SetFormatOptions(options); }

  void StartArray() override { writer_->StartArray(); }

  void EndArray() override { writer_->EndArray(); }

  void StartObject() override { writer_->StartObject(); }

  void EndObject() override { writer_->EndObject(); }

  void Key(const char* d) override { writer_->Key(d); }

  void String(const char* d) override { writer_->String(d); }

  void Bool(bool d) override { writer_->Bool(d); }

  void Int(int d) override { writer_->Int(d); }

  void Double(double d) override { writer_->Double(d); }

private:
  std::unique_ptr<rapidjson::PrettyWriter<rapidjson::StringBuffer>> writer_;
};

//---------------------------------------------------------------------------
class vtkMRMLJsonWriter::vtkInternal
{
public:
  vtkInternal(vtkMRMLJsonWriter* external);
  ~vtkInternal();

  void WriteVector(double* v, int numberOfComponents = 3);
  vtkMRMLJsonWriter::vtkInternal* GetActiveWriter();

  void SetFileWriter(std::unique_ptr<rapidjson::PrettyWriter<rapidjson::FileWriteStream>> writer)
  {
    this->Writer = std::make_unique<FileWriter>(std::move(writer));
  }

  void SetStringWriter(std::unique_ptr<rapidjson::PrettyWriter<rapidjson::StringBuffer>> writer)
  {
    this->Writer = std::make_unique<StringWriter>(std::move(writer));
  }

  std::vector<char> WriteBuffer;
  FILE* WriteFileHandle{ 0 };
  std::unique_ptr<rapidjson::FileWriteStream> FileWriteStream;
  std::unique_ptr<rapidjson::StringBuffer> StringBuffer;
  std::unique_ptr<BaseWriter> Writer;

protected:
  vtkMRMLJsonWriter* External;
};

#endif
