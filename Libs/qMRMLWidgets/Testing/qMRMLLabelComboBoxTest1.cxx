// Qt includes
#include <QApplication>
#include <QTimer>
#include <QDebug>

// CTK includes
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLLabelComboBox.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLdGEMRICProceduralColorNode.h>
//#include <vtkMRMLFreeSurferProceduralColorNode.h>

// VTK includes
#include <vtkSmartPointer.h>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

// --------------------------------------------------------------------------
#define ctkCheckReturnValue(OBJECT, GETTER, EXPECTED_VALUE) \
  if (OBJECT.GETTER() != EXPECTED_VALUE)                    \
    {                                                       \
    std::cerr << "line " << __LINE__ << " - Problem with "  \
        << #GETTER << "()" << std::endl                     \
        << "  Current:" << OBJECT.GETTER() << std::endl     \
        << "  Expected:"<< EXPECTED_VALUE << std::endl;     \
    return EXIT_FAILURE;                                    \
    }

// --------------------------------------------------------------------------
#define ctkExerciseMethod(OBJECT, SETTER, GETTER, VALUE_TO_SET, EXPECTED_VALUE) \
  OBJECT.SETTER(VALUE_TO_SET);                                           \
  ctkCheckReturnValue(OBJECT, GETTER, EXPECTED_VALUE)

// --------------------------------------------------------------------------
int qMRMLLabelComboBoxTest1( int argc, char * argv [] )
{
  ctkLogger::configure();

  QApplication app(argc, argv);

  VTK_CREATE(vtkMRMLColorTableNode, colorTableNode);
  colorTableNode->SetType(vtkMRMLColorTableNode::Labels);
  
  qMRMLLabelComboBox labelComboBox;

  // Test1 - Check if mrmlColorNode()/setMRMLColorNode() work
  ctkCheckReturnValue(labelComboBox, mrmlColorNode, 0);
  ctkExerciseMethod(labelComboBox, setMRMLColorNode, mrmlColorNode,
                    colorTableNode, colorTableNode.GetPointer());
  labelComboBox.setMRMLColorNode(0);

  // Test2 - Check if noneEnabled()/setNoneEnabled() work
  ctkCheckReturnValue(labelComboBox, noneEnabled, false);
  ctkExerciseMethod(labelComboBox, setNoneEnabled, noneEnabled, true, true);


  labelComboBox.setNoneEnabled(false);

  // Test3 - Check if currentColor()/setCurrentColor() work properly when
  // no ColorNode has been assigned  and NoneEnabled is false
  ctkCheckReturnValue(labelComboBox, currentColor, -1);
  ctkExerciseMethod(labelComboBox, setCurrentColor, currentColor, 10, -1);

  labelComboBox.setMRMLColorNode(colorTableNode);

  // Test4 - Check if currentColor()/setCurrentColor() work properly when
  // a valid ColorNode has been assigned and NoneEnabled is false
  ctkCheckReturnValue(labelComboBox, currentColor, -1);
  ctkExerciseMethod(labelComboBox, setCurrentColor, currentColor, 5, 5);
  ctkExerciseMethod(labelComboBox, setCurrentColor, currentColor, -6, 5);
  ctkExerciseMethod(labelComboBox, setCurrentColor, currentColor, -1, 5);
  ctkExerciseMethod(labelComboBox, setCurrentColor, currentColor, 256, 256);
  ctkExerciseMethod(labelComboBox, setCurrentColor, currentColor, 257, 256);

  qMRMLLabelComboBox labelComboBox2;

  labelComboBox2.setNoneEnabled(true);

  // Test5 - Check if currentColor()/setCurrentColor() work properly when
  // no ColorNode has been assigned  and NoneEnabled is true
  ctkCheckReturnValue(labelComboBox2, currentColor, -1);
  ctkExerciseMethod(labelComboBox2, setCurrentColor, currentColor, 10, -1);

  labelComboBox2.setMRMLColorNode(colorTableNode);

  // Test6 - Check if currentColor()/setCurrentColor() work properly when
  // a valid ColorNode has been assigned and NoneEnabled is true
  ctkCheckReturnValue(labelComboBox2, currentColor, -1);
  ctkExerciseMethod(labelComboBox2, setCurrentColor, currentColor, 5, 5);
  ctkExerciseMethod(labelComboBox2, setCurrentColor, currentColor, -6, 5);
  ctkExerciseMethod(labelComboBox2, setCurrentColor, currentColor, -1, -1);
  ctkExerciseMethod(labelComboBox2, setCurrentColor, currentColor, 256, 256);
  ctkExerciseMethod(labelComboBox2, setCurrentColor, currentColor, 257, 256);

  // Show widgets
  labelComboBox.show();
  labelComboBox.printAdditionalInfo();
  labelComboBox2.show();
  labelComboBox.printAdditionalInfo();
  
  QTimer autoExit;
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QObject::connect(&autoExit, SIGNAL(timeout()), &app, SLOT(quit()));
    autoExit.start(1000);
    }

  return app.exec();
}
