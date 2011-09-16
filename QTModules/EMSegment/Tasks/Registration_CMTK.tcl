# This function will be called by the wizard and expects a path or "" in the case CMTK is not installed
proc Get_CMTK_Installation_Path { } {
    set REGISTRATION_PACKAGE_FOLDER [Get_Installation_Path "CMTK4Slicer" "registration"]
    return $REGISTRATION_PACKAGE_FOLDER
}


proc CMTKGetPixelTypeFromVolumeNode { volumeNode } {

    set referenceVolume [$volumeNode GetImageData]
    set scalarType [$referenceVolume GetScalarTypeAsString]
    switch -exact "$scalarType" {
        "char"           { set PIXELTYPE "--char" }
        "unsigned char"  { set PIXELTYPE "--byte" }
        "short"          { set PIXELTYPE "--short" }
        "unsigned short" { set PIXELTYPE "--ushort" }
        "int"            { set PIXELTYPE "--int" }
        "unsigned int"   { set PIXELTYPE "--uint" }
        "float"          { set PIXELTYPE "--float" }
        "double"         { set PIXELTYPE "--double" }
        default {
            PrintError "CMTKGetPixelTypeFromVolumeNode: Cannot handle a volume of type $scalarType"
            set PIXELTYPE "INVALID"
        }
    }
    return $PIXELTYPE
}

# ----------------------------------------------------------------------------
proc CMTKResampleCLI { inputVolumeNode referenceVolumeNode outVolumeNode transformDirName interpolationType backgroundLevel } {
    variable SCENE
    variable LOGIC
    variable REGISTRATION_PACKAGE_FOLDER

    $LOGIC PrintText "TCL: =========================================="
    $LOGIC PrintText "TCL: == Resample Image CLI : CMTKResampleCLI "
    $LOGIC PrintText "TCL: =========================================="

    set CMD "$REGISTRATION_PACKAGE_FOLDER/reformatx"


    set outVolumeFileName [CreateTemporaryFileNameForNode $outVolumeNode]
    if { $outVolumeFileName == "" } { return 1 }

    set inputVolumeFileName [WriteDataToTemporaryDir $inputVolumeNode Volume]
    if { $inputVolumeFileName == "" } { return 1 }

    set referenceVolumeFileName [WriteDataToTemporaryDir $referenceVolumeNode Volume]
    if { $referenceVolumeFileName == "" } { return 1 }


    set CMD "$CMD --pad-out $backgroundLevel"
    set CMD "$CMD --outfile \"$outVolumeFileName\""
    set CMD "$CMD --floating \"$inputVolumeFileName\"  --interpolation"

    # Naming convention is based on BRAINSResample
    switch -exact  "$interpolationType" {
        "NearestNeighbor"  { set CMD "$CMD nn" }
        "Linear"           { set CMD "$CMD linear" }
        "BSpline"          { set CMD "$CMD cubic" }
        "WindowedSinc"     { set CMD "$CMD sinc-cosine" }
        "PartialVolume"    { set CMD "$CMD pv" }
        "SincHamming"      { set CMD "$CMD sinc-hamming" }
        default {
            PrintError "CMTKResampleCLI: interpolation of type $interpolationType is unknown"
            return 1
        }
    }

    set CMD "$CMD [CMTKGetPixelTypeFromVolumeNode $referenceVolumeNode]"

    # - no attributes after this line that start with the flag ---
    set CMD "$CMD \"$referenceVolumeFileName\""
    set CMD "$CMD \"$transformDirName\""

    $LOGIC PrintText "TCL: Executing $CMD"
    catch { eval exec $CMD } errmsg
    $LOGIC PrintText "TCL: $errmsg"


    # Write results back to scene
    # This does not work $::slicer3::ApplicationLogic RequestReadData [$outVolumeNode GetID] $outVolumeFileName 0 1
    ReadDataFromDisk $outVolumeNode $outVolumeFileName Volume

    # clean up
    file delete -force $outVolumeFileName
    file delete -force $inputVolumeFileName
    file delete -force $referenceVolumeFileName

    return 0
}

