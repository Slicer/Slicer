/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QString>
#include <QVariant>

// SlicerQt includes
//#include "qSlicerAbstractModule.h"
//#include "qSlicerCoreApplication.h"
//#include "qSlicerModuleManager.h"
#include "qSlicerModelsIO.h"

// Logic includes
#include "vtkSlicerModelsLogic.h"

// MRML includes
#include <vtkMRMLModelNode.h>
#include <vtkMRMLStorageNode.h>

//-----------------------------------------------------------------------------
qSlicerModelsIO::qSlicerModelsIO(QObject* _parent)
  :qSlicerIO(_parent)
{
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
         "*.inflated *.sphere *.white *.smoothwm *.pial)";
}

//-----------------------------------------------------------------------------
bool qSlicerModelsIO::load(const IOProperties& properties)
{
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  // FIXME: Use the following commented lines when qSlicerModelsIO.[x/cxx] will
  // be in the same directory than the Models module. Create its own logic for
  // now.
  // vtkSlicerModelsLogic* modelsLogic =
  //   vtkSlicerModelsLogic::SafeDownCast(
  //     qSlicerCoreApplication::application()->moduleManager()
  //     ->module("Models")->logic());
  vtkSlicerModelsLogic* modelsLogic = vtkSlicerModelsLogic::New();
  modelsLogic->SetMRMLScene(this->mrmlScene());
  Q_ASSERT(modelsLogic);
  vtkMRMLModelNode* node = modelsLogic->AddModel(
    fileName.toLatin1().data());
  if (node)
    {
    this->setLoadedNodes(QStringList(QString(node->GetID())));
    }
  else
    {
    this->setLoadedNodes(QStringList());
    }
  modelsLogic->Delete();
  return node != 0;
}
