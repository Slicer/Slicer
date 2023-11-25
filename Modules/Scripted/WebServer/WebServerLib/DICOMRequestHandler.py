import logging
import pydicom
import urllib
from typing import Optional

import slicer
from .BaseRequestHandler import BaseRequestHandler, BaseRequestLoggingFunction

logger = logging.getLogger(__name__)


class DICOMRequestHandler(BaseRequestHandler):
    """
    Implements the mapping between DICOMweb endpoints
    and ctkDICOMDatabase api calls.
    TODO: only a subset of api calls supported, but enough to server a viewer app (ohif)
    """

    @staticmethod
    def defaultLogMessage(*args):
        """
        Default implementation method to route messages to log.
        Emits with reference to `DICOMRequestHandler` file source.
        """
        logger.debug(*args)

    def __init__(self,
                 logMessage:Optional[BaseRequestLoggingFunction]=None):
        """
        Initialize a new request handler instance.
        :param logMessage: An optional external handle for message logging.
        """
        self.logMessage = logMessage or self.defaultLogMessage
        self.retrieveURLTag = pydicom.tag.Tag(0x00080190)
        self.numberOfStudyRelatedSeriesTag = pydicom.tag.Tag(0x00200206)
        self.numberOfStudyRelatedInstancesTag = pydicom.tag.Tag(0x00200208)

    def canHandleRequest(self, uri: bytes, **_kwargs) -> float:
        """
        Whether the given request is a DICOM request.
        :param uri: The request URI to parse.
        :return: 0.5 confidence if the request is a DICOM request, else 0.0
        """
        parsedURL = urllib.parse.urlparse(uri)
        return 0.5 if parsedURL.path.startswith(b"/dicom") else 0.0

    def handleRequest(
        self, uri: bytes, requestBody: bytes, **_kwargs,
    ) -> tuple[bytes, bytes]:
        """
        Dispatches various dicom requests
        :param uri: The request URI to parse.
        :param requestBody: the binary that came with the request
        :return: tuple of content type (based on file ext) and request body binary (contents of file)
        """
        parsedURL = urllib.parse.urlparse(uri)
        contentType = b"text/plain"
        responseBody = None
        splitPath = parsedURL.path.split(b"/")
        if len(splitPath) > 6 and splitPath[6].startswith(b"instances"):
            self.logMessage("handling instances")
            contentType, responseBody = self.handleInstances(parsedURL, requestBody)
        elif len(splitPath) > 4 and splitPath[4].startswith(b"series"):
            self.logMessage("handling series")
            contentType, responseBody = self.handleSeries(parsedURL, requestBody)
        elif len(splitPath) > 2 and splitPath[2].startswith(b"studies"):
            self.logMessage("handling studies")
            contentType, responseBody = self.handleStudies(parsedURL, requestBody)
        else:
            self.logMessage("Looks like wadouri %s" % parsedURL.query)
            contentType, responseBody = self.handleWADOURI(parsedURL, requestBody)
        return contentType, responseBody

    def handleStudies(self, parsedURL, _requestBody):
        """
        Handle study requests by returning json
        :param parsedURL: the REST path and arguments
        :param requestBody: the binary that came with the request
        """
        contentType = b"application/json"
        splitPath = parsedURL.path.split(b"/")

        offset = 0
        limit = 100
        patientID = None
        params = parsedURL.query.split(b"&")
        for param in params:
            paramName = param.split(b"=")[0].decode("UTF-8").lower()
            if paramName == "offset":
                offset = int(param.split(b"=")[1])
            elif paramName == "limit":
                limit = int(param.split(b"=")[1])
            elif paramName in ["patientid", b"00100020"]:
                patientID = param.split(b"=")[1].decode("UTF-8")

        studyCount = 0
        responseBody = b"[{}]"
        if len(splitPath) == 3:
            # studies qido search
            studyResponseString = b"["
            for patient in slicer.dicomDatabase.patients():
                if studyCount >= offset + limit:
                    break
                for study in slicer.dicomDatabase.studiesForPatient(patient):
                    # Get summary information of all the series (number of instances, modalities) and a representative series
                    representativeSeriesDataset = None
                    series = slicer.dicomDatabase.seriesForStudy(study)
                    numberOfStudyRelatedSeries = len(series)
                    numberOfStudyRelatedInstances = 0
                    modalitiesInStudy = set()
                    skipThisStudy = False
                    for serie in series:
                        seriesInstances = slicer.dicomDatabase.instancesForSeries(serie)
                        numberOfStudyRelatedInstances += len(seriesInstances)
                        if len(seriesInstances) == 0:
                            continue
                        try:
                            filename = slicer.dicomDatabase.fileForInstance(seriesInstances[0])
                            dataset = pydicom.dcmread(filename, stop_before_pixels=True)
                            if representativeSeriesDataset is None:
                                # Use the first valid data set as representative series data
                                representativeSeriesDataset = dataset
                                # Skip this study if does not match requested patient
                                if patientID is not None and dataset.PatientID != patientID:
                                    skipThisStudy = True
                                    break
                        except Exception as e:
                            self.logMessage(f'Error while attempting to read instance {seriesInstances[0]} from file "{filename}": {e}')
                            dataset = None
                        if dataset is not None:
                            try:
                                modalitiesInStudy.add(dataset.Modality)
                            except AttributeError as e:
                                self.logMessage(f"Modality information was not found in {filename} ({seriesInstances[0]})")
                    if skipThisStudy:
                        continue
                    if representativeSeriesDataset is None:
                        self.logMessage("Could not find any instances for study %s" % study)
                        continue
                    if not modalitiesInStudy:
                        modalitiesInStudy = ["OT"]
                    # Assemble study response from representative series
                    dataset = representativeSeriesDataset
                    try:
                        studyDataset = pydicom.dataset.Dataset()
                        studyDataset.SpecificCharacterSet = ["ISO_IR 100"]
                        studyDataset.StudyDate = dataset.StudyDate
                        studyDataset.StudyTime = dataset.StudyTime
                        studyDataset.StudyDescription = dataset.StudyDescription if hasattr(studyDataset, "StudyDescription") else None
                        studyDataset.StudyInstanceUID = dataset.StudyInstanceUID
                        studyDataset.AccessionNumber = dataset.AccessionNumber
                        studyDataset.InstanceAvailability = "ONLINE"
                        studyDataset.ModalitiesInStudy = list(modalitiesInStudy)
                        studyDataset.ReferringPhysicianName = dataset.ReferringPhysicianName
                        studyDataset[self.retrieveURLTag] = pydicom.dataelem.DataElement(
                            0x00080190, "UR", "http://example.com")  # TODO: provide WADO-RS RetrieveURL
                        studyDataset.PatientName = dataset.PatientName
                        studyDataset.PatientID = dataset.PatientID
                        studyDataset.PatientBirthDate = dataset.PatientBirthDate
                        studyDataset.PatientSex = dataset.PatientSex
                        studyDataset.StudyID = dataset.StudyID if hasattr(studyDataset, "StudyID") else None
                        studyDataset[self.numberOfStudyRelatedSeriesTag] = pydicom.dataelem.DataElement(
                            self.numberOfStudyRelatedSeriesTag, "IS", str(numberOfStudyRelatedSeries))
                        studyDataset[self.numberOfStudyRelatedInstancesTag] = pydicom.dataelem.DataElement(
                            self.numberOfStudyRelatedInstancesTag, "IS", str(numberOfStudyRelatedInstances))

                        # We got here, which means we have a valid study that matches query criteria and so it could be returned.
                        studyCount += 1
                        # If an offset is specified and this study index is smaller than the offset then do not return it
                        # (the client has probably already already retrieved this study).
                        if studyCount > offset:
                            jsonDataset = studyDataset.to_json(studyDataset)
                            studyResponseString += jsonDataset.encode() + b","

                    except AttributeError:
                        self.logMessage(f"Skipping study {study} with missing attribute")
            if studyResponseString.endswith(b","):
                studyResponseString = studyResponseString[:-1]
            studyResponseString += b"]"
            responseBody = studyResponseString
        elif splitPath[4] == b"metadata":
            self.logMessage("returning metadata")
            contentType = b"application/json"
            responseBody = b"["
            studyUID = splitPath[3].decode()
            series = slicer.dicomDatabase.seriesForStudy(studyUID)
            for serie in series:
                seriesInstances = slicer.dicomDatabase.instancesForSeries(serie)
                for instance in seriesInstances:
                    try:
                        filename = slicer.dicomDatabase.fileForInstance(instance)
                        dataset = pydicom.dcmread(filename, stop_before_pixels=True)
                    except Exception as e:
                        self.logMessage(f'Error while attempting to read instance {instance} from file "{filename}": {e}')
                        continue
                    jsonDataset = dataset.to_json()
                    responseBody += jsonDataset.encode() + b","
            if responseBody.endswith(b","):
                responseBody = responseBody[:-1]
            responseBody += b"]"
        return contentType, responseBody

    def handleInstances(self, parsedURL, _requestBody):
        """
        Handle series requests by returning json
        :param parsedURL: the REST path and arguments
        :param requestBody: the binary that came with the request
        """
        contentType = b"application/json"
        splitPath = parsedURL.path.split(b"/")
        responseBody = b"[{}]"
        if len(splitPath) == 7:  # .../instances
            # instance qido search
            seriesUID = splitPath[5].decode()
            instancesResponseString = b"["
            instances = slicer.dicomDatabase.instancesForSeries(seriesUID)
            for instance in instances:
                try:
                    filename = slicer.dicomDatabase.fileForInstance(instance)
                    dataset = pydicom.dcmread(filename, stop_before_pixels=True)
                except Exception as e:
                    self.logMessage(f'Error while attempting to read instance {instance} from file "{filename}": {e}')
                    continue
                instanceDataset = pydicom.dataset.Dataset()
                instanceDataset.SpecificCharacterSet = ["ISO_IR 100"]
                instanceDataset.SOPClassUID = dataset.SOPClassUID
                instanceDataset.SOPInstanceUID = dataset.SOPInstanceUID
                instanceDataset.InstanceAvailability = "ONLINE"
                instanceDataset[self.retrieveURLTag] = pydicom.dataelem.DataElement(
                    0x00080190, "UR", "http://example.com")  # TODO: provide WADO-RS RetrieveURL
                instanceDataset.StudyInstanceUID = dataset.StudyInstanceUID
                instanceDataset.SeriesInstanceUID = dataset.SeriesInstanceUID
                instanceDataset.InstanceNumber = dataset.InstanceNumber
                instanceDataset.Rows = dataset.Rows
                instanceDataset.Columns = dataset.Columns
                instanceDataset.BitsAllocated = dataset.BitsAllocated
                instanceDataset.BitsStored = dataset.BitsStored
                instanceDataset.HighBit = dataset.HighBit
                jsonDataset = instanceDataset.to_json(instanceDataset)
                instancesResponseString += jsonDataset.encode() + b","
            if instancesResponseString.endswith(b","):
                instancesResponseString = instancesResponseString[:-1]
            instancesResponseString += b"]"
            responseBody = instancesResponseString
        elif len(splitPath) == 8:  # .../instances/NNN (download)
            instanceUID = splitPath[7].decode()
            contentType = b"application/dicom"
            path = slicer.dicomDatabase.fileForInstance(instanceUID)
            fp = open(path, "rb")
            responseBody = fp.read()
            fp.close()
        elif len(splitPath) == 9 and splitPath[8] == b"metadata":  # .../instances/NNN/metadata
            self.logMessage("returning instance metadata")
            contentType = b"application/json"
            instanceUID = splitPath[7].decode()
            dataset = pydicom.dcmread(slicer.dicomDatabase.fileForInstance(instanceUID), stop_before_pixels=True)
            jsonDataset = dataset.to_json()
            responseBody = b"[" + jsonDataset.encode() + b"]"
        return contentType, responseBody

    def handleSeries(self, parsedURL, _requestBody):
        """
        Handle series requests by returning json
        :param parsedURL: the REST path and arguments
        :param requestBody: the binary that came with the request
        """
        contentType = b"application/json"
        splitPath = parsedURL.path.split(b"/")
        responseBody = b"[{}]"
        if len(splitPath) == 5:
            # series qido search
            studyUID = splitPath[-2].decode()
            seriesResponseString = b"["
            series = slicer.dicomDatabase.seriesForStudy(studyUID)
            for serie in series:
                instances = slicer.dicomDatabase.instancesForSeries(serie, 1)
                firstInstance = instances[0]
                try:
                    filename = slicer.dicomDatabase.fileForInstance(firstInstance)
                    dataset = pydicom.dcmread(filename, stop_before_pixels=True)
                except Exception as e:
                    self.logMessage(f'Error while attempting to read instance {firstInstance} from file "{filename}": {e}')
                    continue
                seriesDataset = pydicom.dataset.Dataset()
                seriesDataset.SpecificCharacterSet = ["ISO_IR 100"]
                seriesDataset.SeriesInstanceUID = dataset.SeriesInstanceUID
                try:
                    # Required (type 1) field, but we don't disqualify the series if it does not have it
                    seriesDataset.Modality = dataset.Modality
                except AttributeError as e:
                    self.logMessage(f"Modality information was not found in {filename} ({firstInstance})")
                    seriesDataset.Modality = "OT"
                try:
                    # Required (type 2) field, but we don't disqualify the series if it does not have it
                    seriesDataset.SeriesNumber = dataset.SeriesNumber
                except AttributeError as e:
                    self.logMessage(f"Series number was not found in {filename} ({firstInstance})")
                    seriesDataset.SeriesNumber = ""
                if hasattr(dataset, "PerformedProcedureStepStartDate"):
                    seriesDataset.PerformedProcedureStepStartDate = dataset.PerformedProcedureStepStartDate
                if hasattr(dataset, "PerformedProcedureStepStartTime"):
                    seriesDataset.PerformedProcedureStepStartTime = dataset.PerformedProcedureStepStartTime
                jsonDataset = seriesDataset.to_json(seriesDataset)
                seriesResponseString += jsonDataset.encode() + b","
            if seriesResponseString.endswith(b","):
                seriesResponseString = seriesResponseString[:-1]
            seriesResponseString += b"]"
            responseBody = seriesResponseString
        elif len(splitPath) == 7 and splitPath[6] == b"metadata":
            self.logMessage("returning series metadata")
            contentType = b"application/json"
            responseBody = b"["
            seriesUID = splitPath[5].decode()
            seriesInstances = slicer.dicomDatabase.instancesForSeries(seriesUID)
            for instance in seriesInstances:
                try:
                    filename = slicer.dicomDatabase.fileForInstance(instance)
                    dataset = pydicom.dcmread(filename, stop_before_pixels=True)
                except Exception as e:
                    self.logMessage(f'Error while attempting to read instance {instance} from file "{filename}": {e}')
                    continue
                jsonDataset = dataset.to_json()
                responseBody += jsonDataset.encode() + b","
            if responseBody.endswith(b","):
                responseBody = responseBody[:-1]
            responseBody += b"]"
        return contentType, responseBody

    def handleWADOURI(self, parsedURL, _requestBody):
        """
        Handle wado uri by returning the binary part10 contents of the dicom file
        :param parsedURL: the REST path and arguments
        :param requestBody: the binary that came with the request
        """
        q = urllib.parse.parse_qs(parsedURL.query)
        try:
            instanceUID = q[b"objectUID"][0].decode().strip()
        except KeyError:
            return None, None
        self.logMessage("found uid %s" % instanceUID)
        contentType = b"application/dicom"
        path = slicer.dicomDatabase.fileForInstance(instanceUID)
        fp = open(path, "rb")
        responseBody = fp.read()
        fp.close()
        return contentType, responseBody
