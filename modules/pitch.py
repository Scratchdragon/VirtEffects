from module import *

class Pitch(Module):
    def processAudio(self, data, chunk, player):
        for i in range(len(data)):
            if i % 2 == 0:
                data[i] = 0
        return super().processAudio(data, chunk, player)