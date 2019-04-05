/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "qMRMLMarkupsDisplayNodeWidgetPlugin.h"
#include "qMRMLMarkupsDisplayNodeWidget.h"

//------------------------------------------------------------------------------
qMRMLMarkupsDisplayNodeWidgetPlugin
::qMRMLMarkupsDisplayNodeWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qMRMLMarkupsDisplayNodeWidgetPlugin
::createWidget(QWidget *_parent)
{
  qMRMLMarkupsDisplayNodeWidget* _widget
    = new qMRMLMarkupsDisplayNodeWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qMRMLMarkupsDisplayNodeWidgetPlugin
::domXml() const
{
  return "<widget class=\"qMRMLMarkupsDisplayNodeWidget\" \
          name=\"MarkupsDisplayNodeWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qMRMLMarkupsDisplayNodeWidgetPlugin
::includeFile() const
{
  return "qMRMLMarkupsDisplayNodeWidget.h";
}

//------------------------------------------------------------------------------
bool qMRMLMarkupsDisplayNodeWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qMRMLMarkupsDisplayNodeWidgetPlugin
::name() const
{
  return "qMRMLMarkupsDisplayNodeWidget";
}
