/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLColorTableNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.0 $

=========================================================================auto=*/

#ifndef __vtkMRMLColorTableNode_h
#define __vtkMRMLColorTableNode_h

#include "vtkMRMLColorNode.h"

/// \brief MRML node to represent discrete color information.
///
/// Color nodes describe color look up tables. The tables may be pre-generated by
/// Slicer (the label map colors, a random one) or created by
/// a user. More than one model or label volume or editor can access the prebuilt
/// nodes.
class VTK_MRML_EXPORT vtkMRMLColorTableNode : public vtkMRMLColorNode
{
public:
  static vtkMRMLColorTableNode* New();
  vtkTypeMacro(vtkMRMLColorTableNode, vtkMRMLColorNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes(const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode* node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "ColorTable"; }

  /// Access lookup table object that stores table values.
  /// \sa SetAndObserveLookupTable()
  vtkLookupTable* GetLookupTable() override;

  /// Set lookup table object that this object will use.
  /// By default, all colors in the lookup table are set to "defined" for backward compatibility.
  /// To avoid this, set markAllColorsAsDefined to false.
  /// \sa GetLookupTable()
  virtual void SetAndObserveLookupTable(vtkLookupTable* newLookupTable, bool markAllColorsAsDefined = true);

  /// \deprecated Kept only for backward compatibility.
  /// Use SetAndObserveLookupTable method instead.
  /// \sa SetAndObserveLookupTable()
  virtual void SetLookupTable(vtkLookupTable* newLookupTable) { this->SetAndObserveLookupTable(newLookupTable, /*markAllColorsAsDefined=*/true); }

  ///
  /// Get/Set for Type
  void SetType(int type) override;
  // GetType is defined in ColorTableNode class via macro.
  void SetTypeToFullRainbow();
  void SetTypeToGrey();
  void SetTypeToIron();
  void SetTypeToRainbow();
  void SetTypeToOcean();
  void SetTypeToDesert();
  void SetTypeToInvGrey();
  void SetTypeToReverseRainbow();
  void SetTypeToFMRI();
  void SetTypeToFMRIPA();
  void SetTypeToLabels();
  void SetTypeToRandom();
  void SetTypeToRed();
  void SetTypeToGreen();
  void SetTypeToBlue();
  void SetTypeToCyan();
  void SetTypeToMagenta();
  void SetTypeToYellow();
  void SetTypeToWarm1();
  void SetTypeToWarm2();
  void SetTypeToWarm3();
  void SetTypeToCool1();
  void SetTypeToCool2();
  void SetTypeToCool3();
  void SetTypeToWarmShade1();
  void SetTypeToWarmShade2();
  void SetTypeToWarmShade3();
  void SetTypeToCoolShade1();
  void SetTypeToCoolShade2();
  void SetTypeToCoolShade3();
  void SetTypeToWarmTint1();
  void SetTypeToWarmTint2();
  void SetTypeToWarmTint3();
  void SetTypeToCoolTint1();
  void SetTypeToCoolTint2();
  void SetTypeToCoolTint3();

  void ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData) override;

  ///
  /// The list of valid table types

