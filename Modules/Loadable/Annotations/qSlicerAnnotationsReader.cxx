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

// Slicer includes
#include "qSlicerAnnotationsIOOptionsWidget.h"
#include "qSlicerAnnotationsReader.h"

// Logic includes
#include <vtkSlicerApplicationLogic.h>
#include "vtkSlicerAnnotationModuleLogic.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerAnnotationsReaderPrivate
{
  public:
  vtkSmartPointer<vtkSlicerAnnotationModuleLogic> AnnotationLogic;
};


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Annotations
//-----------------------------------------------------------------------------
qSlicerAnnotationsReader::qSlicerAnnotationsReader(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerAnnotationsReaderPrivate)
{
}

qSlicerAnnotationsReader::qSlicerAnnotationsReader(vtkSlicerAnnotationModuleLogic* logic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerAnnotationsReaderPrivate)
{
  this->setAnnotationLogic(logic);
}

//-----------------------------------------------------------------------------
qSlicerAnnotationsReader::~qSlicerAnnotationsReader() = default;

//-----------------------------------------------------------------------------
void qSlicerAnnotationsReader::setAnnotationLogic(vtkSlicerAnnotationModuleLogic* logic)
{
  Q_D(qSlicerAnnotationsReader);
  d->AnnotationLogic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogic* qSlicerAnnotationsReader::annotationLogic()const
{
  Q_D(const qSlicerAnnotationsReader);
  return d->AnnotationLogic.GetPointer();
}

//-----------------------------------------------------------------------------
QString qSlicerAnnotationsReader::description()const
{
  return "Annotation";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerAnnotationsReader::fileType()const
{
  return QString("AnnotationFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerAnnotationsReader::extensions()const
{
  return QStringList()
    << "Annotations (*.acsv)";
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerAnnotationsReader::options()const
{
  return new qSlicerAnnotationsIOOptionsWidget;
}

//-----------------------------------------------------------------------------
bool qSlicerAnnotationsReader::load(const IOProperties& properties)
{
  Q_D(qSlicerAnnotationsReader);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  QString name = QFileInfo(fileName).baseName();
  if (properties.contains("name"))
    {
    name = properties["name"].toString();
    }

  if (d->AnnotationLogic.GetPointer() == nullptr)
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

  char * nodeID = d->AnnotationLogic->LoadAnnotation(
    fileName.toUtf8(), name.toUtf8(), fileType);
  if (!nodeID)
    {
    this->setLoadedNodes(QStringList());
    return false;
    }
  this->setLoadedNodes( QStringList(QString(nodeID)) );
  if (properties.contains("name"))
    {
    std::string uname = this->mrmlScene()->GetUniqueNameByString(
      properties["name"].toString().toUtf8());
    this->mrmlScene()->GetNodeByID(nodeID)->SetName(uname.c_str());
    }
  return true;
}
