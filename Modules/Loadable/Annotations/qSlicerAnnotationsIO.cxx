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
#include "qSlicerAnnotationsIO.h"
#include "qSlicerAnnotationsIOOptionsWidget.h"

// Logic includes
#include <vtkSlicerApplicationLogic.h>
#include "vtkSlicerAnnotationModuleLogic.h"

// MRML includes
#include <vtkMRMLNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerAnnotationsIOPrivate
{
  public:
  vtkSmartPointer<vtkSlicerAnnotationModuleLogic> AnnotationLogic;
};


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Annotations
//-----------------------------------------------------------------------------
qSlicerAnnotationsIO::qSlicerAnnotationsIO(QObject* _parent)
  : qSlicerIO(_parent)
  , d_ptr(new qSlicerAnnotationsIOPrivate)
{
}

qSlicerAnnotationsIO::qSlicerAnnotationsIO(vtkSlicerAnnotationModuleLogic* logic, QObject* _parent)
  :qSlicerIO(_parent)
  , d_ptr(new qSlicerAnnotationsIOPrivate)
{
  this->setAnnotationLogic(logic);
}

//-----------------------------------------------------------------------------
qSlicerAnnotationsIO::~qSlicerAnnotationsIO()
{
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationsIO::setAnnotationLogic(vtkSlicerAnnotationModuleLogic* logic)
{
  Q_D(qSlicerAnnotationsIO);
  d->AnnotationLogic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogic* qSlicerAnnotationsIO::annotationLogic()const
{
  Q_D(const qSlicerAnnotationsIO);
  return d->AnnotationLogic.GetPointer();
}

//-----------------------------------------------------------------------------
QString qSlicerAnnotationsIO::description()const
{
  return "Annotation";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerAnnotationsIO::fileType()const
{
  return qSlicerIO::AnnotationFile;
}

//-----------------------------------------------------------------------------
QStringList qSlicerAnnotationsIO::extensions()const
{
  return QStringList()
    << "Annotations (*.acsv)";
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerAnnotationsIO::options()const
{
  return new qSlicerAnnotationsIOOptionsWidget;
}

//-----------------------------------------------------------------------------
bool qSlicerAnnotationsIO::load(const IOProperties& properties)
{
  Q_D(qSlicerAnnotationsIO);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  QString name = QFileInfo(fileName).baseName();
  if (properties.contains("name"))
    {
    name = properties["name"].toString();
    }

  if (d->AnnotationLogic.GetPointer() == 0)
    {
    return false;
    }
  
  // file type
  int fileType = vtkSlicerAnnotationModuleLogic::None;
  if (properties.contains("fiducial") && properties["fiducial"].toBool() == true)
    {
    fileType = vtkSlicerAnnotationModuleLogic::Fiducial;
    }
  else if (properties.contains("ruler") && properties["ruler"].toBool() == true)
    {
    fileType = vtkSlicerAnnotationModuleLogic::Ruler;
    }
  else if (properties.contains("roi") && properties["roi"].toBool() == true)
    {
    fileType = vtkSlicerAnnotationModuleLogic::ROI;
    }
//  else if (properties.contains("list") && properties["list"].toBool() == true)
//    {
//    fileType = vtkSlicerAnnotationModuleLogic::List;
//    }

  char * nodeID = d->AnnotationLogic->LoadAnnotation(
    fileName.toLatin1(), name.toLatin1(), fileType);
  if (!nodeID)
    {
    this->setLoadedNodes(QStringList());
    return false;
    }
  this->setLoadedNodes( QStringList(QString(nodeID)) );
  if (properties.contains("name"))
    {
    std::string uname = this->mrmlScene()->GetUniqueNameByString(
      properties["name"].toString().toLatin1());
    this->mrmlScene()->GetNodeByID(nodeID)->SetName(uname.c_str());
    }
  return true;
}
