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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QStringList>
#include <QStyle>

// qMRML includes
#include "qSlicerStyle.h"
#include "qSlicerLightStyle.h"
#include "qSlicerDarkStyle.h"
#include "qSlicerStylePlugin.h"

// --------------------------------------------------------------------------
// qSlicerStylePlugin methods

//-----------------------------------------------------------------------------
qSlicerStylePlugin::qSlicerStylePlugin() = default;

//-----------------------------------------------------------------------------
qSlicerStylePlugin::~qSlicerStylePlugin() = default;

//-----------------------------------------------------------------------------
QStyle* qSlicerStylePlugin::create( const QString & key )
{
  if (key.compare("Slicer", Qt::CaseInsensitive) == 0)
    {
    return new qSlicerStyle();
    }
  if (key.compare("Light Slicer", Qt::CaseInsensitive) == 0)
    {
    return new qSlicerLightStyle();
    }
  if (key.compare("Dark Slicer", Qt::CaseInsensitive) == 0)
    {
    return new qSlicerDarkStyle();
    }
  return nullptr;
}

//-----------------------------------------------------------------------------
QStringList qSlicerStylePlugin::keys() const
{
  return QStringList() << "Slicer" << "Light Slicer" << "Dark Slicer";
}

