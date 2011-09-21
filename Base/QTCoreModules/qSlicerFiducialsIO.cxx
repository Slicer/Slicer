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
//#include "qSlicerAbstractModule.h"
//#include "qSlicerCoreApplication.h"
//#include "qSlicerModuleManager.h"
#include "qSlicerFiducialsIO.h"

// Logic includes
#include "vtkSlicerFiducialsLogic.h"

// MRML includes
#include <vtkMRMLFiducialListNode.h>

//-----------------------------------------------------------------------------
qSlicerFiducialsIO::qSlicerFiducialsIO(QObject* _parent)
  :qSlicerIO(_parent)
{
}

//-----------------------------------------------------------------------------
QString qSlicerFiducialsIO::description()const
{
  return "Fiducials";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerFiducialsIO::fileType()const
{
  return qSlicerIO::FiducialListFile;
}

//-----------------------------------------------------------------------------
QStringList qSlicerFiducialsIO::extensions()const
{
  return QStringList() << "Fiducials (*.fcsv)";
}

//-----------------------------------------------------------------------------
bool qSlicerFiducialsIO::load(const IOProperties& properties)
{
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  //vtkSlicerTransformLogic* transformLogic =
  //  vtkSlicerTransformLogic::SafeDownCast(
  //    qSlicerCoreApplication::application()->moduleManager()
  //    ->module("Fiducials")->logic());
  //Q_ASSERT(transformLogic && transformLogic->GetMRMLScene() == this->mrmlScene());
  vtkSlicerFiducialsLogic* fiducialsLogic = vtkSlicerFiducialsLogic::New();
  fiducialsLogic->SetMRMLScene(this->mrmlScene());
  vtkMRMLFiducialListNode* node = fiducialsLogic->LoadFiducialList(
    fileName.toLatin1().data());
  if (node)
    {
    this->setLoadedNodes(QStringList(QString(node->GetID())));
    }
  else
    {
    this->setLoadedNodes(QStringList());
    }
  fiducialsLogic->Delete();
  return node != 0;
}

// TODO: add the save() method. Use vtkSlicerTransformLogic::SaveTransform()
