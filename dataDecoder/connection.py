from arduino import getSerialPort, getSerialConnection

NUM_PACKAGES = 66

class Connection:
    """
    Almost useless abstraction for the connection to the arduino. Makes the main methods in decoder.py less bloated, therefore it's fine.
    """
    def __init__(self):
        """
        Throws exception when no arduino is found or some other error occured.
        """
        self.port = getSerialPort()
        if self.port==None:
            print("No appropriate port found.")
            exit(1)
        self.connection = getSerialConnection(self.port)
        if self.connection==None:
            print("Unable to connect to Teensy on {}".format(str(self.port)))
            exit(1);

        print("Connected on port {}".format(str(self.port)))

        
    def getNext(self):
        """
        Hides the possible exceptions from readline(), so we only have to take care of None in main. 
        Reads 2 bytes of data. 
        """
        try:
            return self.connection.read(NUM_PACKAGES)
        except:
            pass

