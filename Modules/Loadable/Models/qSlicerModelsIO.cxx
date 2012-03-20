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
#include <QFileInfo>

// SlicerQt includes
#include "qSlicerModelsIO.h"

// Logic includes
#include "vtkSlicerModelsLogic.h"

// MRML includes
#include "vtkMRMLModelNode.h"

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerModelsIOPrivate
{
public:
  vtkSmartPointer<vtkSlicerModelsLogic> ModelsLogic;
};

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Models
qSlicerModelsIO::qSlicerModelsIO(vtkSlicerModelsLogic* _modelsLogic, QObject* _parent)
  : qSlicerIO(_parent)
  , d_ptr(new qSlicerModelsIOPrivate)
{
  this->setModelsLogic(_modelsLogic);
}

//-----------------------------------------------------------------------------
qSlicerModelsIO::~qSlicerModelsIO()
{
}

//-----------------------------------------------------------------------------
void qSlicerModelsIO::setModelsLogic(vtkSlicerModelsLogic* newModelsLogic)
{
  Q_D(qSlicerModelsIO);
  d->ModelsLogic = newModelsLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerModelsLogic* qSlicerModelsIO::modelsLogic()const
{
  Q_D(const qSlicerModelsIO);
  return d->ModelsLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerModelsIO::description()const
{
  return "Model";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerModelsIO::fileType()const
{
  return qSlicerIO::ModelFile;
}

//-----------------------------------------------------------------------------
QStringList qSlicerModelsIO::extensions()const
{
  return QStringList()
    << "Model (*.vtk *.vtp *.g *.byu *.stl *.orig"
         "*.inflated *.sphere *.white *.smoothwm *.pial *.obj)";
}

//-----------------------------------------------------------------------------
bool qSlicerModelsIO::load(const IOProperties& properties)
{
  Q_D(qSlicerModelsIO);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  this->setLoadedNodes(QStringList());
  if (d->ModelsLogic.GetPointer() == 0)
    {
    return false;
    }
  vtkMRMLModelNode* node = d->ModelsLogic->AddModel(
    fileName.toLatin1());
  if (!node)
    {
    return false;
    }
  this->setLoadedNodes( QStringList(QString(node->GetID())) );
  if (properties.contains("name"))
    {
    std::string uname = this->mrmlScene()->GetUniqueNameByString(
      properties["name"].toString().toLatin1());
    node->SetName(uname.c_str());
    }
  return true;
}
