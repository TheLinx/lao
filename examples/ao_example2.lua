local ao = require("ao")

local numeric_version = string.gsub(_VERSION, "^%D+", "")
if tonumber(numeric_version) < 5.2 then
  _G.bit = require 'bit'  -- LuaBitOp http://bitop.luajit.org/api.html
elseif _G.bit32 then
  _G.bit = _G.bit32
else
  local f = load([[
  _G.bit = {}
  _G.bit.bor    = function (a,b) return a|b  end
  _G.bit.band   = function (a,b) return a&b  end
  _G.bit.rshift = function (a,n) return a>>n end
  ]])
  f()
end

local schar = string.char

BUF_SIZE = 4096
freq = 440.0

-- Initialize
--ao.initialize()
--this is done when requiring, but can still be used if you need to restart the environment

-- Setup for default driver
driverid = ao.driverId("wav")
format = {
  bits = 16;
  channels = 2;
  rate = 44100;
  byteFormat = "little";
}

-- Open driver
device = ao.openFile(driverid, "boop.wav", false, format)
if not device then
  error("Error opening device.")
end

-- Play some stuff
buf_size = format.bits/8 * format.channels * format.rate
buffer = {}
for i=0,format.rate do
  sample = math.floor((0.75 * 32768 * math.sin(2 * math.pi * freq * i/format.rate)) + 0.5)
  local a = bit.band(sample, 0xff)
  buffer[4*i+1] = schar(a)
  buffer[4*i+3] = schar(a)
  local b = bit.band(bit.rshift(sample, 8), 0xff)
  buffer[4*i+2] = schar(b)
  buffer[4*i+4] = schar(b)
end

device:play(table.concat(buffer), buf_size, {})
print("ao_example2.lua lao script - see boop.wav")

-- Close and shutdown is handled by the garbage collector!
