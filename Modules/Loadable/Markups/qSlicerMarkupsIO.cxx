/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QFileInfo>

// SlicerQt includes
#include "qSlicerMarkupsIO.h"

// Logic includes
#include <vtkSlicerApplicationLogic.h>
#include "vtkSlicerMarkupsLogic.h"

// MRML includes

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerMarkupsIOPrivate
{
  public:
  vtkSmartPointer<vtkSlicerMarkupsLogic> MarkupsLogic;
};

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Annotations
//-----------------------------------------------------------------------------
qSlicerMarkupsIO::qSlicerMarkupsIO(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerMarkupsIOPrivate)
{
}

qSlicerMarkupsIO::qSlicerMarkupsIO(vtkSlicerMarkupsLogic* logic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerMarkupsIOPrivate)
{
  this->setMarkupsLogic(logic);
}

//-----------------------------------------------------------------------------
qSlicerMarkupsIO::~qSlicerMarkupsIO()
{
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsIO::setMarkupsLogic(vtkSlicerMarkupsLogic* logic)
{
  Q_D(qSlicerMarkupsIO);
  d->MarkupsLogic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerMarkupsLogic* qSlicerMarkupsIO::markupsLogic()const
{
  Q_D(const qSlicerMarkupsIO);
  return d->MarkupsLogic.GetPointer();
}

//-----------------------------------------------------------------------------
QString qSlicerMarkupsIO::description()const
{
  return "MarkupsFiducials";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerMarkupsIO::fileType()const
{
  return QString("MarkupsFiducials");
}

//-----------------------------------------------------------------------------
QStringList qSlicerMarkupsIO::extensions()const
{
  return QStringList()
    << "Markups Fiducials (*.fcsv)"
    << " Annotation Fiducial (*.acsv)";
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsIO::load(const IOProperties& properties)
{
  Q_D(qSlicerMarkupsIO);

  // get the properties
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  QString name = QFileInfo(fileName).baseName();
  if (properties.contains("name"))
    {
    name = properties["name"].toString();
    }

  if (d->MarkupsLogic.GetPointer() == 0)
    {
    return false;
    }

  // pass to logic to do the loading
  char * nodeIDs = d->MarkupsLogic->LoadMarkupsFiducials(fileName.toLatin1(), name.toLatin1());

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
