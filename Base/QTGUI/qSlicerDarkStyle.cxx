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
#include <QLinearGradient>
#include <QPalette>

// qMRML includes
#include "qSlicerDarkStyle.h"

// --------------------------------------------------------------------------
// qSlicerStyle methods

// --------------------------------------------------------------------------
qSlicerDarkStyle::qSlicerDarkStyle() = default;

// --------------------------------------------------------------------------
qSlicerDarkStyle::~qSlicerDarkStyle() = default;

//------------------------------------------------------------------------------
QPalette qSlicerDarkStyle::standardPalette()const
{
  QPalette palette = this->Superclass::standardPalette();

  // From darkest to lightest
  palette.setColor(QPalette::Shadow, "#0d0d0f");

  palette.setColor(QPalette::AlternateBase, "#4d4d3f");
  palette.setColor(QPalette::ToolTipBase, "#3d3d3f");

  // Cleanlooks style requires a window gradient if a button gradient is used.
  QLinearGradient windowGradient;
  windowGradient.setColorAt(0., QColor("#424244"));
  windowGradient.setColorAt(1., QColor("#424244"));
  QBrush windowBrush(windowGradient);
  windowBrush.setColor(QColor("#424244"));
  palette.setBrush(QPalette::Window, windowBrush);
  windowGradient.setColorAt(0., QColor("#222224"));
  windowGradient.setColorAt(1., QColor("#222224"));
  windowBrush = QBrush(windowGradient);
  windowBrush.setColor(QColor("#222224"));
  palette.setBrush(QPalette::Disabled, QPalette::Window, windowBrush);

  palette.setColor(QPalette::Base, "#525254");
  palette.setColor(QPalette::Disabled, QPalette::Base, "#323234");

  palette.setColor(QPalette::Dark, "#3d3d3f");
  palette.setColor(QPalette::Mid, "#4d4d4f");
  QLinearGradient buttonGradient(0.,0.,0.,1.);
  buttonGradient.setColorAt(0., QColor("#6d6d6f"));
  buttonGradient.setColorAt(1., QColor("#5d5d5f"));
  QBrush buttonBrush(buttonGradient);
  buttonBrush.setColor(QColor("#626264")); // for the scrollbars
  palette.setBrush(QPalette::Active, QPalette::Button, buttonBrush);
  palette.setBrush(QPalette::Inactive, QPalette::Button, buttonBrush);

  buttonGradient.setColorAt(0., QColor("#424244"));
  buttonGradient.setColorAt(1., QColor("#525254"));
  buttonBrush = QBrush(buttonGradient);
  buttonBrush.setColor(QColor("#4d4d4f"));
  palette.setBrush(QPalette::Disabled, QPalette::Button, buttonBrush);
  palette.setColor(QPalette::Midlight, "#727274");
  palette.setColor(QPalette::Light, "#828284");

  palette.setColor(QPalette::WindowText, "#e2e2e4");
  palette.setColor(QPalette::Disabled, QPalette::WindowText, "#323234");
  palette.setColor(QPalette::Text, "#f2f2f4");
  palette.setColor(QPalette::Disabled, QPalette::Text, "#828284");
  palette.setColor(QPalette::ToolTipText, "#e2e2e4");
  palette.setColor(QPalette::ButtonText, "#ededef");
  palette.setColor(QPalette::Disabled, QPalette::ButtonText, "#727274");

  palette.setColor(QPalette::BrightText, Qt::white);
  return palette;
}
