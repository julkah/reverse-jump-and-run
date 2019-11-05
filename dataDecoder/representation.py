from enum import Enum
from time import sleep


"""
Very basic configuration of the output:
    WIDTH/HEIGHT    - determine the dimensions of the board
    markers         - smybols that __can__ be used to display different block types
"""
WIDTH  = 16
HEIGHT =  8
markers = "DCBA"

class Location:
    def __init__(self, x=0, y=0):
        self.x = x
        self.y = y

    def __eq__(self, other):
        return self.x == other.x and self.y == other.y

    def following(self):
        """
        Calculate the next block on the board while keeping the given dimensions.
        Order: Left to right. Top to Bottom.
        """
        nextX = (self.x + 1) % WIDTH
        nextY = ((self.y + 1) % HEIGHT) if nextX < self.x else self.y

        return Location(nextX,nextY)


class Block:
    
    def __init__(self, location=Location(0,0), blockType=0):
        self.location = location
        self.blockType = blockType

    def setBlockType(self, blockType):
        self.blockType = blockType
        return self

    def empty(self):
        return self.blockType == 0


def getBlockByPosition(blocklist, location):
    """
    Searching for the first block that matches the location.
    If no block is found, return a block instance with a EMPTY block type.
    """
    for block in blocklist:
        if block.location == location:
            return block
    return Block(location, 0)

def draw(blocklist, buttonlist):
    """
    Prints how the symbols and types relate to each other. Displays the entire board and which buttons are pressed. Only displays the board, if any data was received, be it block or button data.
    """
    # empty  = True

    # for x in blocklist:
    #     if not x.empty():
    #         empty = False
    #         break
    # for x in buttonlist:
    #     if x == 1:
    #         empty = False
    #         break

    # if(empty):
    #     return

    blocktypes = {0 : '_'}
    ids = list(markers)

    for b in blocklist:
        if b.blockType not in blocktypes:
            blocktypes[b.blockType] = ids.pop()

    print(blocktypes)

    for y in range(HEIGHT):
        for x in range(WIDTH):
            print(blocktypes[getBlockByPosition(blocklist, Location(x,y)).blockType], end='')
        print()
    print("\n" * 2)
    
    for i in range(len(buttonlist)):
        if buttonlist[i]:
            print("Button {} pressed".format(i))
            sleep(2)

