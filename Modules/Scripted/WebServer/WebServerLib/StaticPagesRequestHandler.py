import logging
import mimetypes
import os
import re

from .BaseRequestHandler import BaseRequestHandler, BaseRequestLoggingFunction

logger = logging.getLogger(__name__)


class StaticPagesRequestHandler(BaseRequestHandler):
    """Serves static pages content (files) from the configured docroot

    uriRewriteRules member variable contain a list of string pairs. iI each pair,
        the first one is a regexp match pattern, the second is a format string that uses the matching
        results to create the rewritten uri. For example, add .html and remove subpaths using this rule:
        `("(.*)/browse.*", "{0}/browse.html")`
    """

    @staticmethod
    def defaultLogMessage(*args):
        """
        Default implementation method to route messages to log.
        Emits with reference to `StaticPagesRequestHandler` file source.
        """
        logger.debug(*args)

    def __init__(self, docroot, logMessage: BaseRequestLoggingFunction | None = None):
        """
        Initialize a new request handler instance.
        :param docroot: directory path of static pages content
        :param logMessage: An optional external handle for message logging.
        """
        self.uriRewriteRules = []
        self.docroot = docroot
        self.logMessage = logMessage or self.defaultLogMessage
        self.logMessage("docroot: %s" % self.docroot)

    def canHandleRequest(self, **_kwargs) -> float:
        """
        Whether we can handle the incoming request.

        Work to determine content to serve is done in `handleRequest`,
        so for now we indicate that we can handle any request with
        a low confidence.

        :return: 0.1 confidence, indicating that any request can be
            potentially handled but other request handlers may be more
            suitable for specific requests.
        """
        return 0.1

    def handleRequest(
        self, method: str, uri: bytes, requestBody: bytes,
    ) -> tuple[bytes, bytes]:
        """Return directory listing or binary contents of files
        TODO: other header fields like modified time

        :param uri: portion of the url specifying the file path
        :param requestBody: binary data passed with the http request
        :return: tuple of content type (based on file ext) and request body binary (contents of file)
        """

        # rewrite URL paths according to rules
        for match, replace in self.uriRewriteRules:
            matched = re.match(match, uri.decode())
            if matched:
                uri = replace.format(*matched.groups()).encode()
                self.logMessage(f"Path rewritten to: {uri}")

        contentType = b"text/plain"
        responseBody = None
        if uri.startswith(b"/"):
            uri = uri[1:]
        path = os.path.join(self.docroot, uri)
        self.logMessage("docroot: %s" % self.docroot)
        if os.path.isdir(path):
            for index in b"index.html", b"index.htm":
                index = os.path.join(path, index)
                if os.path.exists(index):
                    path = index
        self.logMessage(b"Serving: %s" % path)
        if os.path.isdir(path):
            contentType = b"text/html"
            responseBody = b"<ul>"
            for entry in os.listdir(path):
                responseBody += b"<li><a href='%s'>%s</a></li>" % (os.path.join(uri, entry), entry)
            responseBody += b"</ul>"
        else:
            ext = os.path.splitext(path)[-1].decode()
            if ext in mimetypes.types_map:
                contentType = mimetypes.types_map[ext].encode()
            try:
                fp = open(path, "rb")
                responseBody = fp.read()
                fp.close()
            except OSError:
                responseBody = None
        return contentType, responseBody
