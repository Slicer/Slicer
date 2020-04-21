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
#include "qSlicerFileWriter.h"

//-----------------------------------------------------------------------------
class qSlicerFileWriterPrivate
{
public:
  QStringList WrittenNodes;
};

//----------------------------------------------------------------------------
qSlicerFileWriter::qSlicerFileWriter(QObject* parentObject)
  : qSlicerIO(parentObject)
  , d_ptr(new qSlicerFileWriterPrivate)
{
}

//----------------------------------------------------------------------------
qSlicerFileWriter::~qSlicerFileWriter() = default;

//----------------------------------------------------------------------------
bool qSlicerFileWriter::canWriteObject(vtkObject* object)const
{
  Q_UNUSED(object);
  return false;
}

//----------------------------------------------------------------------------
bool qSlicerFileWriter::write(const qSlicerIO::IOProperties& properties)
{
  Q_D(qSlicerFileWriter);
  Q_UNUSED(properties);
  d->WrittenNodes.clear();
  return false;
}

//----------------------------------------------------------------------------
void qSlicerFileWriter::setWrittenNodes(const QStringList& nodes)
{
  Q_D(qSlicerFileWriter);
  d->WrittenNodes = nodes;
}

//----------------------------------------------------------------------------
QStringList qSlicerFileWriter::writtenNodes()const
{
  Q_D(const qSlicerFileWriter);
  return d->WrittenNodes;
}
