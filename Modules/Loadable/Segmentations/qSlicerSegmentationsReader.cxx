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

// Segmentations includes
#include "qSlicerSegmentationsReader.h"

#include "vtkMRMLSegmentationNode.h"

// Qt includes
#include <QFileInfo>

// Logic includes
#include "vtkSlicerSegmentationsModuleLogic.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerSegmentationsReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerSegmentationsModuleLogic> SegmentationsLogic;
};

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Segmentations
qSlicerSegmentationsReader::qSlicerSegmentationsReader(vtkSlicerSegmentationsModuleLogic* segmentationsLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSegmentationsReaderPrivate)
{
  this->setSegmentationsLogic(segmentationsLogic);
}

//-----------------------------------------------------------------------------
qSlicerSegmentationsReader::~qSlicerSegmentationsReader()
{
}

//-----------------------------------------------------------------------------
void qSlicerSegmentationsReader::setSegmentationsLogic(vtkSlicerSegmentationsModuleLogic* newSegmentationsLogic)
{
  Q_D(qSlicerSegmentationsReader);
  d->SegmentationsLogic = newSegmentationsLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerSegmentationsModuleLogic* qSlicerSegmentationsReader::segmentationsLogic()const
{
  Q_D(const qSlicerSegmentationsReader);
  return d->SegmentationsLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerSegmentationsReader::description()const
{
  return "Segmentation";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerSegmentationsReader::fileType()const
{
  return QString("SegmentationFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerSegmentationsReader::extensions()const
{
  return QStringList() << "Segmentation (*.seg.nrrd)" << "Segmentation (*.seg.vtm)" << "Segmentation (*.nrrd)" << "Segmentation (*.vtm)";
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentationsReader::load(const IOProperties& properties)
{
  Q_D(qSlicerSegmentationsReader);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  this->setLoadedNodes(QStringList());
  if (d->SegmentationsLogic.GetPointer() == 0)
    {
    return false;
    }

  vtkMRMLSegmentationNode* node = d->SegmentationsLogic->LoadSegmentationFromFile(fileName.toLatin1().constData());
  if (!node)
    {
    this->setLoadedNodes(QStringList());
    return false;
    }

  this->setLoadedNodes( QStringList(QString(node->GetID())) );
  if (properties.contains("name"))
    {
    std::string uname = this->mrmlScene()->GetUniqueNameByString(properties["name"].toString().toLatin1());
    node->SetName(uname.c_str());
    }

  return true;
}
