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

// MRMLLogic includes
#include "vtkSlicerColorLogic.h"

// MRML includes
#include "vtkMRMLCoreTestingUtilities.h"
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkTimerLog.h>

// STD includes

#include "vtkMRMLCoreTestingMacros.h"

using namespace vtkMRMLCoreTestingUtilities;

//----------------------------------------------------------------------------
namespace
{
  bool TestDefaults();
  bool TestTerminology();
}

int vtkSlicerColorLogicTest1(int vtkNotUsed(argc), char * vtkNotUsed(argv)[])
{
  bool res = true;
  res = TestDefaults() && res;
  res = TestTerminology() && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}
namespace
{

//----------------------------------------------------------------------------
bool TestDefaults()
{
  // To load the freesurfer file, SLICER_HOME is requested
  //vtksys::SystemTools::PutEnv("SLICER_HOME=..." );
  vtkNew<vtkMRMLScene> scene;
  vtkSlicerColorLogic* colorLogic = vtkSlicerColorLogic::New();

  vtkSmartPointer<vtkTimerLog> overallTimer = vtkSmartPointer<vtkTimerLog>::New();
  overallTimer->StartTimer();

  colorLogic->SetMRMLScene(scene.GetPointer());

  overallTimer->StopTimer();
  std::cout << "AddDefaultColorNodes: " << overallTimer->GetElapsedTime() << "s"
            << " " << 1. / overallTimer->GetElapsedTime() << "fps" << std::endl;
  overallTimer->StartTimer();

  colorLogic->Delete();

  std::cout << "RemoveDefaultColorNodes: " << overallTimer->GetElapsedTime() << "s"
            << " " << 1. / overallTimer->GetElapsedTime() << "fps" << std::endl;

  return true;
}

//----------------------------------------------------------------------------
// Do the terminology testing here since the Colors module populates the terminology
// file list for loading.
bool TestTerminology()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerColorLogic> colorLogic;
  // setting the scene adds the default color nodes
  colorLogic->SetMRMLScene(scene.GetPointer());

  std::cout << "Testing terminology:" << std::endl;

  std::string lutName = "GenericAnatomyColors";
  if (!colorLogic->TerminologyExists(lutName))
    {
    std::cerr << "Line " << __LINE__
              << ": adding default colors failed to add the terminology for "
              << lutName << std::endl;
    return false;
    }

  std::cout << "Testing PrintCategorizationFromLabel:" << std::endl;
  colorLogic->PrintCategorizationFromLabel(1, lutName.c_str());

  std::cout << "Testing LookupCategorizationFromLabel:" << std::endl;
  vtkMRMLColorLogic::ColorLabelCategorization labelCat;
  if (!colorLogic->LookupCategorizationFromLabel(1, labelCat, lutName.c_str()))
    {
    std::cerr << "Line " << __LINE__
              << ": failed to look up terminology categorization from label 1 for "
              << lutName << std::endl;
    return false;
    }
  labelCat.Print(std::cout);
  if (labelCat.SegmentedPropertyType.CodeMeaning.length() == 0 ||
      labelCat.SegmentedPropertyType.CodeMeaning.compare("Tissue") != 0)
    {
    std::cerr << "Line " << __LINE__
              << ": for label 1, code meaning does not match 'Tissue': '"
              <<  labelCat.SegmentedPropertyType.CodeMeaning.c_str()
              << "'" << std::endl;
    return false;
    }

  // create a new terminology
  std::string pelvisLUTName = "PelvisColor";
  std::cout << "Creating a terminology for lut named " << pelvisLUTName << std::endl;
  std::string regionValue = "T-02480";
  std::string regionMeaning = "Skin of abdomen";
  std::string regionScheme= "SRT";
  std::string catValue = "T-D0050";
  std::string catMeaning = "Tissue";
  std::string catScheme = "SRT";
  std::string typeValue = "T-01000";
  std::string typeMeaning = "Skin";
  std::string typeScheme = "SRT";
  std::string regionModMeaning, regionModValue, regionModScheme, modMeaning, modValue, modScheme;
  std::string sep = std::string(":");
  std::string region = regionValue + sep + regionScheme + sep + regionMeaning;
  std::string category = catValue + sep + catScheme + sep + catMeaning;
  std::string segmentedPropertyType = typeValue + sep + typeScheme + sep + typeMeaning;

