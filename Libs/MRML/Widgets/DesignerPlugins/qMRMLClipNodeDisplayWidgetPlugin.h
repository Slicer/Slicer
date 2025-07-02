/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported in part through NIH grant R01 HL153166.

==============================================================================*/

#ifndef __qMRMLClipNodeDisplayWidgetPlugin_h
#define __qMRMLClipNodeDisplayWidgetPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

class QMRML_WIDGETS_PLUGINS_EXPORT qMRMLClipNodeDisplayWidgetPlugin
  : public QObject
  , public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLClipNodeDisplayWidgetPlugin(QObject* _parent = nullptr);

  QWidget* createWidget(QWidget* _parent) override;
  QString domXml() const override;
  QIcon icon() const override;
  QString includeFile() const override;
  bool isContainer() const override;
  QString name() const override;
};

#endif
