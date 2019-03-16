/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLColorNode.h"
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkMRMLCoreTestingMacros.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

/// \brief Concrete implementation of vtkMRMLColorNode
class vtkMRMLColorNodeTestHelper1 : public vtkMRMLColorNode
{
public:
  // Provide a concrete New.
  static vtkMRMLColorNodeTestHelper1 *New();

  vtkTypeMacro(vtkMRMLColorNodeTestHelper1,vtkMRMLColorNode);

  vtkMRMLNode* CreateNodeInstance() override
    {
    return vtkMRMLColorNodeTestHelper1::New();
    }

  const char * GetTypeAsString() override
    {
    return "vtkMRMLColorNodeTestHelper1";
    }

  virtual int ReadFile()
    {
    std::cout << "vtkMRMLColorNodeTestHelper1 pretending to read a file " << std::endl;
    return EXIT_SUCCESS;
    }
  int GetNumberOfColors() override {return 1;}
  bool GetColor(int vtkNotUsed(ind), double color[4]) override
    {
    color[0] = 10;
    color[1] = 100;
    color[2] = 200;
    return true;
    }

  vtkMRMLStorageNode* CreateDefaultStorageNode() override
    {
    // just some random storage node to pass the storage node test of basic MRML node tests
    return vtkMRMLColorTableStorageNode::New();
    }
};
vtkStandardNewMacro(vtkMRMLColorNodeTestHelper1);

//---------------------------------------------------------------------------
int TestGetColorNameAsFileName();
int TestGetColorNameAsFileName(const char* colorName,
                                const char* expectedColorFileName,
                                const char* substr = "_");

//---------------------------------------------------------------------------
int vtkMRMLColorNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLColorNodeTestHelper1> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName());

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestGetColorNameAsFileName()
{
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName("validName", "validName"));
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName("name with space", "name_with_space"));
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName("n^a$m(e!w)i-t_h~v{a}l.iD#c%h'ars", "n^a$m(e!w)i-t_h~v{a}l.iD#c%h'ars"));
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName("n`a@m&e*w+i=t[h]i;n:v\\a|l\"i<D>c,h/a?rs", "n_a_m_e_w_i_t_h_i_n_v_a_l_i_D_c_h_a_rs"));
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName("name with \nreturn", "name_with__return"));
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName("ÑÂme wïth àçÇénts", "____me_w__th_________nts"));
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName(
    "very very very very very very very very very very "
    "very very very very very very very very very very "
    "very very very very very very very very very very "
    "very very very very very very very very very very "
    "very very very very very very very very very very "
    "very very very long name",
    "very_very_very_very_very_very_very_very_very_very_"
    "very_very_very_very_very_very_very_very_very_very_"
    "very_very_very_very_very_very_very_very_very_very_"
    "very_very_very_very_very_very_very_very_very_very_"
    "very_very_very_very_very_very_very_very_very_very_"
    "very_v"));
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName("name with space", "name with space", " "));
  CHECK_EXIT_SUCCESS(TestGetColorNameAsFileName("name with space", "name__with__space", "__"));
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestGetColorNameAsFileName(const char* colorName, const char * expectedColorFileName, const char* substr)
{
  vtkNew<vtkMRMLColorNodeTestHelper1> colorNode;
  colorNode->SetNamesFromColors();
  colorNode->SetColorName(0, colorName);
  std::string fileName = colorNode->GetColorNameAsFileName(0, substr);
  if (fileName != std::string(expectedColorFileName))
    {
    std::cerr << "Input: " << colorName
              << "\nOutput: " << fileName.c_str()
              << "\n Expected: " << expectedColorFileName <<std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
