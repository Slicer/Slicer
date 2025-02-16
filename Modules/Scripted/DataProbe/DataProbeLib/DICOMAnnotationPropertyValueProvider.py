import slicer

ROLE_ANY = slicer.vtkMRMLSliceCompositeNode.LayerNone
ROLE_BACKGROUND = slicer.vtkMRMLSliceCompositeNode.LayerBackground
ROLE_FOREGROUND = slicer.vtkMRMLSliceCompositeNode.LayerForeground
ROLE_LABEL = slicer.vtkMRMLSliceCompositeNode.LayerLabel


class DICOMAnnotationPropertyValueProvider:
    # Static in abstract provider
    @staticmethod
    def GetRoleValueAsInteger(attributes):
        if "role" in attributes:
            role = attributes["role"]

            def get_digit(s):
                return int(s) if s.isdigit() else ROLE_ANY

            if role == "foreground" or get_digit(role) == ROLE_FOREGROUND:
                return ROLE_FOREGROUND
            elif role == "background" or get_digit(role) == ROLE_BACKGROUND:
                return ROLE_BACKGROUND
            elif role == "label" or get_digit(role) == ROLE_LABEL:
                return ROLE_LABEL
        return ROLE_ANY

    # To be registered
    @staticmethod
    def CanProvideValueForPropertyName(propertyName):
        return propertyName in DICOMAnnotationPropertyValueProvider.GetSupportedProperties()

    @staticmethod
    def GetValueForPropertyName(propertyName, attributes, sliceNode):

        if isinstance(sliceNode, str):
            sliceNode = slicer.mrmlScene.GetNodeByID(sliceNode)

        # Do not attempt to retrieve dicom values if no local database exists
        if not slicer.dicomDatabase.isOpen:
            return ""

        sliceLogic = slicer.app.applicationLogic().GetSliceLogic(sliceNode)
        if not sliceLogic:
            return ""

        # Get the roles
        backgroundVolume = sliceLogic.GetNthLayerVolumeNode(slicer.vtkMRMLSliceLogic.LayerBackground)
        foregroundVolume = sliceLogic.GetNthLayerVolumeNode(slicer.vtkMRMLSliceLogic.LayerForeground)
        role_value = DICOMAnnotationPropertyValueProvider.GetRoleValueAsInteger(attributes)

        # Case I: Both background and foreground
        output = ""
        uid = None
        if backgroundVolume is not None and foregroundVolume is not None:
            bgUids = backgroundVolume.GetAttribute("DICOM.instanceUIDs")
            fgUids = foregroundVolume.GetAttribute("DICOM.instanceUIDs")
            if bgUids and fgUids:
                bgUid = bgUids.partition(" ")[0]
                fgUid = fgUids.partition(" ")[0]
                uid = bgUid  # bgUid used as default

                backgroundDicomDic = DICOMAnnotationPropertyValueProvider.extractDICOMValues(bgUid)
                foregroundDicomDic = DICOMAnnotationPropertyValueProvider.extractDICOMValues(fgUid)

                if not DICOMAnnotationPropertyValueProvider.dicsMatch(backgroundDicomDic, foregroundDicomDic):
                    return ""

                properties = {
                    "SeriesDate": "Series Date",
                    "SeriesTime": "Series Time",
                    "SeriesDescription": "Series Description",
                }

                if propertyName in properties:
                    key = properties[propertyName]
                    # If they don't match, we want to differentiate
                    if backgroundDicomDic[key] != foregroundDicomDic[key]:
                        if role_value == ROLE_BACKGROUND:
                            output = "B: " + output
                        elif role_value == ROLE_FOREGROUND:
                            output = "F: " + output
                            uid = fgUid
                    elif role_value == ROLE_FOREGROUND:
                        # If they do match, we also want to return an empty string
                        # for the foreground case, so we don't print the same thing
                        # twice
                        return ""
            elif bgUids:  # and DICOMAnnotationPropertyValueProvider.backgroundDICOMAnnotationsPersistence:
                uid = bgUids.partition(" ")[0]
            else:
                return ""
        elif backgroundVolume is not None and (role_value == ROLE_BACKGROUND or role_value == ROLE_ANY):
            uids = backgroundVolume.GetAttribute("DICOM.instanceUIDs")
            if uids:
                uid = uids.partition(" ")[0]
        elif foregroundVolume is not None and (role_value == ROLE_FOREGROUND or role_value == ROLE_ANY):
            uids = foregroundVolume.GetAttribute("DICOM.instanceUIDs")
            if uids:
                uid = uids.partition(" ")[0]
        else:
            return ""

        dicomDic = DICOMAnnotationPropertyValueProvider.extractDICOMValues(uid)

        if propertyName == "PatientName":
            output += dicomDic["Patient Name"].replace("^", ", ")
        elif propertyName == "PatientID" and dicomDic["Patient ID"] != "":
            output += "ID: " + dicomDic["Patient ID"]
        elif propertyName == "PatientBirthDate":
            output += DICOMAnnotationPropertyValueProvider.formatDICOMDate(dicomDic["Patient Birth Date"])
        elif propertyName == "PatientInfo":
            output += DICOMAnnotationPropertyValueProvider.makePatientInfo(dicomDic)
        elif propertyName == "SeriesDate":
            output += DICOMAnnotationPropertyValueProvider.formatDICOMDate(dicomDic["Series Date"])
        elif propertyName == "SeriesTime":
            output += DICOMAnnotationPropertyValueProvider.formatDICOMTime(dicomDic["Series Time"])
        elif propertyName == "SeriesDescription":
            output += dicomDic["Series Description"]
        elif propertyName == "InstitutionName":
            output += dicomDic["Institution Name"]
        elif propertyName == "ReferringPhysician" and dicomDic["Referring Physician Name"] != "":
            output += dicomDic["Referring Physician Name"].replace("^", ", ")
        elif propertyName == "Manufacturer":
            output += dicomDic["Manufacturer"]
        elif propertyName == "Model":
            output += dicomDic["Model"]
        elif propertyName == "Patient-Position":
            output += dicomDic["Patient Position"]
        elif propertyName == "MR" and dicomDic["Repetition Time"] != "":
            output += "TR: " + dicomDic["Repetition Time"]
        elif propertyName == "TE" and dicomDic["Echo Time"] != "":
            output += "TE: " + dicomDic["Echo Time"]

        return output

    @staticmethod
    def GetSupportedProperties():
        return [
            "PatientName",
            "PatientID",
            "PatientInfo",
            "SeriesDate",
            "SeriesTime",
            "SeriesDescription",
            "InstitutionName",
            "ReferringPhysician",
            "Manufacturer",
            "Model",
            "Patient-Position",
            "TR",
            "TE",
            ]

    # Implementation helper routines
    @staticmethod
    def dicsMatch(backgroundDicomDic, foregroundDicomDic):
        patient_name_match = backgroundDicomDic["Patient Name"] == foregroundDicomDic["Patient Name"]
        patient_id_match = backgroundDicomDic["Patient ID"] == foregroundDicomDic["Patient ID"]
        birth_date_match = backgroundDicomDic["Patient Birth Date"] == foregroundDicomDic["Patient Birth Date"]

        return patient_name_match and patient_id_match and birth_date_match

    @staticmethod
    def formatDICOMDate(date):
        standardDate = ""
        if date != "":
            date = date.rstrip()
            # convert to ISO 8601 Date format
            standardDate = date[:4] + "-" + date[4:6] + "-" + date[6:]
        return standardDate

    @staticmethod
    def formatDICOMTime(time):
        if time == "":
            # time field is empty
            return ""
        studyH = time[:2]
        if int(studyH) > 12:
            studyH = str(int(studyH) - 12)
            clockTime = " PM"
        else:
            studyH = studyH
            clockTime = " AM"
        studyM = time[2:4]
        studyS = time[4:6]
        return studyH + ":" + studyM + ":" + studyS + clockTime

    @staticmethod
    def makePatientInfo(dicomDic):
        # This will give an string of patient's birth date,
        # patient's age and sex
        patientInfo = dicomDic["Patient Birth Date"] + ", " + dicomDic["Patient Age"] + ", " + dicomDic["Patient Sex"]
        return patientInfo

    @staticmethod
    def extractDICOMValues(uid):
        p = {}
        tags = {
            "0008,0021": "Series Date",
            "0008,0031": "Series Time",
            "0008,0060": "Modality",
            "0008,0070": "Manufacturer",
            "0008,0080": "Institution Name",
            "0008,0090": "Referring Physician Name",
            "0008,103e": "Series Description",
            "0008,1090": "Model",
            "0010,0010": "Patient Name",
            "0010,0020": "Patient ID",
            "0010,0030": "Patient Birth Date",
            "0010,0040": "Patient Sex",
            "0010,1010": "Patient Age",
            "0018,5100": "Patient Position",
            "0018,0080": "Repetition Time",
            "0018,0081": "Echo Time",
        }
        for tag, tag_value in tags.items():
            value = slicer.dicomDatabase.instanceValue(uid, tag)
            p[tag_value] = value
        return p
