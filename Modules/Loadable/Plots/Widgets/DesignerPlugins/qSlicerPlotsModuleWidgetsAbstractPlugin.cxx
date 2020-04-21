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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qSlicerPlotsModuleWidgetsAbstractPlugin.h"

//-----------------------------------------------------------------------------
qSlicerPlotsModuleWidgetsAbstractPlugin::qSlicerPlotsModuleWidgetsAbstractPlugin() = default;

//-----------------------------------------------------------------------------
QString qSlicerPlotsModuleWidgetsAbstractPlugin::group() const
{
  return "Slicer [Plots Widgets]";
}

//-----------------------------------------------------------------------------
QIcon qSlicerPlotsModuleWidgetsAbstractPlugin::icon() const
{
  return QIcon();
}

//-----------------------------------------------------------------------------
QString qSlicerPlotsModuleWidgetsAbstractPlugin::toolTip() const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerPlotsModuleWidgetsAbstractPlugin::whatsThis() const
{
  return QString();
}
