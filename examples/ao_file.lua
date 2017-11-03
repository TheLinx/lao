local ao = require("ao")

-- ao.initialize()   is done automatically :-)
-- you will only need it if you have to restart the environment

-- Setup the default live driver
local wav_driver = ao.driverId("wav")
local format = { bits=16; channels=2; rate=44100; byteFormat="little"; }

-- Open the driver
local device = ao.openFile(wav_driver, "beep.wav", false, format)
if not device then error("Error opening device.") end

-- Play a one second sine-wave
local freq = 440.0
local buf_size = format.bits/8 * format.channels * format.rate
local buffer = {}
for i = 0,format.rate do    -- one second
   sample = 0.75 * math.sin(2*math.pi*freq*i / format.rate)
   buffer[2*i+1] = sample   -- left
   buffer[2*i+2] = sample   -- right
end
device:play( ao.array2string(buffer) )   -- assumes 16 bits ...

-- Close and shutdown is handled by the garbage collector :-)
