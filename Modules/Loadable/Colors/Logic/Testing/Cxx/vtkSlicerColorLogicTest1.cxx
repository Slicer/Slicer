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
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkTimerLog.h>

// STD includes

#include "vtkMRMLCoreTestingMacros.h"

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

  colorLogic->SetDebug(1);

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
  if (pelvisLabelCat.AnatomicRegion.CodeMeaning.compare(regionMeaning) &&
    pelvisLabelCat.AnatomicRegion.CodeValue.compare(regionValue) &&
    pelvisLabelCat.AnatomicRegion.CodingSchemeDesignator.compare(regionScheme) &&
    pelvisLabelCat.AnatomicRegionModifier.CodeMeaning.compare(regionModMeaning) &&
    pelvisLabelCat.AnatomicRegionModifier.CodeValue.compare(regionModValue) &&
    pelvisLabelCat.AnatomicRegionModifier.CodingSchemeDesignator.compare(regionModScheme) &&
    pelvisLabelCat.SegmentedPropertyCategory.CodeMeaning.compare(catMeaning) &&
    pelvisLabelCat.SegmentedPropertyCategory.CodeValue.compare(catValue) &&
    pelvisLabelCat.SegmentedPropertyCategory.CodingSchemeDesignator.compare(catScheme) &&
    pelvisLabelCat.SegmentedPropertyType.CodeMeaning.compare(typeMeaning) &&
    pelvisLabelCat.SegmentedPropertyType.CodeValue.compare(typeValue) &&
    pelvisLabelCat.SegmentedPropertyType.CodingSchemeDesignator.compare(typeScheme) &&
    pelvisLabelCat.SegmentedPropertyTypeModifier.CodeMeaning.compare(modMeaning) &&
    pelvisLabelCat.SegmentedPropertyTypeModifier.CodeValue.compare(modValue) &&
    pelvisLabelCat.SegmentedPropertyTypeModifier.CodingSchemeDesignator.compare(modScheme))
    {
    std::cerr << "Line " << __LINE__
              << ": for label 1, LUT "
              << pelvisLUTName
              << ", failed comparing terminology with what they were set to."
              << std::endl;
    return false;
    }
  // check the utility methods to access the values
  if (regionMeaning.compare(colorLogic->GetAnatomicRegionCodeMeaning(1, pelvisLUTName.c_str())) &&
      regionValue.compare(colorLogic->GetAnatomicRegionCodeValue(1, pelvisLUTName.c_str())) &&
      regionScheme.compare(colorLogic->GetAnatomicRegionCodingSchemeDesignator(1, pelvisLUTName.c_str())) &&
      regionModMeaning.compare(colorLogic->GetAnatomicRegionModifierCodeMeaning(1, pelvisLUTName.c_str())) &&
      regionModValue.compare(colorLogic->GetAnatomicRegionModifierCodeValue(1, pelvisLUTName.c_str())) &&
      regionModScheme.compare(colorLogic->GetAnatomicRegionModifierCodingSchemeDesignator(1, pelvisLUTName.c_str())) &&
      catMeaning.compare(colorLogic->GetSegmentedPropertyCategoryCodeMeaning(1, pelvisLUTName.c_str())) &&
      catValue.compare(colorLogic->GetSegmentedPropertyCategoryCodeValue(1, pelvisLUTName.c_str())) &&
      catScheme.compare(colorLogic->GetSegmentedPropertyCategoryCodingSchemeDesignator(1, pelvisLUTName.c_str())) &&
      typeMeaning.compare(colorLogic->GetSegmentedPropertyTypeCodeMeaning(1, pelvisLUTName.c_str())) &&
      typeValue.compare(colorLogic->GetSegmentedPropertyTypeCodeValue(1, pelvisLUTName.c_str())) &&
      typeScheme.compare(colorLogic->GetSegmentedPropertyTypeCodingSchemeDesignator(1, pelvisLUTName.c_str())) &&
      modMeaning.compare(colorLogic->GetSegmentedPropertyTypeModifierCodeMeaning(1, pelvisLUTName.c_str())) &&
      modValue.compare(colorLogic->GetSegmentedPropertyTypeModifierCodeValue(1, pelvisLUTName.c_str())) &&
      modScheme.compare(colorLogic->GetSegmentedPropertyTypeModifierCodingSchemeDesignator(1, pelvisLUTName.c_str())))
    {
      std::cerr << "Line " << __LINE__
              << ": for label 1, LUT "
              << pelvisLUTName
              << ", failed comparing terminology accessors with what they were set to."
              << std::endl;
    return false;
    }

  // check the utility method to get the concatenated strings
  if (category.compare(colorLogic->GetSegmentedPropertyCategory(1, pelvisLUTName.c_str())) &&
      segmentedPropertyType.compare(colorLogic->GetSegmentedPropertyType(1, pelvisLUTName.c_str())) &&
      region.compare(colorLogic->GetAnatomicRegion(1, pelvisLUTName.c_str()))
      )
    {
    std::cerr << "Line " << __LINE__
              << ": for label 1, LUT "
              << pelvisLUTName
              << ", failed comparing terminology concatenated accessors with what they were set to."
              << std::endl;
    return false;
    }
  // check the utility methods getting concatenated terminology strings where the values haven't been set
  std::string regionMods = colorLogic->GetAnatomicRegionModifier(1, pelvisLUTName.c_str());
  std::string typeMods = colorLogic->GetSegmentedPropertyTypeModifier(1, pelvisLUTName.c_str());
  if (regionMods.compare("") &&
      typeMods.compare(""))
    {
    std::cerr << "Line " << __LINE__
              << ": for label 1, LUT "
              << pelvisLUTName
              << ", failed comparing terminology concatenated accessors when expecting empty strings."
              << "\n\tregion modifier = '" << regionMods.c_str()
              << "'\n\ttype modifier = '" << typeMods.c_str() << "'"
              << std::endl;
    return false;
    }

  // check the method to get a terminology via strings
  std::string termValue = colorLogic->GetTerminologyFromLabel("AnatomicRegion", "CodeMeaning", 1, pelvisLUTName.c_str());
  if (regionMeaning.compare(termValue) != 0)
    {
    std::cerr << "Line " << __LINE__
              << ": for label 1, LUT "
              << pelvisLUTName
              << ", failed comparing terminology string accessors, expected "
              << regionMeaning.c_str()
              << " but got "
              << termValue.c_str()
              << std::endl;
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
  if (missingPelvisLabelCat.AnatomicRegion.CodeMeaning.length() ||
    missingPelvisLabelCat.AnatomicRegion.CodeValue.length() ||
    missingPelvisLabelCat.AnatomicRegion.CodingSchemeDesignator.length() ||
    missingPelvisLabelCat.AnatomicRegionModifier.CodeMeaning.length() ||
    missingPelvisLabelCat.AnatomicRegionModifier.CodeValue.length() ||
    missingPelvisLabelCat.AnatomicRegionModifier.CodingSchemeDesignator.length() ||
    missingPelvisLabelCat.SegmentedPropertyCategory.CodeMeaning.length() ||
    missingPelvisLabelCat.SegmentedPropertyCategory.CodeValue.length() ||
    missingPelvisLabelCat.SegmentedPropertyCategory.CodingSchemeDesignator.length() ||
    missingPelvisLabelCat.SegmentedPropertyType.CodeMeaning.length() ||
    missingPelvisLabelCat.SegmentedPropertyType.CodeValue.length() ||
    missingPelvisLabelCat.SegmentedPropertyType.CodingSchemeDesignator.length() ||
    missingPelvisLabelCat.SegmentedPropertyTypeModifier.CodeMeaning.length() ||
    missingPelvisLabelCat.SegmentedPropertyTypeModifier.CodeValue.length() ||
    missingPelvisLabelCat.SegmentedPropertyTypeModifier.CodingSchemeDesignator.length())
    {
    std::cerr << "Line " << __LINE__
              << ": failed on trying to look up missing terminology categorization "
              << " from label 100 for "
              << pelvisLUTName << ", have some non empty strings"
              << std::endl;
    return false;
    }

  return true;
}

}
