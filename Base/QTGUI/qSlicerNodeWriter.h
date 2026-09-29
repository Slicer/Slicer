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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerNodeWriter_h
#define __qSlicerNodeWriter_h

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"
#include "qSlicerNodeWriterOptionsWidget.h"
#include "qSlicerVTKFileWriter.h"

// Slicer includes
#include <vtkMRMLIOProperties.h>
#include <vtkMRMLNodeWriter.h>
#include <vtkMRMLStorableNode.h>

// VTK includes
#include <vtkNew.h>

class vtkMRMLNode;

/// Utility class that is ready to use for most of the nodes.
///
/// \deprecated All tasks are delegated to vtkMRMLNodeWriter.
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerNodeWriter : public qSlicerVTKFileWriter
{
  Q_OBJECT
  /// Some storage nodes don't support the compression option
  Q_PROPERTY(bool supportUseCompression READ supportUseCompression WRITE setSupportUseCompression);

public:
  typedef qSlicerVTKFileWriter Superclass;
  qSlicerNodeWriter(const QString& description, const qSlicerIO::IOFileType& fileType, const QStringList& nodeClassNames, bool supportUseCompression, QObject* parent)
    : Superclass(nullptr, parent)
  {
    vtkNew<vtkMRMLNodeWriter> writer;
    writer->SetDescription(description.toStdString());
    writer->SetFileType(fileType.toStdString());
    writer->SetSupportUseCompression(supportUseCompression);
    // The writer is set in this constructor so that subclasses that may override methods are detected
    this->setFileWriter(writer);
    this->setNodeClassNames(nodeClassNames);
  }

  /// VTK-based writer that performs all tasks
  vtkMRMLNodeWriter* nodeWriter() const { return vtkMRMLNodeWriter::SafeDownCast(this->fileWriter()); }

  void setSupportUseCompression(bool support) { this->nodeWriter()->SetSupportUseCompression(support); }
  bool supportUseCompression() const { return this->nodeWriter()->GetSupportUseCompression(); }

  virtual vtkMRMLNode* getNodeByID(const char* id) const { return id ? this->nodeWriter()->GetNodeByID(id) : nullptr; }

  /// Write the node identified by "nodeID" property into the "fileName" file.
  /// The node is found using getNodeByID() and checked using canWriteObjectConfidence(), which may be
  /// overridden in subclasses, then the VTK-based writer writes the node.
  bool write(const qSlicerIO::IOProperties& properties) override
  {
    vtkMRMLNodeWriter* writer = this->nodeWriter();
    if (!writer)
    {
      return false;
    }
    writer->ClearWrittenNodeIDs();
    writer->SetScene(this->mrmlScene());
    vtkMRMLStorableNode* node = vtkMRMLStorableNode::SafeDownCast(this->getNodeByID(properties.value("nodeID").toString().toUtf8().constData()));
    if (!node || this->canWriteObjectConfidence(node) <= 0.0)
    {
      return false;
    }
    vtkNew<vtkMRMLIOProperties> vtkProperties;
    qSlicerIO::toVTKProperties(properties, vtkProperties);
    if (node->GetID() && writer->GetNodeByID(node->GetID()) == node && writer->CanWriteObjectConfidence(node) > 0.0)
    {
      // The VTK-based writer can write the node: use Write(), which may be overridden in vtkMRMLNodeWriter subclasses
      return writer->Write(vtkProperties);
    }
    // This writer accepts a node that the VTK-based writer would not (getNodeByID() or canWriteObject() is overridden)
    return writer->WriteNode(node, vtkProperties);
  }

  /// Options widget (for writers that call this method from their options() implementation).
  qSlicerIOOptions* options() const override
  {
    qSlicerNodeWriterOptionsWidget* options = new qSlicerNodeWriterOptionsWidget;
    options->setShowUseCompression(this->supportUseCompression());
    return options;
  }

  /// Constructor for subclasses. A default vtkMRMLNodeWriter is created,
  /// which may be replaced by a vtkMRMLNodeWriter subclass by calling setFileWriter.
  explicit qSlicerNodeWriter(QObject* parent)
    : Superclass(nullptr, parent)
  {
    vtkNew<vtkMRMLNodeWriter> writer;
    this->setFileWriter(writer);
  }

protected:
  void setNodeClassNames(const QStringList& nodeClassNames)
  {
    std::vector<std::string> classNames;
    for (const QString& className : nodeClassNames)
    {
      classNames.push_back(className.toStdString());
    }
    this->nodeWriter()->SetNodeClassNames(classNames);
  }
  QStringList nodeClassNames() const
  {
    QStringList classNames;
    for (const std::string& className : this->nodeWriter()->GetNodeClassNames())
    {
      classNames << QString::fromStdString(className);
    }
    return classNames;
  }
};

#endif
