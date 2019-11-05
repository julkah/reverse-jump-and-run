from arduino import getSerialPort, getSerialConnection
from representation import *
from connection import * 
from time import sleep


def decodeInput(blocklist, buttonlist, bytestr):
    """
    Update the current state of the block/buttonlist accordingly to
    the recieved input. Can behave just like id in some cases.
    """

    if bytestr[0] == 0xFF:

        loc = Location(0,0)
        for i in range (1,NUM_PACKAGES-2):
            blockdata = bytestr[i]
            block1 = (blockdata & 0xF0) >> 4
            blocklist.append(Block(loc,block1))
            loc = loc.following()
            block2 = blockdata & 0x0F
            blocklist.append(Block(loc,block2))
            loc = loc.following()

        buttonlist = [((bytestr[NUM_PACKAGES-1]&0x30)==0x30), ((bytestr[NUM_PACKAGES-1]&0x0C)==0x0C), ((bytestr[NUM_PACKAGES-1]&0x03)==0x03)]

    return blocklist, buttonlist

def main():
    """
    This non-stop reads the input and print the board in a text-like manner. Waits 0 seconds between each update.
    """

    con = Connection()
    data = None


    while 1:

        blocklist = []
        buttonlist = []

        data = con.getNext()
        
        # This line is only used for debugging. Prints the data read from the serial port in binary. 
        # print(bin(int(str(data.hex()), 16))) 

        if data == None:
            continue

        blocklist, buttonlist = decodeInput(blocklist, buttonlist, data)

        draw(blocklist, buttonlist)

if __name__ == "__main__":
    main()
