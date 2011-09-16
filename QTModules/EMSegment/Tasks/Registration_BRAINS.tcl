    # ----------------------------------------------------------------------------
    # returns transformation when no error occurs
    # now call commandline directly
    #
    proc BRAINSResampleCLI { inputVolumeNode referenceVolumeNode outVolumeNode transformation_Node_or_FileName backgroundLevel interpolationType BRAINSBSpline } {
        variable SCENE
        variable LOGIC

        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == Resample Image CLI"
        $LOGIC PrintText "TCL: =========================================="

        set PLUGINS_DIR "[$LOGIC GetPluginsDirectory]"
        set CMD "${PLUGINS_DIR}/BRAINSResample"

        set tmpInputVolumeFileName [WriteImageDataToTemporaryDir $inputVolumeNode ]
        if { $tmpInputVolumeFileName == "" } { return 1 }
        set RemoveFiles "$tmpInputVolumeFileName"
        set CMD "$CMD --inputVolume \"$tmpInputVolumeFileName\""

        set tmpReferenceVolumeFileName [WriteImageDataToTemporaryDir $referenceVolumeNode ]
        if { $tmpReferenceVolumeFileName == "" } { return 1 }
        set RemoveFiles "$RemoveFiles \"$tmpReferenceVolumeFileName\""
        set CMD "$CMD --referenceVolume \"$tmpReferenceVolumeFileName\""

        if { $transformation_Node_or_FileName == "" } {
            PrintError "BRAINSResampleCLI: transformation node not correctly defined"
            return 1
        }

        if { $BRAINSBSpline } {
            # use a BSpline transformation
            set transformationNode $transformation_Node_or_FileName
            set tmpTransformFileName [WriteDataToTemporaryDir $transformationNode Transform]
            if { $tmpTransformFileName == "" } { return 1 }
            set RemoveFiles "$RemoveFiles \"$tmpTransformFileName\""
            set CMD "$CMD --warpTransform \"$tmpTransformFileName\""
        } else {
            # use deformation field
            set transformationFileName $transformation_Node_or_FileName
            set CMD "$CMD --deformationVolume \"$transformationFileName\""
        }

        if { $outVolumeNode == "" } {
            PrintError "BRAINSResampleCLI: output volume node not correctly defined"
            return 1
        }
        set outVolumeFileName [CreateTemporaryFileNameForNode $outVolumeNode]
        if { $outVolumeFileName == "" } { return 1 }

        set CMD "$CMD --outputVolume \"$outVolumeFileName\""

        set CMD "$CMD --defaultValue \"$backgroundLevel\""

        set CMD "$CMD --pixelType [BRAINSGetPixelTypeFromVolumeNode $referenceVolumeNode]"

        # --interpolationMode <NearestNeighbor|Linear|BSpline|WindowedSinc>
        set CMD "$CMD --interpolationMode $interpolationType"

        $LOGIC PrintText "TCL: Executing $CMD"
        catch { eval exec $CMD } errmsg
        $LOGIC PrintText "TCL: $errmsg"


        # Write results back to scene
        # This does not work $::slicer3::ApplicationLogic RequestReadData [$outVolumeNode GetID] $outVolumeFileName 0 1
        ReadDataFromDisk $outVolumeNode $outVolumeFileName Volume
        file delete -force \"$outVolumeFileName\"
        file delete -force \"$tmpInputVolumeFileName\"

        return 0
    }


    # returns a transformation Node
    proc BRAINSRegistration { fixedVolumeNode movingVolumeNode outputVolumeNode backgroundLevel affineType deformableType } {
        variable SCENE
        variable LOGIC
        variable mrmlManager

        $LOGIC PrintText "TCL: =========================================="
        $LOGIC PrintText "TCL: == BRAINSRegistration: affine: $affineType , deformable: $deformableType"
        $LOGIC PrintText "TCL: =========================================="

        if { $fixedVolumeNode == "" || [$fixedVolumeNode GetImageData] == "" } {
            PrintError "AlignInputImages: fixed volume node not correctly defined"
            return ""
        }
        set fixedVolumeFileName [WriteDataToTemporaryDir $fixedVolumeNode Volume]
        set RemoveFiles "\"$fixedVolumeFileName\""
        if { $fixedVolumeFileName == "" } { return "" }


        if { $movingVolumeNode == "" || [$movingVolumeNode GetImageData] == "" } {
            PrintError "AlignInputImages: moving volume node not correctly defined"
            return ""
        }
        set movingVolumeFileName [WriteDataToTemporaryDir $movingVolumeNode Volume]
        set RemoveFiles "$RemoveFiles $movingVolumeFileName"
        if { $movingVolumeFileName == "" } { return "" }


        if { $outputVolumeNode == "" } {
            PrintError "AlignInputImages: output volume node not correctly defined"
            return ""
        }
        set outputVolumeFileName [CreateTemporaryFileNameForNode $outputVolumeNode]
        set RemoveFiles "$RemoveFiles \"$outputVolumeFileName\""
        if { $outputVolumeFileName == "" } { return "" }



        set PLUGINS_DIR "[$LOGIC GetPluginsDirectory]"

        # First BRAINSFit call

        set CMD "\"${PLUGINS_DIR}/BRAINSFit\""

        # Filter options - just set it here to make sure that if default values are changed this still works as it supposed to
        set CMD "$CMD --backgroundFillValue $backgroundLevel"
        set CMD "$CMD --interpolationMode Linear"
        set CMD "$CMD --outputVolumePixelType [BRAINSGetPixelTypeFromVolumeNode $fixedVolumeNode]"

        set CMD "$CMD --fixedVolume \"$fixedVolumeFileName\""
        set CMD "$CMD --movingVolume \"$movingVolumeFileName\""
        set CMD "$CMD --outputVolume \"$outputVolumeFileName\""

        # Do not worry about fileExtensions=".mat" type="linear" reference="movingVolume"
        # these are set in vtkCommandLineModuleLogic.cxx automatically
        if { $deformableType != 0 } {
            set transformNode [vtkMRMLBSplineTransformNode New]
            $transformNode SetName "EMSegmentBSplineTransform"
            $SCENE AddNode $transformNode
            set transID [$transformNode GetID]
            set outputTransformFileName [CreateTemporaryFileNameForNode $transformNode]
            set CMD "$CMD --bsplineTransform \"$outputTransformFileName\""
        } else {
            set transformNode [vtkMRMLLinearTransformNode New]
            $transformNode SetName "EMSegmentLinearTransform"
            $SCENE AddNode $transformNode
            set transID [$transformNode GetID]
            set outputTransformFileName [CreateTemporaryFileNameForNode $transformNode]
            set CMD "$CMD --outputTransform \"$outputTransformFileName\""
        }
        $transformNode Delete
        set RemoveFiles "$RemoveFiles \"$outputTransformFileName\""

        if {0} { ;# this is a comment because of tcl's bracket rules in comments
            if { $affineType == [$mrmlManager GetRegistrationTypeFromString RegistrationTest] } {
                set CMD "$CMD --numberOfIterations 3    --numberOfSamples 100"
            } elseif { $affineType == [$mrmlManager GetRegistrationTypeFromString RegistrationFast] } {
                set CMD "$CMD --numberOfIterations 1500 --numberOfSamples 1000"
            } elseif { $affineType == [$mrmlManager GetRegistrationTypeFromString RegistrationSlow] } {
                set CMD "$CMD --numberOfIterations 1500 --numberOfSamples 300000"
            } else {
                PrintError "BRAINSRegistration: Unknown affineType: $affineType"
                return ""
            }
        }

        if { $deformableType == [$mrmlManager GetRegistrationTypeFromString RegistrationTest] } {
            set CMD "$CMD --numberOfIterations 3    --numberOfSamples 100"
        } elseif { $deformableType == [$mrmlManager GetRegistrationTypeFromString RegistrationFast] } {
            set CMD "$CMD --numberOfIterations 500  --numberOfSamples 30000"
        } elseif { $deformableType == [$mrmlManager GetRegistrationTypeFromString RegistrationSlow] } {
            set CMD "$CMD --numberOfIterations 1500 --numberOfSamples 300000"
        } else {
            PrintError "BRAINSRegistration: Unknown deformableType: $deformableType"
            return ""
        }



        set CMD "$CMD --useRigid --useScaleSkewVersor3D --useAffine"
        if { $deformableType != 0 } {
            set CMD "$CMD --useBSpline"
            set CMD "$CMD --splineGridSize 6,6,6"
            set CMD "$CMD --maxBSplineDisplacement 0"
            set CMD "$CMD --useCachingOfBSplineWeightsMode ON"
        }
        set CMD "$CMD --initializeTransformMode useCenterOfHeadAlign"
        set CMD "$CMD --minimumStepLength 0.005"
        set CMD "$CMD --translationScale 1000"
        set CMD "$CMD --reproportionScale 1"
        set CMD "$CMD --skewScale 1"
        set CMD "$CMD --maskProcessingMode NOMASK"
        set CMD "$CMD --numberOfHistogramBins 40"
        set CMD "$CMD --numberOfMatchPoints 10"
        set CMD "$CMD --costMetric MMI"

        set CMD "$CMD --fixedVolumeTimeIndex 0"
        set CMD "$CMD --movingVolumeTimeIndex 0"
        set CMD "$CMD --debugNumberOfThreads -1"
        set CMD "$CMD --debugLevel 0"
        set CMD "$CMD --failureExitCode -1"

#        set CMD "$CMD --medianFilterSize 0,0,0"
#        set CMD "$CMD --useExplicitPDFDerivativesMode AUTO"
#        set CMD "$CMD --relaxationFactor 0.5"
#        set CMD "$CMD --maximumStepLength 0.2"
#        set CMD "$CMD --costFunctionConvergenceFactor 1e+9"
#        set CMD "$CMD --projectedGradientTolerance 1e-5"
#        set CMD "$CMD --maskProcessingMode ROIAUTO"
#        set CMD "$CMD --ROIAutoDilateSize 3.0"
#        set CMD "$CMD --maskInferiorCutOffFromCenter 65.0"

#        set CMD "$CMD --initialTransform \"$linearTransformFileName\""
#        set CMD "$CMD --initializeTransformMode Off"


        $LOGIC PrintText "TCL: Executing $CMD"
        catch { eval exec $CMD } errmsg
        $LOGIC PrintText "TCL: $errmsg"


        # Read results back to scene
        # $::slicer3::ApplicationLogic RequestReadData [$outVolumeNode GetID] $outputVolumeFileName 0 1
        # Cannot do it that way bc vtkSlicerApplicationLogic needs a cachemanager,
        # which is defined through vtkSlicerCacheAndDataIOManagerGUI.cxx
        # instead:

        # Test:
        # ReadDataFromDisk $outputVolumeNode /home/pohl/Slicer3pohl/463_vtkMRMLScalarVolumeNode17.nrrd Volume
        if { [ReadDataFromDisk $outputVolumeNode $outputVolumeFileName Volume] == 0 } {
            set nodeID [$SCENE GetNodeByID $transID]
            if { $nodeID != "" } {
                $SCENE RemoveNode $nodeID
            }
        }

        # Test:
        # ReadDataFromDisk [$SCENE GetNodeByID $transID] /home/pohl/Slicer3pohl/EMSegmentLinearTransform.mat Transform
        if { [ReadDataFromDisk [$SCENE GetNodeByID $transID] $outputTransformFileName Transform] == 0 } {
            set nodeID [$SCENE GetNodeByID $transID]
            if { $nodeID != "" } {
                $SCENE RemoveNode $nodeID
            }
        }

        # Test:
        # $LOGIC PrintText "==> [[$SCENE GetNodeByID $transID] Print]"

        foreach NAME $RemoveFiles {
            file delete -force $NAME
        }

        # Remove Transformation from image
        $movingVolumeNode SetAndObserveTransformNodeID ""
        $SCENE Edited

        # return ID or ""
        return [$SCENE GetNodeByID $transID]
    }


    proc BRAINSGetPixelTypeFromVolumeNode { volumeNode } {

        set referenceVolume [$volumeNode GetImageData]
        set scalarType [$referenceVolume GetScalarTypeAsString]
        switch -exact "$scalarType" {
            "bit"            { set PIXELTYPE "binary" }
            "unsigned char"  { set PIXELTYPE "uchar" }
            "unsigned short" { set PIXELTYPE "ushort" }
            "unsigned int"   { set PIXELTYPE "uint" }
            "short" -
            "int" -
            "float"          { set PIXELTYPE "$scalarType" }
            default {
                PrintError "BRAINSGetPixelTypeFromVolumeNode: Cannot handle a volume of type $scalarType"
                set PIXELTYPE "INVALID"
            }
        }
        return $PIXELTYPE
    }

