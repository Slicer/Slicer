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

// Qt includes

// qMRML includes
#include "qMRMLColors.h"

//------------------------------------------------------------------------------
QColor qMRMLColors::sliceRed()
{
  // 243, 74, 51 -> #F34A33
  QColor flamingo = QColor::fromRgbF(0.952941176471, 0.290196078431, 0.2);
  return flamingo;
}

//------------------------------------------------------------------------------
QColor qMRMLColors::sliceGreen()
{
  // 110, 176, 75 -> #6EB04B
  QColor chelseaCucumber = QColor::fromRgbF(0.43137254902, 0.690196078431, 0.294117647059);
  return chelseaCucumber;
}

//------------------------------------------------------------------------------
QColor qMRMLColors::sliceYellow()
{
  // 237, 213, 76 -> #EDD54C
  QColor ronchy = QColor::fromRgbF(0.929411764706, 0.835294117647, 0.298039215686);
  return ronchy;
}

//------------------------------------------------------------------------------
QColor qMRMLColors::sliceOrange()
{
  // 225, 112, 18 -> #E17012
  QColor tango = QColor::fromRgbF(0.882352941176, 0.439215686275, 0.0705882352941);
  return tango;
}

//------------------------------------------------------------------------------
QColor qMRMLColors::threeDViewBlue()
{
  // 116, 131, 233 -> #7483E9
  QColor blue = QColor::fromRgb(116, 131, 233);
  return blue;
}

//------------------------------------------------------------------------------
QColor qMRMLColors::sliceGray()
{
  // #8C8C8C
  QColor gray = QColor("#8C8C8C");
  return gray;
}