  /// Grey - greyscale ramp
  /// Iron - neutral
  /// Rainbow - red-orange-yellow-blue-indigo-violet
  /// Ocean - bluish ramp
  /// Desert - orange ramp
  /// InvGrey - inverted greyscale ramp
  /// ReverseRainbow - inverted Rainbow
  /// FMRI - fMRI map
  /// FMRIPA - fMRI Positive Activation map
  /// Labels - the Slicer2 FullRainbow editor labels
  /// Random - 255 random colors
  /// Red - red ramp (like greyscale but with red, meant for layering with cyan)
  /// Green - green ramp (like greyscale but with green, layering with magenta)
  /// Blue - blue ramp (like greyscale but with blue, layering with yellow)
  /// Yellow - yellow ramp (complementary ramp to blue, layering yields gray)
  /// Cyan - cyan ramp (complementary ramp to red, layering yields gray)
  /// Magenta - magenta ramp (complementary ramp to green, layering yields gray)
  /// Warm# - ramps of warm colors that are complimentary to Cool#
  /// WarmShade# - ramps of warm colors with variation in value that are
  ///       complimentary to CoolShade#
  /// WarmTint# - ramps of warm colors with variation in saturation that are
  ///       complimentary to CoolTint#
  /// Cool# - ramps of cool colors that are complimentary to Warm#
  /// CoolShade# - ramps of cool colors with variation in value that are
  ///       complimentary to WarmShade#
  /// CoolTint# - ramps of cool colors with variation in saturation that are
  ///       complimentary to WarmSTint#
  enum
  {
    FullRainbow = 0,
    Grey = 1,
    Iron = 2,
    Rainbow = 3,
    Ocean = 4,
    Desert = 5,
    InvGrey = 6,
    ReverseRainbow = 7,
    FMRI = 8,
    FMRIPA = 9,
    Labels = 10,
    Obsolete = 11,
    Random = 12,
    Red = 15,
    Green = 16,
    Blue = 17,
    Yellow = 18,
    Cyan = 19,
    Magenta = 20,
    Warm1 = 21,
    Warm2 = 22,
    Warm3 = 23,
    Cool1 = 24,
    Cool2 = 25,
    Cool3 = 26,
    WarmShade1 = 27,
    WarmShade2 = 28,
    WarmShade3 = 29,
    CoolShade1 = 30,
    CoolShade2 = 31,
    CoolShade3 = 32,
    WarmTint1 = 33,
    WarmTint2 = 34,
    WarmTint3 = 35,
    CoolTint1 = 36,
    CoolTint2 = 37,
    CoolTint3 = 38
  };

  ///
  /// Return the lowest and highest integers, for use in looping
  int GetFirstType() override { return this->FullRainbow; };
  int GetLastType() override { return this->CoolTint3; };

  ///
  /// return a text string describing the color look up table type
  const char* GetTypeAsString() override;

  ///
  /// Set the size of the color table if it's a User table
  void SetNumberOfColors(int n);

  ///
  /// Set the size of the color table if it's a User table
  int GetNumberOfColors() override;

  ///
  /// Add a color to the User color table, at the end
  void AddColor(const char* name, double r, double g, double b, double a = 1.0);

  ///
  /// Set a color into the User color table. Return 1 on success, 0 on failure.
  int SetColor(int entry, const char* name, double r, double g, double b, double a = 1.0);

  /// Undefine color entry.
  /// Returns true on success.
  bool RemoveColor(int entry);

  /// Set many entries to the same name and color in one batch (with one ModifiedEvent).
  /// This is much more efficient than setting many color entries one by one using SetColor().
  /// Note that this sets the Defined flag for the color entries to true. The \sa SetColorDefined
  /// method can be used to unset it if needed.
  int SetColors(int firstEntry, int lastEntry, const char* name, double r, double g, double b, double a = 1.0);

  /// Undefines entries in the specified range.
  /// Returns true on success, false on failure.
  bool RemoveColors(int firstEntry, int lastEntry);

  /// Set a color into the User color table. Return 1 on success, 0 on failure.
  /// Note that this sets the Defined flag for the color entry to true. The \sa SetColorDefined
  /// method can be used to unset it if needed.
  int SetColor(int entry, double r, double g, double b, double a);
  int SetColor(int entry, double r, double g, double b);
  int SetOpacity(int entry, double opacity);

  /// Retrieve the color associated to the index.
  /// Return true if the color exists, false otherwise
  bool GetColor(int entry, double color[4]) override;

  ///
  /// Clear out the names list.
  void ClearNames();

  ///
  /// Reset when close the scene.
  void Reset(vtkMRMLNode* defaultNode) override;

  ///
  /// Create default storage node or nullptr if does not have one.
  vtkMRMLStorageNode* CreateDefaultStorageNode() override;

protected:
  vtkMRMLColorTableNode();
  ~vtkMRMLColorTableNode() override;
  vtkMRMLColorTableNode(const vtkMRMLColorTableNode&);
  void operator=(const vtkMRMLColorTableNode&);

  /// Log error message and return false if not a valid color index.
  bool IsValidColorIndex(int entry, const std::string& callerMethod, bool isCallerMethodSet = false);

  ///
  /// The look up table, constructed according to the Type.
  vtkLookupTable* LookupTable;

  ///
  /// keep track of where we last added a color.
  int LastAddedColor{ -1 };
};

#endif