  std::cout << "Testing AddTermToTerminology:" << std::endl;
  if (!colorLogic->AddTermToTerminology(pelvisLUTName, 1,
                                        catValue, catScheme, catMeaning,
                                        typeValue, typeScheme, typeMeaning,
                                        modMeaning, modScheme, modValue,
                                        regionValue, regionScheme, regionMeaning,
                                        regionModValue, regionModScheme, regionModMeaning))
    {
    std::cerr << "Line " << __LINE__
              << ": failed to add a new terminology for "
              << pelvisLUTName
              << std::endl;
    return false;
    }
  // check that both terminologies still exist
  if (!colorLogic->TerminologyExists(lutName) ||
      !colorLogic->TerminologyExists(pelvisLUTName))
    {
    std::cerr << "Line " << __LINE__
              << ": after adding a new terminology for "
              << pelvisLUTName
              << ", lost one or both terminologies for it and "
              << lutName
              << std::endl;
    return false;
    }

  // get the label categorisation
  vtkMRMLColorLogic::ColorLabelCategorization pelvisLabelCat;
  std::cout << "Testing new terminology LookupCategorizationFromLabel:" << std::endl;
  if (!colorLogic->LookupCategorizationFromLabel(1, pelvisLabelCat, pelvisLUTName.c_str()))
    {
    std::cerr << "Line " << __LINE__
              << ": failed on trying to look up terminology categorization "
              << " for label 1 for "
              << pelvisLUTName << std::endl;
    return false;
    }
  pelvisLabelCat.Print(std::cout);
  // check the strings
  if (!CheckString(__LINE__, "AnatomicRegion.CodeMeaning",
                   pelvisLabelCat.AnatomicRegion.CodeMeaning.c_str(),
                   regionMeaning.c_str())
      || !CheckString(__LINE__, "AnatomicRegion.CodeValue",
                      pelvisLabelCat.AnatomicRegion.CodeValue.c_str(),
                      regionValue.c_str())
      || !CheckString(__LINE__, "AnatomicRegion.CodingSchemeDesignator",
                      pelvisLabelCat.AnatomicRegion.CodingSchemeDesignator.c_str(),
                      regionScheme.c_str())

      || !CheckString(__LINE__, "AnatomicRegionModifier.CodeMeaning.",
                      pelvisLabelCat.AnatomicRegionModifier.CodeMeaning.c_str(),
                      regionModMeaning.c_str())
      || !CheckString(__LINE__, "AnatomicRegionModifier.CodeValue",
                      pelvisLabelCat.AnatomicRegionModifier.CodeValue.c_str(),
                      regionModValue.c_str())
      || !CheckString(__LINE__, "AnatomicRegionModifier.CodingSchemeDesignator",
                      pelvisLabelCat.AnatomicRegionModifier.CodingSchemeDesignator.c_str(),
                      regionModScheme.c_str())

      || !CheckString(__LINE__, "SegmentedPropertyCategory.CodeMeaning",
                      pelvisLabelCat.SegmentedPropertyCategory.CodeMeaning.c_str(),
                      catMeaning.c_str())
      || !CheckString(__LINE__, "SegmentedPropertyCategory.CodeValue",
                      pelvisLabelCat.SegmentedPropertyCategory.CodeValue.c_str(),
                      catValue.c_str())
      || !CheckString(__LINE__, "SegmentedPropertyCategory.CodingSchemeDesignator",
                      pelvisLabelCat.SegmentedPropertyCategory.CodingSchemeDesignator.c_str(),
                      catScheme.c_str())

      || !CheckString(__LINE__, "SegmentedPropertyType.CodeMeaning",
                      pelvisLabelCat.SegmentedPropertyType.CodeMeaning.c_str(),
                      typeMeaning.c_str())
      || !CheckString(__LINE__, "SegmentedPropertyType.CodeValue",
                      pelvisLabelCat.SegmentedPropertyType.CodeValue.c_str(),
                      typeValue.c_str())
      || !CheckString(__LINE__, "SegmentedPropertyType.CodingSchemeDesignator",
                      pelvisLabelCat.SegmentedPropertyType.CodingSchemeDesignator.c_str(),
                      typeScheme.c_str())

      || !CheckString(__LINE__, "SegmentedPropertyTypeModifier.CodeMeaning",
                      pelvisLabelCat.SegmentedPropertyTypeModifier.CodeMeaning.c_str(),
                      modMeaning.c_str())
      || !CheckString(__LINE__, "SegmentedPropertyTypeModifier.CodeValue",
                   pelvisLabelCat.SegmentedPropertyTypeModifier.CodeValue.c_str(),
                   modValue.c_str())
      || !CheckString(__LINE__, "SegmentedPropertyTypeModifier.CodingSchemeDesignator",
                      pelvisLabelCat.SegmentedPropertyTypeModifier.CodingSchemeDesignator.c_str(),
                      modScheme.c_str()))
    {
      return false;
    }

