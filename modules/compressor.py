from module import *

def lerp(b, a, c):
    return (c * a) + ((1-c) * b)

class Compressor(Module):
    def processAudio(self, data, chunk, player):
        level = self.getSetting("Level") * 800 - 400
        for i in range(len(data)):
            if data[i] > self.getSetting("Cutoff") * 800 - 400:
                data[i] = lerp(data[i], level, self.getSetting("Amount"))
        return super().processAudio(data, chunk, player)