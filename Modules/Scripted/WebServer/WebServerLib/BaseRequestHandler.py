"""Base interface(s) for the Slicer WebServer module."""

import abc

from collections.abc import Callable


BaseRequestLoggingFunction = Callable[[list[any]], None]
"""Function signature for an external handle for message logging."""


class BaseRequestHandler(abc.ABC):
    """
    Abstract base class (ABC) defining the `SlicerRequestHandler` virtual interface.

    Zero or more request handlers may be provided to the WebServer module
    to synchronously handle incoming web requests in the Slicer context.
    The request handler must define an endpoint on the Slicer application,
    such as `/slicer/dicom`, `/slicer/info`, etc.

    A request handler may perform various actions such as reporting information
    to the requesting service or prompting action in the running Slicer instance.

    We encourage developers to extend `SlicerRequestHandler` to implement
    custom request handling behavior and Slicer endpoints.
    """

    @abc.abstractmethod
    def __init__(self, logMessage: BaseRequestLoggingFunction | None, **kwargs):
        """
        Initialize a new request handler instance.
        :param logMessage: An optional external handle for message logging.
        """
        pass

    @abc.abstractmethod
    def canHandleRequest(self, method: str, uri: bytes, requestBody: bytes) -> float:
        """
        Indicate whether this request handler instance can handle
        the incoming request.

        :param method: The HTTP request method. 'GET', 'POST', etc.
        :param uri: The request URI string to parse.
            For example, b'http://127.0.0.1:2016/slicer/test?key=value'
        :param requestBody: The request body to parse.
        :returns: Floating-point confidence value between 0.0 and 1.0
            indicating how confident we are that the request handler
            can handle the incoming request.
        """
        pass

    @abc.abstractmethod
    def handleRequest(
        self, method: str, uri: bytes, requestBody: bytes,
    ) -> tuple[bytes, bytes]:
        """
        Do the work of handling the incoming request.

        SlicerWebServer guarantees that `handleRequest` _may_ be called
        only if `canHandleRequest` indicated a nonzero confidence.

        :param method: The HTTP request method. 'GET', 'POST', etc.
        :param uri: The request URI to parse.
            For example, b'http://127.0.0.1:2016/slicer/test?key=value'
        :param requestBody: The request body to parse.
        :returns: Tuple with the following ordered elements:
            0. The response body MIME type.
                For example, "application/json" or "text/plain".
                See: https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types
            1. The response body content.
        """
        pass
