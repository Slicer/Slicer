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

==============================================================================*/

#include "qSlicerSequencesModuleWidgetsAbstractPlugin.h"

//-----------------------------------------------------------------------------
qSlicerSequencesModuleWidgetsAbstractPlugin::qSlicerSequencesModuleWidgetsAbstractPlugin() = default;

//-----------------------------------------------------------------------------
QString qSlicerSequencesModuleWidgetsAbstractPlugin::group() const
{
  return "Slicer [Sequence Browser Widgets]";
}

//-----------------------------------------------------------------------------
QIcon qSlicerSequencesModuleWidgetsAbstractPlugin::icon() const
{
  return QIcon();
}

//-----------------------------------------------------------------------------
QString qSlicerSequencesModuleWidgetsAbstractPlugin::toolTip() const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerSequencesModuleWidgetsAbstractPlugin::whatsThis() const
{
  return QString();
}