  // check the utility methods to access the values
  if (!CheckString(__LINE__, "GetAnatomicRegionCodeMeaning",
                   regionMeaning.c_str(),
                   colorLogic->GetAnatomicRegionCodeMeaning(1, pelvisLUTName.c_str()).c_str())
      || !CheckString(__LINE__, "GetAnatomicRegionCodeValue",
                      regionValue.c_str(),
                      colorLogic->GetAnatomicRegionCodeValue(1, pelvisLUTName.c_str()).c_str())
      || !CheckString(__LINE__, "GetAnatomicRegionCodingSchemeDesignator",
                      regionScheme.c_str(),
                      colorLogic->GetAnatomicRegionCodingSchemeDesignator(1, pelvisLUTName.c_str()).c_str())

      || !CheckString(__LINE__, "GetAnatomicRegionModifierCodeMeaning",
                      regionModMeaning.c_str(),
                      colorLogic->GetAnatomicRegionModifierCodeMeaning(1, pelvisLUTName.c_str()).c_str())
      || !CheckString(__LINE__, "GetAnatomicRegionModifierCodeValue",
                      regionModValue.c_str(),
                      colorLogic->GetAnatomicRegionModifierCodeValue(1, pelvisLUTName.c_str()).c_str())
      || !CheckString(__LINE__, "GetAnatomicRegionModifierCodingSchemeDesignator",
                      regionModScheme.c_str(),
                      colorLogic->GetAnatomicRegionModifierCodingSchemeDesignator(1, pelvisLUTName.c_str()).c_str())

      || !CheckString(__LINE__, "GetSegmentedPropertyCategoryCodeMeaning",
                      catMeaning.c_str(),
                      colorLogic->GetSegmentedPropertyCategoryCodeMeaning(1, pelvisLUTName.c_str()).c_str())
      || !CheckString(__LINE__, "GetSegmentedPropertyCategoryCodeValue",
                      catValue.c_str(),
                      colorLogic->GetSegmentedPropertyCategoryCodeValue(1, pelvisLUTName.c_str()).c_str())
      || !CheckString(__LINE__, "GetSegmentedPropertyCategoryCodingSchemeDesignator",
                      catScheme.c_str(),
                      colorLogic->GetSegmentedPropertyCategoryCodingSchemeDesignator(1, pelvisLUTName.c_str()).c_str())

      || !CheckString(__LINE__, "GetSegmentedPropertyTypeCodeMeaning",
                      typeMeaning.c_str(),
                      colorLogic->GetSegmentedPropertyTypeCodeMeaning(1, pelvisLUTName.c_str()).c_str())
      || !CheckString(__LINE__, "GetSegmentedPropertyTypeCodeValue",
                      typeValue.c_str(),
                      colorLogic->GetSegmentedPropertyTypeCodeValue(1, pelvisLUTName.c_str()).c_str())
      || !CheckString(__LINE__, "GetSegmentedPropertyTypeCodingSchemeDesignator",
                      typeScheme.c_str(),
                      colorLogic->GetSegmentedPropertyTypeCodingSchemeDesignator(1, pelvisLUTName.c_str()).c_str())

      || !CheckString(__LINE__, "GetSegmentedPropertyTypeModifierCodeMeaning",
                      modMeaning.c_str(),
                      colorLogic->GetSegmentedPropertyTypeModifierCodeMeaning(1, pelvisLUTName.c_str()).c_str())
      || !CheckString(__LINE__, "GetSegmentedPropertyTypeModifierCodeValue",
                      modValue.c_str(),
                      colorLogic->GetSegmentedPropertyTypeModifierCodeValue(1, pelvisLUTName.c_str()).c_str())
      || !CheckString(__LINE__, "GetSegmentedPropertyTypeModifierCodingSchemeDesignator",
                      modScheme.c_str(),
                      colorLogic->GetSegmentedPropertyTypeModifierCodingSchemeDesignator(1, pelvisLUTName.c_str()).c_str()))
    {
    return false;
    }

  // check the utility method to get the concatenated strings
  if (!CheckString(__LINE__, "GetSegmentedPropertyCategory",
                   category.c_str(),
                   colorLogic->GetSegmentedPropertyCategory(1, pelvisLUTName.c_str()).c_str())
      || !CheckString(__LINE__, "GetSegmentedPropertyType",
                      segmentedPropertyType.c_str(),
                      colorLogic->GetSegmentedPropertyType(1, pelvisLUTName.c_str()).c_str())
      || !CheckString(__LINE__, "GetAnatomicRegion",
                      region.c_str(),
                      colorLogic->GetAnatomicRegion(1, pelvisLUTName.c_str()).c_str()))
    {
    return false;
    }
  // check the utility methods getting concatenated terminology strings where the values haven't been set
  std::string regionMods = colorLogic->GetAnatomicRegionModifier(1, pelvisLUTName.c_str());
  std::string typeMods = colorLogic->GetSegmentedPropertyTypeModifier(1, pelvisLUTName.c_str());
  if (!CheckString(__LINE__, "Empty Region modifier",
                   regionMods.c_str(), "")
      || !CheckString(__LINE__, "Empty type modifier",
                      typeMods.c_str(), ""))
    {
    return false;
    }

