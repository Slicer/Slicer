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
#include "qCTKDoubleSlider.h"

// STD includes
#include <stdlib.h>
#include <iostream>

int qCTKDoubleSliderTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);

  qCTKDoubleSlider slider;
  std::cout << "Test setValue(18.54)" << std::endl;
  slider.setValue(18.54);
  if (!qFuzzyCompare(slider.value(), 18.54))
    {
    std::cerr << "qCTKDoubleSlider::setValue failed." << slider.value() << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Test setMinimum(5.21)" << std::endl;
  slider.setMinimum(5.21);
  if (!qFuzzyCompare(slider.minimum(), 5.21) ||
      !qFuzzyCompare(slider.value(),18.54))
    {
    std::cerr << "qCTKDoubleSlider::setMinimum failed." << slider.value() << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Test setMaximum(1340.2)" << std::endl;
  slider.setMaximum(1340.2);
  if (!qFuzzyCompare(slider.maximum(), 1340.2) ||
      !qFuzzyCompare(slider.value(), 18.54))
    {
    std::cerr << "qCTKDoubleSlider::setMinimum failed." << slider.value() << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Test setSingleStep(0.15)" << std::endl;
  slider.setSingleStep(0.15);
  if (!qFuzzyCompare(slider.singleStep(), 0.15) ||
      !qFuzzyCompare(slider.value(), 18.54))
    {
    std::cerr << "qCTKDoubleSlider::setSingleStep failed. Step:" << slider.singleStep() << " Val:" << slider.value() << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Test setSingleStep(0.33)" << std::endl;
  slider.setSingleStep(0.33);
  if (!qFuzzyCompare(slider.singleStep(), 0.33) ||
      !qFuzzyCompare(slider.value(), 18.54))
    {
    std::cerr << "qCTKDoubleSlider::setSingleStep failed." << slider.value() << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Test Step Add" << std::endl;
  slider.triggerAction (QAbstractSlider::SliderSingleStepAdd);
  if (!qFuzzyCompare(slider.value(), 18.87))
    {
    std::cerr << "qCTKDoubleSlider:: Step Add failed" << slider.value() << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Test Step Sub" << std::endl;
  slider.triggerAction (QAbstractSlider::SliderSingleStepSub);
  if (!qFuzzyCompare(slider.value(), 18.54))
    {
    std::cerr << "qCTKDoubleSlider:: Step Sub failed" << slider.value() << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Test setMaximum" << std::endl;
  slider.setMaximum(15.08);
  if (!qFuzzyCompare(slider.maximum(), 15.08) ||
      !qFuzzyCompare(slider.value(), 15.08))
    {
    std::cerr << "qCTKDoubleSlider:: setMaximum failed" << slider.value() << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Test setMaximum" << std::endl;
  slider.setMaximum(5100000.333333);
  if (!qFuzzyCompare(slider.maximum(), 5100000.333333) ||
      !qFuzzyCompare(slider.value(), 15.08))
    {
    std::cerr << "qCTKDoubleSlider:: setMaximum failed" << slider.value() << std::endl;
    return EXIT_FAILURE;
    }    
  std::cout << "Test setValue close to previous value" << std::endl;
  slider.setValue(15.081);
  if (!qFuzzyCompare(slider.value(), 15.081))
    {
    std::cerr << "qCTKDoubleSlider:: setValue failed" << slider.value() << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Test setSingleStep" << std::endl;
  slider.setSingleStep(1.);
  if (!qFuzzyCompare(slider.value(),15.081) || 
      !qFuzzyCompare(slider.sliderPosition(), 15.081))
    {
    std::cerr << "qCTKDoubleSlider:: setSingleStep failed: val" << slider.value() << " pos:" << slider.sliderPosition() << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Test setSingleStep " << std::endl;
  slider.setSingleStep(0.01);
  if (!qFuzzyCompare(slider.value(),15.081) || 
      !qFuzzyCompare(slider.sliderPosition(), 15.081))
    {
    std::cerr << "qCTKDoubleSlider:: setSingleStep failed: val" << slider.value() << " pos:" << slider.sliderPosition() << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

