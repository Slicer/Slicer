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

// QtCore includes
#include "qSlicerSceneIO.h"

// MRML includes
#include <vtkMRMLScene.h>

//-----------------------------------------------------------------------------
qSlicerSceneIO::qSlicerSceneIO(QObject* _parent)
  :qSlicerIO(_parent)
{
}

//-----------------------------------------------------------------------------
QString qSlicerSceneIO::description()const
{
  return "MRML Scene";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerSceneIO::fileType()const
{
  return qSlicerIO::SceneFile;
}

//-----------------------------------------------------------------------------
QStringList qSlicerSceneIO::extensions()const 
{
  return QStringList() << "*.mrml";
}

//-----------------------------------------------------------------------------
bool qSlicerSceneIO::load(const qSlicerIO::IOProperties& properties)
{
  Q_ASSERT(properties.contains("fileName"));
  QString file = properties["fileName"].toString();
  this->mrmlScene()->SetURL(file.toLatin1());
  bool clear = false;
  if (properties.contains("clear"))
    {
    clear = properties["clear"].toBool();
    }
  int res = 0;
  if (clear)
    {
    res = this->mrmlScene()->Connect();
    }
  else
    {
    res = this->mrmlScene()->Import();
    }
  return res;
}
