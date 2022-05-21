import logging
import mimetypes
import os


class StaticPagesRequestHandler(object):
    """Serves static pages content (files) from the configured docroot
    """

    def __init__(self, docroot):
        """
        :param docroot: directory path of static pages content
        :param logMessage: callable to log messages
        """

        self.docroot = docroot
        self.logMessage('docroot: %s' % self.docroot)

    def logMessage(self, *args):
        logging.debug(args)

    def canHandleRequest(self, uri, requestBody):
        return 0.1

    def handleRequest(self, uri, requestBody):
        """Return directory listing or binary contents of files
        TODO: other header fields like modified time

        :param uri: portion of the url specifying the file path
        :param requestBody: binary data passed with the http request
        :return: tuple of content type (based on file ext) and request body binary (contents of file)
        """
        contentType = b'text/plain'
        responseBody = None
        if uri.startswith(b'/'):
            uri = uri[1:]
        path = os.path.join(self.docroot, uri)
        self.logMessage('docroot: %s' % self.docroot)
        if os.path.isdir(path):
            for index in b"index.html", b"index.htm":
                index = os.path.join(path, index)
                if os.path.exists(index):
                    path = index
        self.logMessage(b'Serving: %s' % path)
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
                fp = open(path, 'rb')
                responseBody = fp.read()
                fp.close()
            except IOError:
                responseBody = None
        return contentType, responseBody
