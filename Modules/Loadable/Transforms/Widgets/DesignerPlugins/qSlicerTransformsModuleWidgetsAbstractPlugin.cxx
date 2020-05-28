/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


#include "qSlicerTransformsModuleWidgetsAbstractPlugin.h"

//-----------------------------------------------------------------------------
qSlicerTransformsModuleWidgetsAbstractPlugin::qSlicerTransformsModuleWidgetsAbstractPlugin() = default;

//-----------------------------------------------------------------------------
QString qSlicerTransformsModuleWidgetsAbstractPlugin::group() const
{
  return "Slicer [Transforms Widgets]";
}

//-----------------------------------------------------------------------------
QIcon qSlicerTransformsModuleWidgetsAbstractPlugin::icon() const
{
  return QIcon();
}

//-----------------------------------------------------------------------------
QString qSlicerTransformsModuleWidgetsAbstractPlugin::toolTip() const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerTransformsModuleWidgetsAbstractPlugin::whatsThis() const
{
  return QString();
}
