from module import *

class Volume(Module):
    def processAudio(self, data, chunk, player):
        for i in range(len(data)):
            data[i] = data[i] * 4 * self.getSetting("Volume")
        return super().processAudio(data, chunk, player)