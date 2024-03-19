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

#include "qSlicerModelsWidgetsAbstractPlugin.h"

//-----------------------------------------------------------------------------
qSlicerModelsWidgetsAbstractPlugin::qSlicerModelsWidgetsAbstractPlugin() = default;

//-----------------------------------------------------------------------------
QString qSlicerModelsWidgetsAbstractPlugin::group() const
{
  return "Slicer [Models Widgets]";
}

//-----------------------------------------------------------------------------
QIcon qSlicerModelsWidgetsAbstractPlugin::icon() const
{
  return QIcon();
}

//-----------------------------------------------------------------------------
QString qSlicerModelsWidgetsAbstractPlugin::toolTip() const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerModelsWidgetsAbstractPlugin::whatsThis() const
{
  return QString();
}
