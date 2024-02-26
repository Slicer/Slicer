/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Adam Rankin, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Qt includes
#include <QTextStream>

// Terminologies includes
#include "qSlicerTerminologiesReader.h"

// Logic includes
#include "vtkSlicerTerminologiesModuleLogic.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerTerminologiesReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerTerminologiesModuleLogic> TerminologiesLogic;
};

//-----------------------------------------------------------------------------
qSlicerTerminologiesReader::qSlicerTerminologiesReader(vtkSlicerTerminologiesModuleLogic* terminologiesLogic,
                                                       QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTerminologiesReaderPrivate)
{
  this->setTerminologiesLogic(terminologiesLogic);
}

//-----------------------------------------------------------------------------
qSlicerTerminologiesReader::~qSlicerTerminologiesReader() = default;

//-----------------------------------------------------------------------------
void qSlicerTerminologiesReader::setTerminologiesLogic(vtkSlicerTerminologiesModuleLogic* newTerminologiesLogic)
{
  Q_D(qSlicerTerminologiesReader);
  d->TerminologiesLogic = newTerminologiesLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic* qSlicerTerminologiesReader::terminologiesLogic() const
{
  Q_D(const qSlicerTerminologiesReader);
  return d->TerminologiesLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerTerminologiesReader::description() const
{
  return "Terminology";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerTerminologiesReader::fileType() const
{
  return QString("TerminologyFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerTerminologiesReader::extensions() const
{
  return QStringList() << "Terminology (*.term.json)"
                       << "Terminology (*.json)";
}

//----------------------------------------------------------------------------
double qSlicerTerminologiesReader::canLoadFileConfidence(const QString& fileName) const
{
  double confidence = Superclass::canLoadFileConfidence(fileName);

  // Confidence for .json file is 0.55 (5 characters in the file extension matched),
  // for composite file extensions (.term.json) it would be 0.6.
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
        // Markups json files contain a schema URL like /anatomic-context-schema.json
        // or /segment-context-schema.json around position 200, read a bit further
        // to account for slight variations in the header.
        QString line = in.read(400);
        bool looksLikeTerminology =
          line.contains("/anatomic-context-schema.json") || line.contains("/segment-context-schema.json");
        confidence = (looksLikeTerminology ? 0.6 : 0.4);
      }
    }
  }
  return confidence;
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologiesReader::load(const IOProperties& properties)
{
  Q_D(qSlicerTerminologiesReader);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  this->setLoadedNodes(QStringList());
  if (d->TerminologiesLogic.GetPointer() == nullptr)
  {
    return false;
  }

  bool contextLoaded = d->TerminologiesLogic->LoadContextFromFile(fileName.toUtf8().constData());
  if (!contextLoaded)
  {
    this->setLoadedNodes(QStringList());
    return false;
  }

  return true;
}
