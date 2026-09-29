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

// Qt includes
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

// Slicer includes
#include "qSlicerFileReader.h"
#include "qSlicerNodeWriter.h"
#include "qSlicerVTKFileReader.h"
#include "qSlicerVTKFileWriter.h"

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLFileReader.h>
#include <vtkMRMLFileWriter.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTextNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <string>

namespace
{

//-----------------------------------------------------------------------------
/// VTK-based writer that returns a specific confidence value for the objects that it can write.
class vtkSpecificConfidenceWriter : public vtkMRMLFileWriter
{
public:
  static vtkSpecificConfidenceWriter* New();
  vtkTypeMacro(vtkSpecificConfidenceWriter, vtkMRMLFileWriter);
  double CanWriteObjectConfidence(vtkObject* object) override { return this->CanWriteObject(object) ? 0.7 : 0.0; }
};
vtkStandardNewMacro(vtkSpecificConfidenceWriter);

//-----------------------------------------------------------------------------
/// Qt-based writer that overrides canWriteObject() of the VTK-based writer.
class qSlicerRestrictedVTKFileWriter : public qSlicerVTKFileWriter
{
public:
  qSlicerRestrictedVTKFileWriter(vtkMRMLFileWriter* writer)
    : qSlicerVTKFileWriter(writer)
  {
  }
  bool canWriteObject(vtkObject* object) const override
  {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(object);
    return node && node->GetName() && std::string(node->GetName()) == "Allowed" && this->qSlicerVTKFileWriter::canWriteObject(object);
  }
};

//-----------------------------------------------------------------------------
/// Node writer (for example, in an extension) that overrides canWriteObject().
class qSlicerNeverWritingNodeWriter : public qSlicerNodeWriter
{
public:
  qSlicerNeverWritingNodeWriter()
    : qSlicerNodeWriter("Never writing", QString("NeverFile"), QStringList() << "vtkMRMLTextNode", false, nullptr)
  {
  }
  bool canWriteObject(vtkObject* vtkNotUsed(object)) const override { return false; }
};

//-----------------------------------------------------------------------------
/// Node writer (for example, in an extension) that overrides canWriteObject() to write more node types
/// than the node class names of the VTK-based writer.
class qSlicerExtendedNodeWriter : public qSlicerNodeWriter
{
public:
  qSlicerExtendedNodeWriter()
    : qSlicerNodeWriter("Extended", QString("ExtendedFile"), QStringList() << "vtkMRMLScalarVolumeNode", false, nullptr)
  {
  }
  bool canWriteObject(vtkObject* object) const override
  {
    const bool isTextNode = (vtkMRMLTextNode::SafeDownCast(object) != nullptr);
    return isTextNode || this->qSlicerNodeWriter::canWriteObject(object);
  }
};

//-----------------------------------------------------------------------------
/// VTK-based node writer that overrides Write().
class vtkCountingNodeWriter : public vtkMRMLNodeWriter
{
public:
  static vtkCountingNodeWriter* New();
  vtkTypeMacro(vtkCountingNodeWriter, vtkMRMLNodeWriter);
  bool Write(vtkMRMLIOProperties* properties) override
  {
    ++this->WriteCount;
    return this->Superclass::Write(properties);
  }
  int WriteCount{ 0 };
};
vtkStandardNewMacro(vtkCountingNodeWriter);

//-----------------------------------------------------------------------------
/// Node writer (for example, in an extension) that uses a vtkMRMLNodeWriter subclass.
class qSlicerCountingNodeWriter : public qSlicerNodeWriter
{
public:
  qSlicerCountingNodeWriter()
    : qSlicerNodeWriter(nullptr)
  {
    this->setFileWriter(this->CountingWriter);
    this->setNodeClassNames(QStringList() << "vtkMRMLTextNode");
  }
  vtkNew<vtkCountingNodeWriter> CountingWriter;
};

//-----------------------------------------------------------------------------
/// Legacy Qt-based reader that does not use a VTK-based reader.
class qSlicerLegacyTestFileReader : public qSlicerFileReader
{
public:
  QString description() const override { return "Legacy test file"; }
  IOFileType fileType() const override { return QString("LegacyTestFile"); }
  QStringList extensions() const override { return QStringList() << "Legacy test file (*.tst)"; }
};

} // namespace

