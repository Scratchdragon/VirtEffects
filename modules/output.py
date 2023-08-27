from module import *

class Output(Module):
    def processAudio(self, data, chunk, player):
        player.write(data, chunk)
        return 0