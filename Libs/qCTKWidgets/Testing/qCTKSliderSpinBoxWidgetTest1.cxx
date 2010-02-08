/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

// qCTK includes
#include "qCTKSliderSpinBoxWidget.h"

// QT includes
#include <QApplication>

// STL includes
#include <stdlib.h>
#include <iostream>

int qCTKSliderSpinBoxWidgetTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);

  qCTKSliderSpinBoxWidget sliderSpinBox;

  // the first part of the tests infer 2 decimals
  if (sliderSpinBox.decimals() != 2)
    {
    std::cerr << "qCTKSliderSpinBoxWidget::decimals default value failed." 
              << sliderSpinBox.decimals() << std::endl;
    return EXIT_FAILURE;
    }

  sliderSpinBox.setValue(32.12);

  if (!qFuzzyCompare(sliderSpinBox.value(), 32.12))
    {
    std::cerr << "qCTKSliderSpinBoxWidget::setValue failed." 
              << sliderSpinBox.value() << std::endl;
    return EXIT_FAILURE;
    }

  sliderSpinBox.setMinimum(10.0123);

  if (!qFuzzyCompare(sliderSpinBox.minimum(), 10.01) || 
      !qFuzzyCompare(sliderSpinBox.value(), 32.12))
    {
    std::cerr << "qCTKSliderSpinBoxWidget::setMinimum failed." 
              << sliderSpinBox.minimum() << " "
              << sliderSpinBox.value() << std::endl;
    return EXIT_FAILURE;
    }
  
  sliderSpinBox.setMaximum(2050.9876);

  if (!qFuzzyCompare(sliderSpinBox.maximum(), 2050.99) || 
      !qFuzzyCompare(sliderSpinBox.value(), 32.12))
    {
    std::cerr << "qCTKSliderSpinBoxWidget::setMaximum failed." 
              << sliderSpinBox.maximum() << " "
              << sliderSpinBox.value() << std::endl;
    return EXIT_FAILURE;
    }

  sliderSpinBox.setSingleStep(0.1);

  if (!qFuzzyCompare(sliderSpinBox.singleStep(), 0.1) || 
      !qFuzzyCompare(sliderSpinBox.value(), 32.12))
    {
    std::cerr << "qCTKSliderSpinBoxWidget::setMaximum failed." 
              << sliderSpinBox.singleStep() << " "
              << sliderSpinBox.value() << std::endl;
    return EXIT_FAILURE;
    }

  sliderSpinBox.setDecimals(1);

  if (sliderSpinBox.decimals() != 1 || 
      !qFuzzyCompare(sliderSpinBox.value(), 32.1))
    {
    std::cerr << "qCTKSliderSpinBoxWidget::setDecimals failed." 
              << sliderSpinBox.decimals() << " "
              << sliderSpinBox.value() << std::endl;
    return EXIT_FAILURE;
    }  

  sliderSpinBox.setValue(77.777);

  if (!qFuzzyCompare(sliderSpinBox.value(), 77.8))
    {
    std::cerr << "qCTKSliderSpinBoxWidget::setValue failed." 
              << sliderSpinBox.value() << std::endl;
    return EXIT_FAILURE;
    }  

  sliderSpinBox.setTickInterval(0.1);

  if (!qFuzzyCompare(sliderSpinBox.tickInterval(), 0.1) || 
      !qFuzzyCompare(sliderSpinBox.value(), 77.8))
    {
    std::cerr << "qCTKSliderSpinBoxWidget::setTickInterval failed." 
              << sliderSpinBox.tickInterval() << " "
              << sliderSpinBox.value() << std::endl;
    return EXIT_FAILURE;
    }
  
  sliderSpinBox.setMinimum(80.5678);

  if (!qFuzzyCompare(sliderSpinBox.minimum(), 80.6) || 
      !qFuzzyCompare(sliderSpinBox.value(), 80.6))
    {
    std::cerr << "qCTKSliderSpinBoxWidget::setMinimum failed." 
              << sliderSpinBox.minimum() << " "
              << sliderSpinBox.value() << std::endl;
    return EXIT_FAILURE;
    }

  sliderSpinBox.reset();

  if (!qFuzzyCompare(sliderSpinBox.value(), 80.6))
    {
    std::cerr << "qCTKSliderSpinBoxWidget::reset failed." 
              << sliderSpinBox.value() << std::endl;
    return EXIT_FAILURE;
    }
  
  sliderSpinBox.setSpinBoxAlignment(Qt::AlignRight);

  if (sliderSpinBox.spinBoxAlignment() != Qt::AlignRight || 
      !qFuzzyCompare(sliderSpinBox.value(), 80.6))
    {
    std::cerr << "qCTKSliderSpinBoxWidget::setMinimum failed." 
              << sliderSpinBox.spinBoxAlignment() << " "
              << sliderSpinBox.value() << std::endl;
    return EXIT_FAILURE;
    }

  sliderSpinBox.setAutoSpinBoxWidth(false);

  if (sliderSpinBox.isAutoSpinBoxWidth() != false || 
      !qFuzzyCompare(sliderSpinBox.value(), 80.6))
    {
    std::cerr << "qCTKSliderSpinBoxWidget::setMinimum failed." 
              << sliderSpinBox.isAutoSpinBoxWidth() << " "
              << sliderSpinBox.value() << std::endl;
    return EXIT_FAILURE;
    }
  // FIXME check that the correct signals are sent.
  return EXIT_SUCCESS;
}

