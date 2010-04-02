/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

// QT includes
#include <QApplication>
#include <QString>

// qCTK includes
#include "qCTKDoubleRangeSlider.h"

// STD includes
#include <stdlib.h>
#include <iostream>

bool checkSlider(const qCTKDoubleRangeSlider& slider)
{
  return slider.minimum()  <= slider.minimumValue() &&
    slider.minimumValue() <= slider.maximumValue() &&
    slider.maximumValue() <= slider.maximum();
}

bool checkSlider(const qCTKDoubleRangeSlider& slider, 
                 double min, double minVal, double maxVal, double max)
{
  return qFuzzyCompare(slider.minimum(), min) &&
    qFuzzyCompare(slider.minimumValue(), minVal) &&
    qFuzzyCompare(slider.maximumValue(), maxVal) &&
    qFuzzyCompare(slider.maximum(), max);
}

bool checkSlider(const qCTKDoubleRangeSlider& slider, 
                 double min, double minVal, double maxVal, double max, double minPos, double maxPos)
{
  return qFuzzyCompare(slider.minimum(), min) &&
    qFuzzyCompare(slider.minimumValue(), minVal) &&
    qFuzzyCompare(slider.maximumValue(), maxVal) &&
    qFuzzyCompare(slider.maximum(), max) &&
    qFuzzyCompare(slider.minimumPosition(), minPos) &&
    qFuzzyCompare(slider.maximumPosition(), maxPos);
}

int qCTKDoubleRangeSliderTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);

  qCTKDoubleRangeSlider slider;
  if (!(slider.minimum()  <= slider.minimumValue() &&
        slider.minimumValue() <= slider.maximumValue() &&
        slider.maximumValue() <= slider.maximum() &&
        slider.minimum() <= slider.minimumPosition() &&
        slider.minimumPosition() <= slider.maximumPosition() &&
        slider.maximumPosition() <= slider.maximum()))
    {
    std::cerr << "qCTKDoubleRangeSlider:: 1) "
              << slider.minimum() << " " 
              << slider.minimumValue() << " " 
              << slider.maximumValue() << " " 
              << slider.maximum() << std::endl;
    std::cerr << "qCTKDoubleRangeSlider:: 1) "
              << slider.minimumPosition() << " " 
              << slider.maximumPosition() << std::endl;
    return EXIT_FAILURE;
    }
  slider.setMinimum(10.10);
  slider.setMaximum(3999.99);
  slider.setValues(40.1, 20.4);
  if (!checkSlider(slider, 10.10, 20.4, 40.1, 3999.99, 20.4, 40.1))
    {
    std::cerr << "qCTKDoubleRangeSlider:: 2) "
              << slider.minimum() << " " 
              << slider.minimumValue() << " " 
              << slider.maximumValue() << " " 
              << slider.maximum() << std::endl;
    std::cerr << "qCTKDoubleRangeSlider:: 2) "
              << slider.minimumPosition() << " " 
              << slider.maximumPosition() << std::endl;
    return EXIT_FAILURE;
    }
  slider.setValues(700.4, 20.99);
  if (!checkSlider(slider, 10.1, 20.99, 700.4, 3999.99, 20.99, 700.4))
    {
    std::cerr << "qCTKDoubleRangeSlider:: 3) "
              << slider.minimum() << " " 
              << slider.minimumValue() << " " 
              << slider.maximumValue() << " " 
              << slider.maximum() << std::endl;
    std::cerr << "qCTKDoubleRangeSlider:: 3) "
              << slider.minimumPosition() << " " 
              << slider.maximumPosition() << std::endl;
    return EXIT_FAILURE;
    }
  slider.setValues(800, 900);
  if (!checkSlider(slider, 10.1, 800, 900, 3999.99, 800, 900))
    {
    std::cerr << "qCTKDoubleRangeSlider:: 4) "
              << slider.minimum() << " " 
              << slider.minimumValue() << " " 
              << slider.maximumValue() << " " 
              << slider.maximum() << std::endl;
    std::cerr << "qCTKDoubleRangeSlider:: 4) "
              << slider.minimumPosition() << " " 
              << slider.maximumPosition() << std::endl;
    return EXIT_FAILURE;
    }
  slider.setValues(5000, 6000);
  if (!checkSlider(slider, 10.1, 3999.99, 3999.99, 3999.99, 3999.99, 3999.99))
    {
    std::cerr << "qCTKDoubleRangeSlider:: 5) "
              << slider.minimum() << " " 
              << slider.minimumValue() << " " 
              << slider.maximumValue() << " " 
              << slider.maximum() << std::endl;
    std::cerr << "qCTKDoubleRangeSlider:: 5) "
              << slider.minimumPosition() << " " 
              << slider.maximumPosition() << std::endl;
    return EXIT_FAILURE;
    }
  slider.setRange(-15.21, 25.49999);
  if (!checkSlider(slider, -15.21, 25.49999, 25.49999, 25.49999, 25.49999, 25.49999))
    {
    std::cerr << "qCTKDoubleRangeSlider:: 6) "
              << slider.minimum() << " " 
              << slider.minimumValue() << " " 
              << slider.maximumValue() << " " 
              << slider.maximum() << std::endl;
    return EXIT_FAILURE;
    }
  // nothing happen yet but it would be nice if it moves the 2 values at 
  // the same time.
  slider.triggerAction(QAbstractSlider::SliderSingleStepSub);
  if (!checkSlider(slider, -15.21, 25.49999, 25.49999, 25.49999, 25.49999, 25.49999))
    {
    std::cerr << "qCTKDoubleRangeSlider:: 7) "
              << slider.minimum() << " " 
              << slider.minimumValue() << " " 
              << slider.maximumValue() << " " 
              << slider.maximum() << std::endl;
    std::cerr << "qCTKDoubleRangeSlider:: 7) "
              << slider.minimumPosition() << " " 
              << slider.maximumPosition() << std::endl;
    return EXIT_FAILURE;
    }
  slider.setMaximumValue(10.2);
  if (!checkSlider(slider, -15.21, 10.2, 10.2, 25.49999, 10.2, 10.2))
    {
    std::cerr << "qCTKDoubleRangeSlider:: 8) "
              << slider.minimum() << " " 
              << slider.minimumValue() << " " 
              << slider.maximumValue() << " " 
              << slider.maximum() << std::endl;
    std::cerr << "qCTKDoubleRangeSlider:: 8) "
              << slider.minimumPosition() << " " 
              << slider.maximumPosition() << std::endl;
    return EXIT_FAILURE;
    }
  slider.setMinimumValue(17.7777);
  if (!checkSlider(slider, -15.21, 17.7777, 17.7777, 25.49999, 17.7777, 17.7777))
    {
    std::cerr << "qCTKDoubleRangeSlider:: 9) "
              << slider.minimum() << " " 
              << slider.minimumValue() << " " 
              << slider.maximumValue() << " " 
              << slider.maximum() << std::endl;
    std::cerr << "qCTKDoubleRangeSlider:: 9) "
              << slider.minimumPosition() << " " 
              << slider.maximumPosition() << std::endl;
    return EXIT_FAILURE;
    }
  slider.setMinimumValue(-20);
  if (!checkSlider(slider, -15.21, -15.21, 17.7777, 25.49999, -15.21, 17.7777))
    {
    std::cerr << "qCTKDoubleRangeSlider:: 10) "
              << slider.minimum() << " " 
              << slider.minimumValue() << " " 
              << slider.maximumValue() << " " 
              << slider.maximum() << std::endl;
    std::cerr << "qCTKDoubleRangeSlider:: 10) "
              << slider.minimumPosition() << " " 
              << slider.maximumPosition() << std::endl;
    return EXIT_FAILURE;
    }
  slider.setSingleStep(0.15);
  if (!checkSlider(slider, -15.21, -15.21, 17.7777, 25.49999, -15.21, 17.7777))
    {
    std::cerr << "qCTKDoubleRangeSlider:: 10.5) "
              << slider.minimum() << " " 
              << slider.minimumValue() << " " 
              << slider.maximumValue() << " " 
              << slider.maximum() << std::endl;
    std::cerr << "qCTKDoubleRangeSlider:: 10.5) "
              << slider.minimumPosition() << " " 
              << slider.maximumPosition() << std::endl;
    return EXIT_FAILURE;
    }
  slider.setSingleStep(0.33);
  if (!checkSlider(slider, -15.21, -15.21, 17.7777, 25.49999, -15.21, 17.7777))
    {
    std::cerr << "qCTKDoubleRangeSlider:: 11) "
              << slider.minimum() << " " 
              << slider.minimumValue() << " " 
              << slider.maximumValue() << " " 
              << slider.maximum() << std::endl;
    std::cerr << "qCTKDoubleRangeSlider:: 11) "
              << slider.minimumPosition() << " " 
              << slider.maximumPosition() << std::endl;
    return EXIT_FAILURE;
    }
  slider.setMinimumValue(-15.205);
  if (!checkSlider(slider, -15.21, -15.205, 17.7777, 25.49999, -15.205, 17.7777))
    {
    std::cerr << "qCTKDoubleRangeSlider:: 12) "
              << slider.minimum() << " " 
              << slider.minimumValue() << " " 
              << slider.maximumValue() << " " 
              << slider.maximum() << std::endl;
    std::cerr << "qCTKDoubleRangeSlider:: 12) "
              << slider.minimumPosition() << " " 
              << slider.maximumPosition() << std::endl;
    return EXIT_FAILURE;
    }
  slider.setSingleStep(1.);
  if (!checkSlider(slider, -15.21, -15.205, 17.7777, 25.49999, -15.205, 17.7777))
    {
    std::cerr << "qCTKDoubleRangeSlider:: 13) "
              << slider.minimum() << " " 
              << slider.minimumValue() << " " 
              << slider.maximumValue() << " " 
              << slider.maximum() << std::endl;
    std::cerr << "qCTKDoubleRangeSlider:: 13) "
              << slider.minimumPosition() << " " 
              << slider.maximumPosition() << std::endl;
    return EXIT_FAILURE;
    }
  slider.setSingleStep(0.01);
  if (!checkSlider(slider, -15.21, -15.205, 17.7777, 25.49999, -15.205, 17.7777))
    {
    std::cerr << "qCTKDoubleRangeSlider:: 14) "
              << slider.minimum() << " " 
              << slider.minimumValue() << " " 
              << slider.maximumValue() << " " 
              << slider.maximum() << std::endl;
    std::cerr << "qCTKDoubleRangeSlider:: 14) "
              << slider.minimumPosition() << " " 
              << slider.maximumPosition() << std::endl;
    return EXIT_FAILURE;
    }
  // FIXME
  //slider.setPositions(0.1, 20.5);
  // if (!checkSlider(slider, -15.21, 0.1, 20.5, 25.49999, 0.1, 20.5))
  //   {
  //   std::cerr << "qCTKDoubleRangeSlider:: 15) "
  //             << slider.minimum() << " " 
  //             << slider.minimumValue() << " " 
  //             << slider.maximumValue() << " " 
  //             << slider.maximum() << std::endl;
  //   std::cerr << "qCTKDoubleRangeSlider:: 15) "
  //             << slider.minimumPosition() << " " 
  //             << slider.maximumPosition() << std::endl;
  //   return EXIT_FAILURE;
  //   }
  // slider.setTracking(false); 
  // slider.setPositions(0.123456, 20.123465);
  // if (!checkSlider(slider, -15.21, 0.1, 20.5, 25.49999, 0.123456, 20.123456))
  //   {
  //   std::cerr << "qCTKDoubleRangeSlider:: 16) "
  //             << slider.minimum() << " " 
  //             << slider.minimumValue() << " " 
  //             << slider.maximumValue() << " " 
  //             << slider.maximum() << std::endl;
  //   std::cerr << "qCTKDoubleRangeSlider:: 16) "
  //             << slider.minimumPosition() << " " 
  //             << slider.maximumPosition() << std::endl;
  //   return EXIT_FAILURE;
  //   }

  return EXIT_SUCCESS;
}
