local ao = require("ao")

local numeric_version = string.gsub(_VERSION, "^%D+", "")
if tonumber(numeric_version) < 5.2 then
  bit = require 'bit'  -- LuaBitOp http://bitop.luajit.org/api.html
elseif _G.bit32 then
  bit = _G.bit32
else
  local f = load([[
  bit = {}
  bit.bor    = function (a,b) return a|b  end
  bit.band   = function (a,b) return a&b  end
  bit.rshift = function (a,n) return a>>n end
  ]])
  f()
end

local schar = string.char
local floor = math.floor
local sin   = math.sin
local pi    = math.pi

-- ao.initialize()   is done automatically;
-- you will only need it if you have to restart the environment

-- Setup the default live driver
local default_driver = ao.defaultDriverId()
local format = {
  bits = 16;
  channels = 2;
  rate = 44100;
  byteFormat = "little";
}

-- Open the driver
local device = ao.openLive(default_driver, format)
if not device then error("Error opening device.") end

-- Play a one second sine-wave
local freq = 440.0
local buf_size = format.bits/8 * format.channels * format.rate
local buffer = {}
for i=0,format.rate do
  local sample = floor((.75 * 32768 * sin(2*pi*freq*i/format.rate)) + .5)
  local lsb = bit.band(sample, 0xff)
  buffer[4*i+1] = schar(lsb)
  buffer[4*i+3] = schar(lsb)
  local msb = bit.band(bit.rshift(sample, 8), 0xff)
  buffer[4*i+2] = schar(msb)
  buffer[4*i+4] = schar(msb)
end

device:play(table.concat(buffer), buf_size)

-- Close and shutdown is handled by the garbage collector!
