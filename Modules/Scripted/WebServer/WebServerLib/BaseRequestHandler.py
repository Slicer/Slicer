"""Base interface(s) for the Slicer WebServer module."""

import abc

from collections.abc import Callable


BaseRequestLoggingFunction = Callable[[list[any]], None]
"""Function signature for an external handle for message logging."""


class _KeepOpen:
    """Sentinel type — see :data:`KEEP_OPEN`."""

    __slots__ = ()

    def __repr__(self):
        return "KEEP_OPEN"

    def __bool__(self):
        # Falsy so that a handler that returns KEEP_OPEN but whose return value
        # is accidentally treated as a normal response body falls into the 404
        # branch rather than emitting the sentinel's repr to the client.
        return False


KEEP_OPEN = _KeepOpen()
"""
Singleton response-body sentinel used to request that SlicerHTTPServer hand
ownership of the underlying TCP socket to the handler after sending initial
headers, instead of writing a single ``Content-Length``-bounded response and
closing.

A handler that wants to stream data to the client over time (Server-Sent
Events, WebSocket upgrade, long-polling, progressive media, etc.) returns::

    return (b"text/event-stream", KEEP_OPEN)

from :meth:`BaseRequestHandler.handleRequest`. :class:`SlicerHTTPServer` then:

    1. Writes an HTTP status line + the caller-supplied ``Content-Type`` (and
       ``Cache-Control: no-cache`` and ``Connection: keep-alive``) — but *no*
       ``Content-Length`` — to the client socket.
    2. Invokes :meth:`BaseRequestHandler.onConnectionTakeover` with the raw
       socket and the existing read/write ``QSocketNotifier`` instances.
    3. Ceases all further management of the connection. The handler is
       responsible for writing data, reading any further client traffic, and
       closing the socket when the stream ends.
"""


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
            1. The response body content, or :data:`KEEP_OPEN` to take
                ownership of the socket for streaming; see
                :meth:`onConnectionTakeover`.
        """
        pass

    def onConnectionTakeover(self, socket, writeNotifier, readNotifier):
        """
        Called by :class:`SlicerHTTPServer` after :meth:`handleRequest`
        returned a ``(contentType, KEEP_OPEN)`` tuple.

        The server has already written the HTTP status line, ``Content-Type``,
        ``Cache-Control: no-cache``, ``Connection: keep-alive``, and the
        blank-line header terminator to ``socket``. No ``Content-Length`` is
        sent. From this point forward the handler owns the socket and both
        notifiers; the server will not touch them.

        The default implementation disables the notifiers and closes the
        socket — a handler that returns ``KEEP_OPEN`` without overriding
        this method is buggy, and the default prevents it from leaking a
        half-open connection.

        :param socket: The underlying :class:`socket.socket` for the
            connection. Still connected, TCP_NODELAY not set.
        :param writeNotifier: A ``qt.QSocketNotifier(fileno, Write)`` ready
            for the handler to connect to its own write slot. Not yet
            enabled.
        :param readNotifier: The ``qt.QSocketNotifier(fileno, Read)`` the
            server used to read the request. Already disabled. The handler
            may re-enable it if it expects more client→server traffic
            (WebSocket frames, SSE client abort detection, etc.).
        """
        writeNotifier.setEnabled(False)
        readNotifier.setEnabled(False)
        try:
            socket.close()
        except OSError:
            pass
