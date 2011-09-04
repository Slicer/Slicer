/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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
#include "qSlicerColorIO.h"

// Logic includes
#include "vtkSlicerColorLogic.h"

// MRML includes
#include <vtkMRMLColorNode.h>

//-----------------------------------------------------------------------------
qSlicerColorIO::qSlicerColorIO(QObject* _parent)
  :qSlicerIO(_parent)
{
}

//-----------------------------------------------------------------------------
QString qSlicerColorIO::description()const
{
  return "Color";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerColorIO::fileType()const
{
  return qSlicerIO::ColorTableFile;
}

//-----------------------------------------------------------------------------
QStringList qSlicerColorIO::extensions()const
{
  return QStringList() << "Color (*.txt *.ctbl)";
}

//-----------------------------------------------------------------------------
bool qSlicerColorIO::load(const IOProperties& properties)
{
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  // TODO: Set the logic to qSlicerColorIO directly
  vtkSlicerColorLogic* colorLogic =
    vtkSlicerColorLogic::SafeDownCast(
      qSlicerCoreApplication::application()->moduleManager()
        ->module("Colors")->logic());
  Q_ASSERT(colorLogic && colorLogic->GetMRMLScene() == this->mrmlScene());
  //vtkSlicerColorLogic* colorLogic = vtkSlicerColorLogic::New();
  //Q_ASSERT(colorLogic);
  //colorLogic->SetMRMLScene(this->mrmlScene());
  vtkMRMLColorNode* node = colorLogic->LoadColorFile(fileName.toLatin1());
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

// TODO: add the save() method. Use vtkSlicerColorLogic::SaveColor()
