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

/// QtCore includes
#include "qSlicerFileReader.h"

// CTK includes
#include <ctkUtils.h>

// Slicer includes
#include <vtkMRMLFileIOHandler.h>

//-----------------------------------------------------------------------------
class qSlicerFileReaderPrivate
{
public:
  QStringList LoadedNodes;
};

//----------------------------------------------------------------------------
qSlicerFileReader::qSlicerFileReader(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerFileReaderPrivate)
{
}

//----------------------------------------------------------------------------
qSlicerFileReader::~qSlicerFileReader() = default;

//----------------------------------------------------------------------------
QStringList qSlicerFileReader::extensions() const
{
  return QStringList() << "*.*";
}

//----------------------------------------------------------------------------
bool qSlicerFileReader::canLoadFile(const QString& fileName) const
{
  return this->supportedNameFilters(fileName).count() > 0;
}

//----------------------------------------------------------------------------
double qSlicerFileReader::canLoadFileConfidence(const QString& fileName) const
{
  if (!this->canLoadFile(fileName))
  {
    return 0.0;
  }
  int longestExtensionMatch = 0;
  this->supportedNameFilters(fileName, &longestExtensionMatch);
  // If longer extension is matched then the confidence that this is a good reader is
  // slightly higher. For example, for "somefile.seg.nrrd", a reader that is specifically
  // for ".seg.nrrd" files get slightly higher confidence than readers that of generic ".nrrd" files.
  return 0.5 + 0.01 * longestExtensionMatch;
}

//----------------------------------------------------------------------------
QStringList qSlicerFileReader::supportedNameFilters(const QString& fileName, int* longestExtensionMatchPtr /* =nullptr */) const
{
  std::vector<std::string> nameFilters;
  ctk::qListToSTLVector(this->extensions(), nameFilters);
  std::vector<std::string> matchingNameFiltersVector =
    vtkMRMLFileIOHandler::GetMatchingNameFilters(fileName.toStdString(), nameFilters, /*requireReadableFile=*/true, longestExtensionMatchPtr);
  QStringList matchingNameFilters;
  ctk::stlVectorToQList(matchingNameFiltersVector, matchingNameFilters);
  return matchingNameFilters;
}

//----------------------------------------------------------------------------
bool qSlicerFileReader::load(const IOProperties& properties)
{
  Q_D(qSlicerFileReader);
  Q_UNUSED(properties);
  d->LoadedNodes.clear();
  return false;
}

//----------------------------------------------------------------------------
void qSlicerFileReader::setLoadedNodes(const QStringList& nodes)
{
  Q_D(qSlicerFileReader);
  d->LoadedNodes = nodes;
}

//----------------------------------------------------------------------------
QStringList qSlicerFileReader::loadedNodes() const
{
  Q_D(const qSlicerFileReader);
  return d->LoadedNodes;
}

//----------------------------------------------------------------------------
bool qSlicerFileReader::examineFileInfoList(QFileInfoList& fileInfoList, QFileInfo& archetypeFileInfo, qSlicerIO::IOProperties& ioProperties) const
{
  Q_UNUSED(fileInfoList);
  Q_UNUSED(archetypeFileInfo);
  Q_UNUSED(ioProperties);
  return false;
}
