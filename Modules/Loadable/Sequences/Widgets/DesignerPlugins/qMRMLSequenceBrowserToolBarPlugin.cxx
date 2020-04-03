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

#include "qMRMLSequenceBrowserToolBarPlugin.h"
#include "qMRMLSequenceBrowserToolBar.h"

// --------------------------------------------------------------------------
qMRMLSequenceBrowserToolBarPlugin::qMRMLSequenceBrowserToolBarPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLSequenceBrowserToolBarPlugin::createWidget(QWidget *_parent)
{
  qMRMLSequenceBrowserToolBar* _widget = new qMRMLSequenceBrowserToolBar(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLSequenceBrowserToolBarPlugin::domXml() const
{
  return "<widget class=\"qMRMLSequenceBrowserToolBar\" \
          name=\"MRMLSequenceBrowserToolBar\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLSequenceBrowserToolBarPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qMRMLSequenceBrowserToolBarPlugin::includeFile() const
{
  return "qMRMLSequenceBrowserToolBar.h";
}

// --------------------------------------------------------------------------
bool qMRMLSequenceBrowserToolBarPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLSequenceBrowserToolBarPlugin::name() const
{
  return "qMRMLSequenceBrowserToolBar";
}
