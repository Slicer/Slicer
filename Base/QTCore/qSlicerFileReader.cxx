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

/// Qt includes
#include <QFileInfo>

// CTK includes
#include <ctkUtils.h>

/// QtCore includes
#include "qSlicerFileReader.h"

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
QStringList qSlicerFileReader::extensions()const
{
  return QStringList() << "*.*";
}

//----------------------------------------------------------------------------
bool qSlicerFileReader::canLoadFile(const QString& fileName)const
{
  QStringList res = this->supportedNameFilters(fileName);
  return res.count() > 0;
}

//----------------------------------------------------------------------------
QStringList qSlicerFileReader::supportedNameFilters(const QString& fileName, int* longestExtensionMatchPtr /* =nullptr */)const
{
  if (longestExtensionMatchPtr)
    {
    (*longestExtensionMatchPtr) = 0;
    }
  QStringList matchingNameFilters;
  QFileInfo file(fileName);
  if (!file.isFile() ||
      !file.isReadable() ||
      file.suffix().contains('~')) //temporary file
    {
    return matchingNameFilters;
    }
  foreach(const QString& nameFilter, this->extensions())
    {
    foreach(QString extension, ctk::nameFilterToExtensions(nameFilter))
      {
      QRegExp regExp(extension, Qt::CaseInsensitive, QRegExp::Wildcard);
      Q_ASSERT(regExp.isValid());
      if (regExp.exactMatch(file.absoluteFilePath()))
        {
        extension.remove('*'); // wildcard does not count, that's not a specific match
        int matchedExtensionLength = extension.size();
        if (longestExtensionMatchPtr && (*longestExtensionMatchPtr) < matchedExtensionLength)
          {
          (*longestExtensionMatchPtr) = matchedExtensionLength;
          }
        matchingNameFilters << nameFilter;
        }
      }
    }
  matchingNameFilters.removeDuplicates();
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
QStringList qSlicerFileReader::loadedNodes()const
{
  Q_D(const qSlicerFileReader);
  return d->LoadedNodes;
}

//----------------------------------------------------------------------------
bool qSlicerFileReader::examineFileInfoList(QFileInfoList &fileInfoList, QFileInfo &archetypeFileInfo, qSlicerIO::IOProperties &ioProperties)const
{
  Q_UNUSED(fileInfoList);
  Q_UNUSED(archetypeFileInfo);
  Q_UNUSED(ioProperties);
  return(false);
}