//-----------------------------------------------------------------------------
int qSlicerVTKFileReaderWriterTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  QTemporaryDir tempDir;
  CHECK_BOOL(tempDir.isValid(), true);
  const QString filePath = QDir(tempDir.path()).filePath("file.tst");
  {
    QFile file(filePath);
    CHECK_BOOL(file.open(QIODevice::WriteOnly), true);
    file.write("content");
  }

  vtkNew<vtkMRMLScene> scene;
  vtkMRMLNode* textNode = scene->AddNewNodeByClass("vtkMRMLTextNode", "Text");
  vtkMRMLNode* allowedTextNode = scene->AddNewNodeByClass("vtkMRMLTextNode", "Allowed");

  // Legacy reader: no VTK-based reader, confidence is computed from the matched extension length
  {
    qSlicerLegacyTestFileReader legacyReader;
    CHECK_NULL(legacyReader.ioHandler());
    CHECK_BOOL(legacyReader.canLoadFile(filePath), true);
    CHECK_DOUBLE(legacyReader.canLoadFileConfidence(filePath), 0.5 + 0.01 * QString(".tst").size());
    CHECK_BOOL(legacyReader.load(qSlicerIO::IOProperties()), false);
    CHECK_BOOL(legacyReader.loadedNodes().isEmpty(), true);
  }

  // VTK-based reader: all calls are forwarded
  {
    vtkNew<vtkMRMLFileReader> reader;
    reader->SetDescription("VTK test file");
    reader->SetFileType("VTKTestFile");
    reader->SetNameFilters({ "VTK test file (*.tst)" });
    qSlicerVTKFileReader vtkReader(reader);
    vtkReader.setMRMLScene(scene);
    CHECK_POINTER(vtkReader.ioHandler(), reader.GetPointer());
    CHECK_POINTER(vtkReader.fileReader(), reader.GetPointer());
    CHECK_POINTER(reader->GetScene(), scene.GetPointer());
    CHECK_BOOL(vtkReader.description() == "VTK test file", true);
    CHECK_BOOL(vtkReader.fileType() == "VTKTestFile", true);
    CHECK_BOOL(vtkReader.extensions() == QStringList() << "VTK test file (*.tst)", true);
    CHECK_BOOL(vtkReader.canLoadFile(filePath), true);
    CHECK_BOOL(vtkReader.canLoadFile(filePath + ".other"), false);
    CHECK_DOUBLE(vtkReader.canLoadFileConfidence(filePath), reader->CanLoadFileConfidence(filePath.toStdString()));
    CHECK_POINTER(vtkReader.userMessages(), reader->GetUserMessages());
  }

  // VTK-based writer: the confidence of the VTK-based writer is used
  {
    vtkNew<vtkSpecificConfidenceWriter> writer;
    writer->SetNodeClassNames({ "vtkMRMLTextNode" });
    qSlicerVTKFileWriter vtkWriter(writer);
    CHECK_POINTER(vtkWriter.ioHandler(), writer.GetPointer());
    CHECK_BOOL(vtkWriter.canWriteObject(textNode), true);
    CHECK_DOUBLE(vtkWriter.canWriteObjectConfidence(textNode), 0.7);

    // A subclass overrides canWriteObject(): confidence is computed from canWriteObject()
    qSlicerRestrictedVTKFileWriter restrictedWriter(writer);
    CHECK_BOOL(restrictedWriter.canWriteObject(textNode), false);
    CHECK_DOUBLE(restrictedWriter.canWriteObjectConfidence(textNode), 0.0);
    CHECK_BOOL(restrictedWriter.canWriteObject(allowedTextNode), true);
    CHECK_DOUBLE(restrictedWriter.canWriteObjectConfidence(allowedTextNode), writer->GetConfidenceForMatchingClass());
  }

  // Node writer: the confidence of the VTK-based writer is used
  {
    qSlicerNodeWriter nodeWriter("Text", QString("TextFile"), QStringList() << "vtkMRMLTextNode", false, nullptr);
    CHECK_NOT_NULL(nodeWriter.nodeWriter());
    CHECK_POINTER(nodeWriter.ioHandler(), nodeWriter.nodeWriter());
    CHECK_BOOL(nodeWriter.fileType() == "TextFile", true);
    CHECK_DOUBLE(nodeWriter.canWriteObjectConfidence(textNode), nodeWriter.nodeWriter()->CanWriteObjectConfidence(textNode));
    CHECK_BOOL(nodeWriter.canWriteObjectConfidence(textNode) > 0.0, true);

    // A node writer subclass overrides canWriteObject(): confidence is computed from canWriteObject()
    qSlicerNeverWritingNodeWriter neverWritingNodeWriter;
    CHECK_BOOL(neverWritingNodeWriter.nodeWriter()->CanWriteObjectConfidence(textNode) > 0.0, true);
    CHECK_DOUBLE(neverWritingNodeWriter.canWriteObjectConfidence(textNode), 0.0);

    // Writing uses the same check as canWriteObjectConfidence()
    // (a storage node is only created for short text if it is forced)
    vtkMRMLTextNode::SafeDownCast(textNode)->SetForceCreateStorageNode(vtkMRMLTextNode::CreateStorageNodeAlways);
    const QString textFilePath = QDir(tempDir.path()).filePath("text.txt");
    qSlicerIO::IOProperties properties;
    properties["nodeID"] = QString(textNode->GetID());
    properties["fileName"] = textFilePath;
    neverWritingNodeWriter.setMRMLScene(scene);
    CHECK_BOOL(neverWritingNodeWriter.write(properties), false);
    CHECK_BOOL(neverWritingNodeWriter.writtenNodes().isEmpty(), true);
    CHECK_BOOL(QFile::exists(textFilePath), false);

    // A node writer subclass that writes more node types than the VTK-based writer
    qSlicerExtendedNodeWriter extendedNodeWriter;
    extendedNodeWriter.setMRMLScene(scene);
    CHECK_BOOL(extendedNodeWriter.nodeWriter()->CanWriteObjectConfidence(textNode) > 0.0, false);
    CHECK_BOOL(extendedNodeWriter.canWriteObjectConfidence(textNode) > 0.0, true);
    CHECK_BOOL(extendedNodeWriter.write(properties), true);
    CHECK_BOOL(extendedNodeWriter.writtenNodes() == QStringList() << QString(textNode->GetID()), true);
    CHECK_BOOL(QFile::exists(textFilePath), true);

    // Plain node writer
    const QString textFilePath2 = QDir(tempDir.path()).filePath("text2.txt");
    properties["fileName"] = textFilePath2;
    nodeWriter.setMRMLScene(scene);
    CHECK_BOOL(nodeWriter.write(properties), true);
    CHECK_BOOL(nodeWriter.writtenNodes() == QStringList() << QString(textNode->GetID()), true);
    CHECK_BOOL(QFile::exists(textFilePath2), true);

    // Node writer that uses a vtkMRMLNodeWriter subclass: its Write() method is used
    const QString textFilePath3 = QDir(tempDir.path()).filePath("text3.txt");
    properties["fileName"] = textFilePath3;
    qSlicerCountingNodeWriter countingNodeWriter;
    countingNodeWriter.setMRMLScene(scene);
    CHECK_BOOL(countingNodeWriter.write(properties), true);
    CHECK_INT(countingNodeWriter.CountingWriter->WriteCount, 1);
    CHECK_BOOL(countingNodeWriter.writtenNodes() == QStringList() << QString(textNode->GetID()), true);
    CHECK_BOOL(QFile::exists(textFilePath3), true);
  }

  return EXIT_SUCCESS;
}
