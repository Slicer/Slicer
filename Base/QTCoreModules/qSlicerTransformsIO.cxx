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

// SlicerQt includes
#include "qSlicerAbstractModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerTransformsIO.h"

// Logic includes
#include "vtkSlicerTransformLogic.h"

// MRML includes
#include <vtkMRMLTransformNode.h>

//-----------------------------------------------------------------------------
qSlicerTransformsIO::qSlicerTransformsIO(QObject* _parent)
  :qSlicerIO(_parent)
{
}

//-----------------------------------------------------------------------------
QString qSlicerTransformsIO::description()const
{
  return "Transform";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerTransformsIO::fileType()const
{
  return qSlicerIO::TransformFile;
}

//-----------------------------------------------------------------------------
QStringList qSlicerTransformsIO::extensions()const
{
  return QStringList() << "Transform (*.tfm *.mat *.txt)";
}

//-----------------------------------------------------------------------------
bool qSlicerTransformsIO::load(const IOProperties& properties)
{
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  vtkSlicerTransformLogic* transformLogic =
    vtkSlicerTransformLogic::SafeDownCast(
      qSlicerCoreApplication::application()->moduleManager()
      ->module("Transforms")->logic());
  Q_ASSERT(transformLogic && transformLogic->GetMRMLScene() == this->mrmlScene());
  //transformLogic->SetMRMLScene(this->mrmlScene());
  vtkMRMLTransformNode* node = transformLogic->AddTransform(
    fileName.toLatin1().data(), this->mrmlScene());
  if (node)
    {
    this->setLoadedNodes(QStringList(QString(node->GetID())));
    }
  else
    {
    this->setLoadedNodes(QStringList());
    }
  return node != 0;
}

// TODO: add the save() method. Use vtkSlicerTransformLogic::SaveTransform()
