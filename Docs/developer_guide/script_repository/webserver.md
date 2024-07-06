## WebServer

### Create a custom web server instance to serve static content

```python
import WebServer
import WebServerLib
# serve content from the temp directory on port 9916 and print all messages
handler = WebServerLib.StaticPagesRequestHandler(docroot=b"/tmp", logMessage=print)
logic = WebServer.WebServerLogic(port=9916, requestHandlers=[handler], logMessage=print)
logic.start()
print(f"Open 'http://localhost:{logic.port}'")
# stop later with logic.stop()
```

:::{note}

Hosting web content is a complex topic, with security implications.  Use this feature
with caution.  It's best to use this only on a trusted network.

:::

### Create a custom web server to handle request endpoints

See the WebServerLib for more complete examples.

```python
import urllib
import WebServer
import WebServerLib

class ExampleRequestHandler(WebServerLib.BaseRequestHandler):

    def __init__(self, logMessage = None):
        """
        Initialize a new Example request handler instance.
        :param logMessage: An optional external handle for message logging.
        """
        self.logMessage = logMessage

    def canHandleRequest(self, uri: bytes, **_kwargs) -> float:
        """
        Whether the given request is a Example request.
        :param uri: The request URI to parse.
        :return: 0.5 confidence if the request is an Example request, else 0.0
        """
        parsedURL = urllib.parse.urlparse(uri)
        return 0.5 if parsedURL.path.startswith(b"/example") else 0.0

    def handleRequest(self, method: str, uri: bytes, requestBody: bytes, **_kwargs) -> tuple[bytes, bytes]:
        """
        Dispatches various example requests.
        :param method: The HTTP request method. 'GET', 'POST', etc.
        :param uri: The request URI to parse.
        :param requestBody: the binary that came with the request
        :return: tuple of content type (based on file ext) and request body binary (contents of file)
        """
        parsedURL = urllib.parse.urlparse(uri)
        contentType = b"text/plain"
        responseBody = None
        splitPath = parsedURL.path.split(b"/")
        if len(splitPath) > 2 and splitPath[2] == b"ping":
            self.logMessage("handling collections")
            responseBody = b"pong"
        else:
            self.logMessage("Unhandled Example request path: %s" % parsedURL.path)
            responseBody = b"Unhandled Example request path"

        return contentType, responseBody

# create a server with a custom handler class - here it does nothing, but it
# can access and use anything from the Slicer python environment
PORT = 2042
import WebServer
logMessage = WebServer.WebServerLogic.defaultLogMessage
requestHandlers = [ExampleRequestHandler()]
logic = WebServer.WebServerLogic(port=PORT, logMessage=logMessage, enableSlicer=False, enableStaticPages=False, enableDICOM=False, requestHandlers=requestHandlers)

logic.start()
print(f"Open 'http://localhost:{logic.port}/example/ping'")
# stop later with logic.stop()
```