# ----------------------------------------------------------------------------
proc CMTKRegistration { fixedVolumeNode movingVolumeNode outVolumeNode backgroundLevel deformableType affineType} {
    variable SCENE
    variable LOGIC
    variable REGISTRATION_PACKAGE_FOLDER
    variable mrmlManager

    # Do not get rid of debug mode variable - it is sometimes very helpful !
    set CMTK_DEBUG_MODE 0

    if { $CMTK_DEBUG_MODE } {
        $LOGIC PrintText ""
        $LOGIC PrintText "DEBUG: ==========CMTKRegistration DEBUG MODE ============="
        $LOGIC PrintText ""
    }

    $LOGIC PrintText "TCL: =========================================="
    $LOGIC PrintText "TCL: == Image Alignment CommandLine: $deformableType "
    $LOGIC PrintText "TCL: =========================================="

    # check arguments

    if { $fixedVolumeNode == "" || [$fixedVolumeNode GetImageData] == "" } {
        PrintError "CMTKRegistration: fixed volume node not correctly defined"
        return ""
    }

    if { $movingVolumeNode == "" || [$movingVolumeNode GetImageData] == "" } {
        PrintError "CMTKRegistration: moving volume node not correctly defined"
        return ""
    }

    if { $outVolumeNode == "" } {
        PrintError "CMTKRegistration: output volume node not correctly defined"
        return ""
    }

    set fixedVolumeFileName [WriteDataToTemporaryDir $fixedVolumeNode Volume]
    if { $fixedVolumeFileName == "" } {
        # remove files
        return ""
    }
    set RemoveFiles "$fixedVolumeFileName"


    set movingVolumeFileName [WriteDataToTemporaryDir $movingVolumeNode Volume]
    if { $movingVolumeFileName == "" } {
        #remove files
        return ""
    }
    set RemoveFiles "$RemoveFiles $movingVolumeFileName"


    set outVolumeFileName [CreateTemporaryFileNameForNode $outVolumeNode]
    if { $outVolumeFileName == "" } {
        #remove files
        return ""
    }
    set RemoveFiles "$RemoveFiles $outVolumeFileName"

    ## CMTK specific arguments

    set CMD "$REGISTRATION_PACKAGE_FOLDER/registration"

    if { $affineType == [$mrmlManager GetRegistrationTypeFromString RegistrationTest] } {
        set CMD "$CMD --dofs 0"
    } elseif { $affineType == [$mrmlManager GetRegistrationTypeFromString RegistrationFast] } {
        set CMD "$CMD --accuracy 0.5 --initxlate --exploration 8.0 --dofs 6 --dofs 9"
    } elseif { $affineType == [$mrmlManager GetRegistrationTypeFromString RegistrationSlow] } {
        set CMD "$CMD --accuracy 0.1 --initxlate --exploration 8.0 --dofs 6 --dofs 9"
    } else {
        PrintError "CMTKRegistration: Unknown affineType: $affineType"
        return ""
    }


    # affine
    set outLinearTransformDirName [CreateDirName "xform"]

    set outTransformDirName $outLinearTransformDirName

    set CMD "$CMD -o \"$outLinearTransformDirName\""
    set CMD "$CMD --write-reformatted \"$outVolumeFileName\""
    set CMD "$CMD \"$fixedVolumeFileName\""
    set CMD "$CMD \"$movingVolumeFileName\""


    ## execute affine registration
    if { $CMTK_DEBUG_MODE } {
        $LOGIC PrintText ""
        $LOGIC PrintText "DEBUG: ========== Skip Affine Registration ============="
        $LOGIC PrintText ""
    } else {
        $LOGIC PrintText "TCL: Executing $CMD"
        catch { eval exec $CMD } errmsg
        $LOGIC PrintText "TCL: $errmsg"
    }

    if { $deformableType != 0 } {

        set CMD "$REGISTRATION_PACKAGE_FOLDER/warp"

        # BSpline
        set outNonLinearTransformDirName [CreateDirName "xform"]
        set outTransformDirName $outNonLinearTransformDirName

        set CMD "$CMD --verbose"
        set CMD "$CMD --fast"
        if { $deformableType == [$mrmlManager GetRegistrationTypeFromString RegistrationTest] } {
            set CMD "$CMD --delta-f-threshold 1"
        } elseif { $deformableType == [$mrmlManager GetRegistrationTypeFromString RegistrationFast] } {
            set CMD "$CMD --grid-spacing 40 --refine 1"
            set CMD "$CMD --energy-weight 5e-2"
            set CMD "$CMD --accuracy 1 --coarsest 6"
        } elseif { $deformableType == [$mrmlManager GetRegistrationTypeFromString RegistrationSlow] } {
            set CMD "$CMD --grid-spacing 40 --refine 4"
            set CMD "$CMD --energy-weight 5e-2"
            set CMD "$CMD --exploration 16 --accuracy 0.1 --coarsest 6"
        } else {
            PrintError "CMTKRegistration: Unknown deformableType: $deformableType"
            return ""
        }

        set CMD "$CMD --initial \"$outLinearTransformDirName\""
        set CMD "$CMD -o \"$outNonLinearTransformDirName\""
        set CMD "$CMD --write-reformatted \"$outVolumeFileName\""
        set CMD "$CMD \"$fixedVolumeFileName\""
        set CMD "$CMD \"$movingVolumeFileName\""

        ## execute bspline registration
        if { $CMTK_DEBUG_MODE } {
            $LOGIC PrintText ""
            $LOGIC PrintText "DEBUG: ========== Skip Non-Rigid Registration ============="
            $LOGIC PrintText ""
        } else {
            $LOGIC PrintText "TCL: Executing $CMD"
            catch { eval exec $CMD } errmsg
            $LOGIC PrintText "TCL: $errmsg"
        }
    }

    if { $CMTK_DEBUG_MODE } {
        $LOGIC PrintText "DEBUG: =========== Defining Result Files  ====="
        set outTransformDirName "/data/EMSegment_DataSet/3.6/DebugCMTK/4Vr63V.xform"
        $LOGIC PrintText "DEBUG: TransformDir: $outTransformDirName"
        set outVolumeFileName "/data/EMSegment_DataSet/3.6/DebugCMTK/dEhJZ8_vtkMRMLScalarVolumeNode18.nrrd"
        $LOGIC PrintText "DEBUG: Output Volume File: $ outVolumeFileName"
        set fixedVolumeFileName "/data/EMSegment_DataSet/3.6/DebugCMTK/xKdDW7_vtkMRMLScalarVolumeNode12.nrrd"
        $LOGIC PrintText "DEBUG: Fixed Volume File: $fixedVolumeFileName"
        set movingVolumeFileName "/data/EMSegment_DataSet/3.6/DebugCMTK/Tg6tDj_vtkMRMLScalarVolumeNode7.nrrd"
        $LOGIC PrintText "DEBUG: Moving Volume File: $movingVolumeFileName\n"
        set RemoveFiles ""
    }



    ## Read results back to scene
    if { [ReadDataFromDisk $outVolumeNode $outVolumeFileName Volume] == 0 } {
        if { [file exists $outVolumeFileName] == 0 } {
            set outTransformDirName ""
        }
    }

    if { [file exists $outTransformDirName] == 0 } {
        set outTransformDirName ""
    }

    # Test:
    # $LOGIC PrintText "==> [[$SCENE GetNodeByID $transID] Print]"
    # exit 0
    foreach NAME $RemoveFiles {
        file delete -force $NAME
    }

    # Remove Transformation from image
    $movingVolumeNode SetAndObserveTransformNodeID ""
    $SCENE Edited

    # return transformation directory name or ""
    puts "outTransformDirName: $outTransformDirName"
    return $outTransformDirName
}


