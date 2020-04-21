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

// Slicer includes
#include "qSlicerScalarOverlayIOOptionsWidget.h"
#include "qSlicerScalarOverlayReader.h"

// Logic includes
#include "vtkSlicerModelsLogic.h"

// MRML includes
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLStorageNode.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerScalarOverlayReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerModelsLogic> ModelsLogic;
};

//-----------------------------------------------------------------------------
qSlicerScalarOverlayReader::qSlicerScalarOverlayReader(
  vtkSlicerModelsLogic* _modelsLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerScalarOverlayReaderPrivate)
{
  this->setModelsLogic(_modelsLogic);
}

//-----------------------------------------------------------------------------
qSlicerScalarOverlayReader::~qSlicerScalarOverlayReader() = default;

//-----------------------------------------------------------------------------
void qSlicerScalarOverlayReader::setModelsLogic(vtkSlicerModelsLogic* newModelsLogic)
{
  Q_D(qSlicerScalarOverlayReader);
  d->ModelsLogic = newModelsLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerModelsLogic* qSlicerScalarOverlayReader::modelsLogic()const
{
  Q_D(const qSlicerScalarOverlayReader);
  return d->ModelsLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerScalarOverlayReader::description()const
{
  return "Scalar Overlay";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerScalarOverlayReader::fileType()const
{
  return QString("ScalarOverlayFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerScalarOverlayReader::extensions()const
{
  return QStringList() << "*.*";
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerScalarOverlayReader::options()const
{
  qSlicerIOOptionsWidget* options = new qSlicerScalarOverlayIOOptionsWidget;
  options->setMRMLScene(this->mrmlScene());
  return options;
}

//-----------------------------------------------------------------------------
bool qSlicerScalarOverlayReader::load(const IOProperties& properties)
{
  Q_D(qSlicerScalarOverlayReader);
  Q_ASSERT(properties.contains("fileName"));
  if (!properties.contains("modelNodeId"))
    {
    qCritical() << Q_FUNC_INFO << " failed: missing fileName or modelNodeId property";
    return false;
    }
  QString fileName = properties["fileName"].toString();

  QString modelNodeId = properties["modelNodeId"].toString();
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(
      this->mrmlScene()->GetNodeByID(modelNodeId.toUtf8()));
  if (modelNode == nullptr)
    {
    qCritical() << Q_FUNC_INFO << " failed: modelNodeId refers to invalid/non-existent node";
    return false;
    }

  if (d->ModelsLogic == nullptr)
    {
    qCritical() << Q_FUNC_INFO << " failed: invalid models module logic";
    return false;
    }

  QStringList loadedNodes;
  bool success = d->ModelsLogic->AddScalar(fileName.toUtf8(), modelNode);
  if (success)
    {
    loadedNodes << QString(modelNodeId);
    }
  this->setLoadedNodes(loadedNodes);
  return success;
}
