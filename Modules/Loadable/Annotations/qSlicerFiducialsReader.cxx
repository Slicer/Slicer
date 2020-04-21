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

// Slicer includes
#include "qSlicerFiducialsReader.h"

// Logic includes
//#include "vtkSlicerFiducialsLogic.h"

#include <vtkSlicerApplicationLogic.h>
#include "vtkSlicerAnnotationModuleLogic.h"

// MRML includes
#include <vtkMRMLFiducialListNode.h>
#include <vtkMRMLNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerFiducialsReaderPrivate
{
  public:
  vtkSmartPointer<vtkSlicerAnnotationModuleLogic> AnnotationLogic;
};

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Annotations
//-----------------------------------------------------------------------------
qSlicerFiducialsReader::qSlicerFiducialsReader(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerFiducialsReaderPrivate)
{
}

qSlicerFiducialsReader::qSlicerFiducialsReader(vtkSlicerAnnotationModuleLogic* logic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerFiducialsReaderPrivate)
{
  this->setAnnotationLogic(logic);
}

//-----------------------------------------------------------------------------
qSlicerFiducialsReader::~qSlicerFiducialsReader() = default;

//-----------------------------------------------------------------------------
void qSlicerFiducialsReader::setAnnotationLogic(vtkSlicerAnnotationModuleLogic* logic)
{
  Q_D(qSlicerFiducialsReader);
  d->AnnotationLogic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogic* qSlicerFiducialsReader::annotationLogic()const
{
  Q_D(const qSlicerFiducialsReader);
  return d->AnnotationLogic.GetPointer();
}

//-----------------------------------------------------------------------------
QString qSlicerFiducialsReader::description()const
{
  return "Fiducials";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerFiducialsReader::fileType()const
{
  return QString("FiducialListFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerFiducialsReader::extensions()const
{
  return QStringList() << "Fiducials (*.fcsv)";
}

//-----------------------------------------------------------------------------
bool qSlicerFiducialsReader::load(const IOProperties& properties)
{
  Q_D(qSlicerFiducialsReader);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  if (d->AnnotationLogic.GetPointer() == nullptr)
    {
    return false;
    }

  char * nodeIDs = d->AnnotationLogic->LoadFiducialList(fileName.toUtf8());

  if (nodeIDs)
    {
    // returned a comma separated list of ids of the nodes that were loaded
    QStringList nodeIDList;
    char *ptr = strtok(nodeIDs, ",");

    while (ptr)
      {
      nodeIDList.append(ptr);
      ptr = strtok(nullptr, ",");
      }
    this->setLoadedNodes(nodeIDList);
    }
  else
    {
    this->setLoadedNodes(QStringList());
    return false;
    }

  return nodeIDs != nullptr;
}

// TODO: add the save() method. Use vtkSlicerTransformLogic::SaveTransform()
