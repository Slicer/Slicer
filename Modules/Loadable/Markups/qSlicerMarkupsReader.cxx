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
#include <QTextStream>

// Slicer includes
#include "qSlicerMarkupsReader.h"

// Logic includes
#include <vtkSlicerApplicationLogic.h>
#include "vtkSlicerMarkupsLogic.h"

// MRML includes
#include "vtkMRMLMessageCollection.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerMarkupsReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerMarkupsLogic> MarkupsLogic;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
qSlicerMarkupsReader::qSlicerMarkupsReader(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerMarkupsReaderPrivate)
{
}

qSlicerMarkupsReader::qSlicerMarkupsReader(vtkSlicerMarkupsLogic* logic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerMarkupsReaderPrivate)
{
  this->setMarkupsLogic(logic);
}

//-----------------------------------------------------------------------------
qSlicerMarkupsReader::~qSlicerMarkupsReader() = default;

//-----------------------------------------------------------------------------
void qSlicerMarkupsReader::setMarkupsLogic(vtkSlicerMarkupsLogic* logic)
{
  Q_D(qSlicerMarkupsReader);
  d->MarkupsLogic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerMarkupsLogic* qSlicerMarkupsReader::markupsLogic() const
{
  Q_D(const qSlicerMarkupsReader);
  return d->MarkupsLogic.GetPointer();
}

//-----------------------------------------------------------------------------
QString qSlicerMarkupsReader::description() const
{
  return tr("Markups");
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerMarkupsReader::fileType() const
{
  return QString("MarkupsFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerMarkupsReader::extensions() const
{
  return QStringList() << tr("Markups") + " (*.mrk.json)" << tr("Markups") + " (*.json)"
                       << tr("Markups Fiducials") + " (*.fcsv)";
}

//----------------------------------------------------------------------------
double qSlicerMarkupsReader::canLoadFileConfidence(const QString& fileName) const
{
  double confidence = Superclass::canLoadFileConfidence(fileName);

  // Confidence for .json file is 0.55 (5 characters in the file extension matched),
  // for composite file extensions (.mrk.json) it would be 0.59.
  // Therefore, confidence below 0.56 means that we got a generic file extension
  // that we need to inspect further.
  if (confidence > 0 && confidence < 0.56)
  {
    // Not a composite file extension, inspect the content
    QString upperCaseFileName = fileName.toUpper();
    if (upperCaseFileName.endsWith("JSON"))
    {
      QFile file(fileName);
      if (file.open(QIODevice::ReadOnly | QIODevice::Text))
      {
        QTextStream in(&file);
        // Markups json files contain a schema URL like .../Schema/markups-schema-v1.0.3.json
        // around position 150, read a bit further to account for slight variations in the header.
        QString line = in.read(300);
        confidence = (line.contains("/markups-schema-v1.") ? 0.6 : 0.4);
      }
    }
  }
  return confidence;
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsReader::load(const IOProperties& properties)
{
  Q_D(qSlicerMarkupsReader);

  // get the properties
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  QString name;
  if (properties.contains("name"))
  {
    name = properties["name"].toString();
  }

  if (d->MarkupsLogic.GetPointer() == nullptr)
  {
    return false;
  }

  // pass to logic to do the loading
  this->userMessages()->ClearMessages();
  char* nodeIDs = d->MarkupsLogic->LoadMarkups(fileName.toUtf8(), name.toUtf8(), this->userMessages());
  if (nodeIDs)
  {
    // returned a comma separated list of ids of the nodes that were loaded
    QStringList nodeIDList;
    char* ptr = strtok(nodeIDs, ",");

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
