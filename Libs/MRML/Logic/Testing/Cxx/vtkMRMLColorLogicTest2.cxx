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
// MRML includes
#include "vtkMRMLCoreTestingUtilities.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// MRMLLogic includes
#include "vtkMRMLColorLogic.h"

using namespace vtkMRMLCoreTestingUtilities;

namespace
{

//----------------------------------------------------------------------------
class vtkMRMLTestColorLogic : public vtkMRMLColorLogic
{
public:
  static vtkMRMLTestColorLogic *New();
  typedef vtkMRMLTestColorLogic Self;

  vtkTypeMacro(vtkMRMLTestColorLogic, vtkMRMLColorLogic);

  std::string test_RemoveLeadAndTrailSpaces(std::string str)
    {
    return this->RemoveLeadAndTrailSpaces(str);
    }

  bool test_ParseTerm(std::string str, StandardTerm& term)
    {
    return this->ParseTerm(str, term);
    }

protected:
vtkMRMLTestColorLogic()
{
}
};

vtkStandardNewMacro(vtkMRMLTestColorLogic);
}

//----------------------------------------------------------------------------
int vtkMRMLColorLogicTest2(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{

  vtkNew<vtkMRMLTestColorLogic> colorLogic;

  //----------------------------------------------------------------------------
  // RemoveLeadAndTrailSpaces
  //----------------------------------------------------------------------------
  if (!CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                   colorLogic->test_RemoveLeadAndTrailSpaces("").c_str(),
                   ""))
    {
    return EXIT_FAILURE;
    }
  if (!CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                   colorLogic->test_RemoveLeadAndTrailSpaces(" ").c_str(),
                   ""))
    {
    return EXIT_FAILURE;
    }
  if (!CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                   colorLogic->test_RemoveLeadAndTrailSpaces("  ").c_str(),
                   ""))
    {
    return EXIT_FAILURE;
    }
  if (!CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                   colorLogic->test_RemoveLeadAndTrailSpaces("   ").c_str(),
                   ""))
    {
    return EXIT_FAILURE;
    }
  if (!CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                   colorLogic->test_RemoveLeadAndTrailSpaces("1").c_str(),
                   "1"))
    {
    return EXIT_FAILURE;
    }
  if (!CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                   colorLogic->test_RemoveLeadAndTrailSpaces("a").c_str(),
                   "a"))
    {
    return EXIT_FAILURE;
    }
  if (!CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                   colorLogic->test_RemoveLeadAndTrailSpaces(" a").c_str(),
                   "a"))
    {
    return EXIT_FAILURE;
    }
  if (!CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                   colorLogic->test_RemoveLeadAndTrailSpaces("a ").c_str(),
                   "a"))
    {
    return EXIT_FAILURE;
    }
  if (!CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                   colorLogic->test_RemoveLeadAndTrailSpaces("  testing string 1  ").c_str(),
                   "testing string 1"))
    {
    return EXIT_FAILURE;
    }
  std::string sampleStringIn1 = "(T-D0050;SRT;Tissue)";
  std::string sampleStringIn2 = " (T-D0050;SRT;Tissue) ";
  std::string sampleStringIn3 = "(T-D0050;SRT;Tissue) ";
  std::string sampleStringIn4 = " (T-D0050;SRT;Tissue)";
  std::string sampleStringOut = "(T-D0050;SRT;Tissue)";
  if (!CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                   colorLogic->test_RemoveLeadAndTrailSpaces(sampleStringIn1).c_str(),
                   sampleStringOut.c_str()))
    {
    return EXIT_FAILURE;
    }
  if (!CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                   colorLogic->test_RemoveLeadAndTrailSpaces(sampleStringIn2).c_str(),
                   sampleStringOut.c_str()))
    {
    return EXIT_FAILURE;
    }
  if (!CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                   colorLogic->test_RemoveLeadAndTrailSpaces(sampleStringIn3).c_str(),
                   sampleStringOut.c_str()))
    {
    return EXIT_FAILURE;
    }
  if (!CheckString(__LINE__, "test_RemoveLeadAndTrailSpaces",
                   colorLogic->test_RemoveLeadAndTrailSpaces(sampleStringIn4).c_str(),
                   sampleStringOut.c_str()))
    {
    return EXIT_FAILURE;
    }

  //----------------------------------------------------------------------------
  // ParseTerm
  //----------------------------------------------------------------------------
  std::string str = "(T-D0050;SRT;Tissue)";
  vtkMRMLColorLogic::StandardTerm term;
  if (!colorLogic->test_ParseTerm(str, term))
    {
    std::cerr << __LINE__
              << "test_ParseTerm: failed to parse string "
              << str
              << std::endl;
    return EXIT_FAILURE;
    }
  // the parsing shouldn't change the input string
  if (!CheckString(__LINE__, "test_ParseTerm empty string",
                   str.c_str(), "(T-D0050;SRT;Tissue)"))
    {
    return EXIT_FAILURE;
    }
  str = "(M-01000;SRT;Morphologically Altered Structure)";
  if (!colorLogic->test_ParseTerm(str, term))
    {
    std::cerr << __LINE__
              << "test_ParseTerm: failed to parse string terminology"
              << str
              << std::endl;
    return EXIT_FAILURE;
    }
  if (!CheckString(__LINE__, "test_ParseTerm code value",
                   term.CodeValue.c_str(), "M-01000"))
    {
    return EXIT_FAILURE;
    }
  if (!CheckString(__LINE__, "test_ParseTerm coding scheme designator",
                   term.CodingSchemeDesignator.c_str(), "SRT"))
    {
    return EXIT_FAILURE;
    }
  if (!CheckString(__LINE__, "test_ParseTerm meaning",
                   term.CodeMeaning.c_str(), "Morphologically Altered Structure"))
    {
    return EXIT_FAILURE;
    }

  // invalid strings
  str = "";
  if (colorLogic->test_ParseTerm(str, term))
    {
    std::cerr << __LINE__
              << "test_ParseTerm: failed to not parse too short invalid string "
              << str
              << std::endl;
    return EXIT_FAILURE;
    }
  str = "(M;S;B)";
  if (colorLogic->test_ParseTerm(str, term))
    {
    std::cerr << __LINE__
              << "test_ParseTerm: failed to not parse too short invalid string "
              << str
              << std::endl;
    return EXIT_FAILURE;
    }

  str = "M-01000;SRT;Morphologically Altered Structure";
  if (colorLogic->test_ParseTerm(str, term))
    {
    std::cerr << __LINE__
              << "test_ParseTerm: failed to not parse invalid string "
              << str
              << std::endl;
    return EXIT_FAILURE;
    }

  str = "(M-01000;SRT Morphologically Altered Structure)";
  if (colorLogic->test_ParseTerm(str, term))
    {
    std::cerr << __LINE__
              << "test_ParseTerm: failed to not parse invalid string "
              << str
              << std::endl;
    return EXIT_FAILURE;
    }
  str = "(M-01000 SRT;Morphologically Altered Structure)";
  if (colorLogic->test_ParseTerm(str, term))
    {
    std::cerr << __LINE__
              << "test_ParseTerm: failed to not parse invalid string "
              << str
              << std::endl;
    return EXIT_FAILURE;
    }
  str = "(M-01000 SRT Morphologically Altered Structure)";
  if (colorLogic->test_ParseTerm(str, term))
    {
    std::cerr << __LINE__
              << "test_ParseTerm: failed to not parse invalid string "
              << str
              << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
