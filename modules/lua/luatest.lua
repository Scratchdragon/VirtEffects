-- The processor provides the global variables Settings (table), Frame (number), FrameIndex (number), and FrameCount (number)
-- The processor will use the value of Frame as an output as well as input

-- This example node is a simple lua implementation of the volume node
Frame = Frame * Settings["Volume"] * 2