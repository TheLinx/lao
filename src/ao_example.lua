--[[
  Note: This is just a concept for now. It doesn't work. At all.
]]
require("ao")
require("bit")

BUF_SIZE = 4096
freq = 440.0

-- Initialize
print("lao example script")
ao.initialize()

-- Setup for default driver
default_driver = ao.defaultDriverId()
format = {
  bits = 16;
  channels = 2;
  rate = 44100;
  byte_format = "little";
}

-- Open driver
device = ao.openLive(default_driver, format)
if not device then
  error("Error opening device.")
end

-- Play some stuff
buf_size = format.bits/8 * format.channels * format.rate
buffer = {}
for i=0,format.rate do
  sample = math.floor((0.75 * 32768 * math.sin(2 * math.pi * freq * i/format.rate)) + 0.5)
  local a = bit.band(sample, 0xff)
  buffer[4*i] = a
  buffer[4*i+2] = a
  local b = bit.band(bit.rshift(sample, 8), 0xff)
  buffer[4*i+1] = b
  buffer[4*i+3] = b
end

device:play(buffer, buf_size)

-- Close and shutdown
device:close()
ao.shutdown()