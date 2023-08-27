import importlib

class Module:
    def __init__(self):
        return

    # Module information
    title = "Module"
    symbol = "ph"

    enabled = False
    outId = -1

    # Settings data
    _settings = {}
    
    # Settings methods
    
    def getSetting(self, key):
        return self._settings[key]
    
    def setSetting(self, key, value):
        self._settings[key] = value
    
    # Override methods

    # Is run once per chunk of audio
    def processAudio(self, data, chunk, player):
        return (data, chunk, player)
    
def LoadModule(path, filename):
    file = open(path + filename + ".layout")
    modImport = importlib.import_module(filename)
    class_ = None

    for line in file.read().splitlines():
        line.strip()
        if line.startswith("#") or line == "":
            continue

        _tokens = line.split(" ")
        tokens = []
        for token in _tokens:
            if token.strip() != "":
                tokens.append(token)

        if tokens[0] == "classname":
            class_ = getattr(modImport, tokens[1])
        elif tokens[0] == "title":
            class_.title = tokens[1]
        elif tokens[0] == "symbol":
            class_.symbol = tokens[1]
        elif tokens[0] == "slider":
            class_.setSetting(class_, tokens[1], float(tokens[2]))
        elif tokens[0] == "switch":
            class_.setSetting(class_, tokens[1], float(tokens[2]))
    file.close()
    return class_
