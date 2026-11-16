/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

#include "qSlicerVectorFieldRenderingModuleWidgetsAbstractPlugin.h"

//-----------------------------------------------------------------------------
qSlicerVectorFieldRenderingModuleWidgetsAbstractPlugin::qSlicerVectorFieldRenderingModuleWidgetsAbstractPlugin() = default;

//-----------------------------------------------------------------------------
QString qSlicerVectorFieldRenderingModuleWidgetsAbstractPlugin::group() const
{
  return "Slicer [MRML Widgets]";
}

//-----------------------------------------------------------------------------
QIcon qSlicerVectorFieldRenderingModuleWidgetsAbstractPlugin::icon() const
{
  return QIcon();
}

//-----------------------------------------------------------------------------
QString qSlicerVectorFieldRenderingModuleWidgetsAbstractPlugin::toolTip() const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerVectorFieldRenderingModuleWidgetsAbstractPlugin::whatsThis() const
{
  return QString();
}
