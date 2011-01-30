import socket
import java

class SocketHandler(java.lang.Thread):
    def __init__(self, clientSocket, clientAddr, bufferSize=4096, *args, **kwargs):
        self.socket = clientSocket
        self.socketAddr = clientAddr
        self.bufferSize = bufferSize
        return

    def run(self):
        return self.readData()

    def readData(self):
        while 1:
            data = self.socket.recv(self.bufferSize)
            ret = self.processData(data)
            if data == None or len(data) == 0 or ret == None:
                self.socket.close()
                break
            self.socket.send(ret)
        return

    def processData(self, data):
        return None

class SocketServer(java.lang.Thread):
    bufferSize = 4096
    def __init__(self, hostname, port, handlerClass, *handlerArgs, **handlerKWArgs):
        self.setDaemon(1)
        self.hostname = hostname
        self.port = port
        self.handlerClass = handlerClass
        self.handlerArgs = handlerArgs
        self.handlerKWArgs = handlerKWArgs
        return

    def run(self):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        if self.hostname == None:
            self.hostname = socket.gethostname()

        self.socket.bind(self.hostname, self.port)
        self.socket.listen(5)
        while 1:
            (clientSocket, clientAddr) = self.socket.accept()
            handlerThread = self.handlerClass(clientSocket, clientAddr, self.bufferSize, 
                                              *self.handlerArgs, **self.handlerHWArgs)
            handlerThread.start()
        return


if __name__ == '__main__':

    class Handler(SocketHandler):
        def __init__(self, sock, addr, x):
            SocketHandler.__init__(sock, addr)
            self.x = x

        def processData(self, data):
            d = data.strip()
            if d == 'quit':
                return None
            return "%s: %s" (x, data)
    
    serv = SocketServer('localhost', 4444, Handler, "handler")

    serv.start()
    
