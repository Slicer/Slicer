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
/// \ingroup Slicer_QtModules_Terminologies
qSlicerTerminologiesReader::qSlicerTerminologiesReader(vtkSlicerTerminologiesModuleLogic* terminologiesLogic, QObject* _parent)
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
vtkSlicerTerminologiesModuleLogic* qSlicerTerminologiesReader::terminologiesLogic()const
{
  Q_D(const qSlicerTerminologiesReader);
  return d->TerminologiesLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerTerminologiesReader::description()const
{
  return "Terminology";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerTerminologiesReader::fileType()const
{
  return QString("TerminologyFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerTerminologiesReader::extensions()const
{
  return QStringList() << "Terminology (*.term.json)" << "Terminology (*.json)";
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