  // check the method to get a terminology via strings
  std::string termValue = colorLogic->GetTerminologyFromLabel("AnatomicRegion", "CodeMeaning", 1, pelvisLUTName.c_str());
  if (!CheckString(__LINE__, "TestTerminology string accessors",
                   regionMeaning.c_str(), termValue.c_str()))
    {
    return false;
    }

  // look for a label that doesn't have a terminology
  std::cout << "Testing new terminology for known missing label:" << std::endl;
  vtkMRMLColorLogic::ColorLabelCategorization missingPelvisLabelCat;
  if (colorLogic->LookupCategorizationFromLabel(100, missingPelvisLabelCat, pelvisLUTName.c_str()))
    {
    std::cerr << "Line " << __LINE__
              << ": failed on trying to look up missing terminology categorization "
              << " from label 100 for "
              << pelvisLUTName << std::endl;
    return false;
    }
  missingPelvisLabelCat.Print(std::cout);
  // check all are empty strings
  if (!CheckString(__LINE__, "AnatomicRegion.CodeMeaning",
                   missingPelvisLabelCat.AnatomicRegion.CodeMeaning.c_str(), "")
      || !CheckString(__LINE__, "AnatomicRegion.CodeValue",
                      missingPelvisLabelCat.AnatomicRegion.CodeValue.c_str(), "")
      || !CheckString(__LINE__, "AnatomicRegion.CodingSchemeDesignator",
                      missingPelvisLabelCat.AnatomicRegion.CodingSchemeDesignator.c_str(), "")

      || !CheckString(__LINE__, "AnatomicRegionModifier.CodeMeaning",
                      missingPelvisLabelCat.AnatomicRegionModifier.CodeMeaning.c_str(), "")
      || !CheckString(__LINE__, "AnatomicRegionModifier.CodeValue",
                      missingPelvisLabelCat.AnatomicRegionModifier.CodeValue.c_str(), "")
      || !CheckString(__LINE__, "AnatomicRegionModifier.CodingSchemeDesignator",
                      missingPelvisLabelCat.AnatomicRegionModifier.CodingSchemeDesignator.c_str(), "")

      || !CheckString(__LINE__, "SegmentedPropertyCategory.CodeMeaning",
                      missingPelvisLabelCat.SegmentedPropertyCategory.CodeMeaning.c_str(), "")
      || !CheckString(__LINE__, "SegmentedPropertyCategory.CodeValue",
                      missingPelvisLabelCat.SegmentedPropertyCategory.CodeValue.c_str(), "")
      || !CheckString(__LINE__, "SegmentedPropertyCategory.CodingSchemeDesignator",
                      missingPelvisLabelCat.SegmentedPropertyCategory.CodingSchemeDesignator.c_str(), "")

      || !CheckString(__LINE__, "SegmentedPropertyType.CodeMeaning",
                      missingPelvisLabelCat.SegmentedPropertyType.CodeMeaning.c_str(), "")
      || !CheckString(__LINE__, "SegmentedPropertyType.CodeValue",
                      missingPelvisLabelCat.SegmentedPropertyType.CodeValue.c_str(), "")
      || !CheckString(__LINE__, "SegmentedPropertyType.CodingSchemeDesignator",
                      missingPelvisLabelCat.SegmentedPropertyType.CodingSchemeDesignator.c_str(), "")

      || !CheckString(__LINE__, "SegmentedPropertyTypeModifier.CodeMeaning",
                      missingPelvisLabelCat.SegmentedPropertyTypeModifier.CodeMeaning.c_str(), "")
      || !CheckString(__LINE__, "SegmentedPropertyTypeModifier.CodeValue",
                      missingPelvisLabelCat.SegmentedPropertyTypeModifier.CodeValue.c_str(), "")
      || !CheckString(__LINE__, "SegmentedPropertyTypeModifier.CodingSchemeDesignator",
                      missingPelvisLabelCat.SegmentedPropertyTypeModifier.CodingSchemeDesignator.c_str(), ""))
    {
    return false;
    }

  return true;
}

}
