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
class qSlicerFiducialsIOPrivate
{
  public:
  vtkSmartPointer<vtkSlicerAnnotationModuleLogic> AnnotationLogic;
};

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Annotations
//-----------------------------------------------------------------------------
qSlicerFiducialsIO::qSlicerFiducialsIO(QObject* _parent)
  :qSlicerIO(_parent)
  , d_ptr(new qSlicerFiducialsIOPrivate)
{
}

qSlicerFiducialsIO::qSlicerFiducialsIO(vtkSlicerAnnotationModuleLogic* logic, QObject* _parent)
  :qSlicerIO(_parent)
  , d_ptr(new qSlicerFiducialsIOPrivate)
{
  this->setAnnotationLogic(logic);
}

//-----------------------------------------------------------------------------
qSlicerFiducialsIO::~qSlicerFiducialsIO()
{
}

//-----------------------------------------------------------------------------
void qSlicerFiducialsIO::setAnnotationLogic(vtkSlicerAnnotationModuleLogic* logic)
{
  Q_D(qSlicerFiducialsIO);
  d->AnnotationLogic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogic* qSlicerFiducialsIO::annotationLogic()const
{
  Q_D(const qSlicerFiducialsIO);
  return d->AnnotationLogic.GetPointer();
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
  Q_D(qSlicerFiducialsIO);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  if (d->AnnotationLogic.GetPointer() == 0)
    {
    return false;
    }

  char * nodeIDs = d->AnnotationLogic->LoadFiducialList(fileName.toLatin1());

  if (nodeIDs)
    {
    // returned a comma separated list of ids of the nodes that were loaded
    QStringList nodeIDList;
    char *ptr = strtok(nodeIDs, ",");

    while (ptr)
      {
      nodeIDList.append(ptr);
      ptr = strtok(NULL, ",");
      }
    this->setLoadedNodes(nodeIDList);
    }
  else
    {
    this->setLoadedNodes(QStringList());
    return false;
    }

  return nodeIDs != 0;
}

// TODO: add the save() method. Use vtkSlicerTransformLogic::SaveTransform()
