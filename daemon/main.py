import pyaudio
import numpy as np
import os, sys
from module import *
import time

# Load all the modules
modules = {}
board = dict()

end = False

modulePath = os.path.abspath("./modules") + "/"
sys.path.append(modulePath)
print("Loading modules from '" + modulePath + "'")
for path in os.listdir(modulePath):
    if path.endswith(".layout"):
        modFile = ".".join(path.split(".")[0:len(path.split(".")) - 1])
        print("Found module file '" + modFile + "'")
        modClass = LoadModule(modulePath, modFile)
        modules[modClass.title] = modClass
        print("Loaded module " + modClass.title + "(" + modClass.symbol + ")")

def LoadBoard():
    board.clear()
    file = open("board.layout")
    for line in file.readlines():
        if line.startswith(".kill"):
            global end
            end = True
            continue

        tokens = line.split(" ")
        mod = tokens[0]

        if mod == "(null)":
            continue

        if not mod in modules.keys():
            print("Board refers to non-loaded module '" + mod + "', aborting")
            continue
        
        id = -1
        for token in tokens[1:len(tokens)]:
            if ":" in token:
                arg = token.split(":")
                if arg[0] == "id":
                    id = int(arg[1])
                    board[id] = modules[mod]()
                elif arg[0] == "outId":
                    board[id].outId = int(arg[1])
                elif arg[0] == "active":
                    board[id].enabled = arg[1].startswith("True")
                else:
                    print("Unknown module variable '" + arg[0] + "'")
            elif "=" in token:
                arg = token.split("=")
                board[id].setSetting(arg[0], float(arg[1]))
            else:
                print("Missing ':' or '=' in board item argument '" + token + "'")
                continue

    file.close()

time.sleep(1)
LoadBoard()

CHUNK = 2**5
RATE = 44100
LEN = 0.01

p = pyaudio.PyAudio()

stream = p.open(format=pyaudio.paInt16, channels=1, rate=RATE, input=True, frames_per_buffer=CHUNK)
player = p.open(format=pyaudio.paInt16, channels=1, rate=RATE, output=True, frames_per_buffer=CHUNK)


def update(data, chunk, player):
    start = -1
    for key, value in board.items():
        if value.title == "Input":
            start = key
            break

    if start == -1 or not board[start].enabled:
        return
    
    # Follow the path
    modIndex = start
    audio = (data, chunk, player)
    
    while modIndex != -1:
        # Make sure the index is valid
        if not modIndex in board.keys():
            return audio
        # Execute the module if enabled
        if board[modIndex].enabled:
            audio = board[modIndex].processAudio(*audio)
        # Move to the next one
        modIndex = board[modIndex].outId
    return audio

print("Effects Daemon started")
while not end:
    for i in range(int(LEN * RATE / CHUNK)):
        data = np.fromstring(stream.read(CHUNK),dtype=np.int16)
        update(data, CHUNK, player)
        
    LoadBoard()


print("Effects Daemon stopped")
stream.stop_stream()
stream.close()
p.terminate()