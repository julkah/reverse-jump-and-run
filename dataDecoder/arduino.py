# 'pip install pySerial' on your console
# gives you access to these modules
from serial import Serial
import serial.tools.list_ports

# Iterate over the list of existing com ports
# and get the one that was manufactured by Arduino CC
# (if you have one from Arduino CC)
def getSerialPort():
    for comport in list(serial.tools.list_ports.comports()):
        if "PJRC" in str(comport.manufacturer) or "Teensy" in str(comport.manufacturer):
            return comport.device
    return None

# If we found a com port with an Arduino
# open the serial connection with the right Baud Rate
def getSerialConnection(port=None):
    port = port if port != None else getSerialPort()
    if port:
        return serial.Serial(port, 9600)
    return None


