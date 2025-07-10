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

// Qt includes
#include <QDebug>
#include <QFileInfo>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerModelsIOOptionsWidget.h"
#include "qSlicerModelsReader.h"

// Logic includes
#include "vtkSlicerModelsLogic.h"

// MRML includes
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLModelNode.h"
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerModelsReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerModelsLogic> ModelsLogic;
};

//-----------------------------------------------------------------------------
qSlicerModelsReader::qSlicerModelsReader(vtkSlicerModelsLogic* _modelsLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerModelsReaderPrivate)
{
  this->setModelsLogic(_modelsLogic);
}

//-----------------------------------------------------------------------------
qSlicerModelsReader::~qSlicerModelsReader() = default;

//-----------------------------------------------------------------------------
void qSlicerModelsReader::setModelsLogic(vtkSlicerModelsLogic* newModelsLogic)
{
  Q_D(qSlicerModelsReader);
  d->ModelsLogic = newModelsLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerModelsLogic* qSlicerModelsReader::modelsLogic() const
{
  Q_D(const qSlicerModelsReader);
  return d->ModelsLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerModelsReader::description() const
{
  return "Model";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerModelsReader::fileType() const
{
  return QString("ModelFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerModelsReader::extensions() const
{
  return QStringList() << "Model (*.vtk *.vtp *.vtu *.g *.byu *.stl *.ply *.obj *.ucd)";
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerModelsReader::options() const
{
  qSlicerIOOptionsWidget* options = new qSlicerModelsIOOptionsWidget;
  options->setMRMLScene(this->mrmlScene());
  return options;
}

//----------------------------------------------------------------------------
double qSlicerModelsReader::canLoadFileConfidence(const QString& fileName) const
{
  double confidence = Superclass::canLoadFileConfidence(fileName);

  // .vtk files can store either an image (DATASET STRUCTURED_POINTS)
  // or a mesh (DATASET POLYDATA). Images are read by the volume reader with default
  // confidence of 0.54 (4 characters in the .vtk file extension matched).
  // Therefore, we set confidence here to 0.6 for meshes and 0.0 for images.
  if (confidence > 0)
  {
    // Not a composite file extension, inspect the content (for now, only nrrd).
    QString upperCaseFileName = fileName.toUpper();
    if (upperCaseFileName.endsWith(".VTK"))
    {
      QFile file(fileName);
      if (file.open(QIODevice::ReadOnly | QIODevice::Text))
      {
        QTextStream in(&file);
        // .vtk image file header contains DATASET STRUCTURED_POINTS at around
        // around position 100, read a bit further to account for slight variations in the header.
        QString line = in.read(200);
        // If dataset is structured points then it is an image, which this reader cannot read.
        confidence = (line.contains("STRUCTURED_POINTS") ? 0.0 : 0.6);
      }
    }
  }
  return confidence;
}

//-----------------------------------------------------------------------------
bool qSlicerModelsReader::load(const IOProperties& properties)
{
  Q_D(qSlicerModelsReader);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  this->setLoadedNodes(QStringList());
  if (!d->ModelsLogic)
  {
    qCritical() << Q_FUNC_INFO << (" failed: Models logic is invalid.");
    return false;
  }
  int coordinateSystem = vtkMRMLStorageNode::CoordinateSystemLPS; // default
  if (properties.contains("coordinateSystem"))
  {
    coordinateSystem = properties["coordinateSystem"].toInt();
  }
  this->userMessages()->ClearMessages();
  vtkMRMLModelNode* node = d->ModelsLogic->AddModel(fileName.toUtf8(), coordinateSystem, this->userMessages());
  if (!node)
  {
    // errors are already logged and userMessages contain details that can be displayed to users
    return false;
  }
  this->setLoadedNodes(QStringList(QString(node->GetID())));
  if (properties.contains("name"))
  {
    std::string uname = this->mrmlScene()->GetUniqueNameByString(properties["name"].toString().toUtf8());
    node->SetName(uname.c_str());
  }

  // If no other nodes are displayed then reset the field of view
  bool otherNodesAreAlreadyVisible = false;
  vtkSmartPointer<vtkCollection> displayNodes =
    vtkSmartPointer<vtkCollection>::Take(this->mrmlScene()->GetNodesByClass("vtkMRMLDisplayNode"));
  for (int displayNodeIndex = 0; displayNodeIndex < displayNodes->GetNumberOfItems(); ++displayNodeIndex)
  {
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(displayNodes->GetItemAsObject(displayNodeIndex));
    if (displayNode->GetDisplayableNode() && displayNode->GetVisibility() && displayNode->GetDisplayableNode() != node)
    {
      otherNodesAreAlreadyVisible = true;
      break;
    }
  }
  if (!otherNodesAreAlreadyVisible)
  {
    qSlicerApplication* app = qSlicerApplication::application();
    if (app && app->layoutManager())
    {
      app->layoutManager()->resetThreeDViews();
    }
  }

  return true;
}
