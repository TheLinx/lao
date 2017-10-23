# lao

This is a Lua module with bindings to
[libao the Cross Platform Audio Output Library](https://xiph.org/ao/),
to provide Lua with portable audio output.

## API

**Data Structures**   
[device](http://rawgit.com/thelinx/lao/master/doc/device.html)   
[info](http://rawgit.com/thelinx/lao/master/doc/info.html)   
[options](http://rawgit.com/thelinx/lao/master/doc/option.html)   
[sampleFormat](http://rawgit.com/thelinx/lao/master/doc/sample_format.html)   
   
**Environment Setup/Teardown**   
[ao.initialize](http://rawgit.com/thelinx/lao/master/doc/ao_initialize.html)   
[ao.shutdown](http://rawgit.com/thelinx/lao/master/doc/ao_shutdown.html)   
   
**Device Setup/Playback/Teardown**   
[ao.openLive](http://rawgit.com/thelinx/lao/master/doc/ao_open_live.html)   
[ao.openFile](http://rawgit.com/thelinx/lao/master/doc/ao_open_file.html)   
[device:play](http://rawgit.com/thelinx/lao/master/doc/device_play.html)   
[device:close](http://rawgit.com/thelinx/lao/master/doc/device_close.html)   
   
**Driver Information**   
[ao.driverId](http://rawgit.com/thelinx/lao/master/doc/ao_driver_id.html)   
[ao.defaultDriverId](http://rawgit.com/thelinx/lao/master/doc/ao_default_driver_id.html)   
[ao.driverInfo](http://rawgit.com/thelinx/lao/master/doc/ao_driver_info.html)   
[ao.driverInfoList](http://rawgit.com/thelinx/lao/master/doc/ao_driver_info_list.html)   
   
**Miscellaneous**   
[ao.isBigEndian](http://rawgit.com/thelinx/lao/master/doc/ao_is_big_endian.html)

## Summary

lao sticks to libao principles, but ensures you're still writing Lua.
The API is
[similar to libao's,](https://xiph.org/ao/doc/libao-api.html)
but simplified so you don't have to deal with a special type
of userdata containing the sample formats, options etc.
Follow these steps:

*   *local ao = require("ao")*
*   Unlike in libao, *ao.initalize* is called when lao is required; but you can still call it to restart a libao environment after calling *ao.shutdown*
*   Call *ao.defaultDriverId* to get the ID number of the default output driver. If you want to specify a particular output driver, you may call *ao.driverId* with a string corresponding to the short name of the device (i.e. "oss", "wav", etc.) instead
*   If you are using the default live output driver, no extra options are needed. If you want special options, you supply a table of options to the *ao.open* function
*   *local device = ao.openLive*   The return value is the device you will use to play things. If you want file output, call *ao.openFile* instead
*   Call *device:play* on each block of audio
*   The following steps are optional since they're done by lao's *__gc* handlers, but you may want to do them manually for different reasons:
*   Call *device:close* to close the device
*   Call *ao.shutdown* to shutdown the libao environment

## Example

       local ao = require( "ao" )
       local schar = string.char
       local sin = math.sin ; local pi = math.pi ; local floor = math.floor
       local default_driver = ao.defaultDriverId()
       format = { bits=16; channels=2; rate=44100; byteFormat="little"; }
       -- Open the driver
       device = assert( ao.openLive(default_driver, format) )
       -- Play a one-second beep
       buf_size = format.channels * format.rate * format.bits/8
       local freq = 440.0
       buffer = {}
       for i = 0,format.rate do    -- one second
          sample = floor((0.75 * 32768 * sin(2*pi*freq*i/format.rate)) + 0.5)
          local a = bit.band(sample, 0xff)
          buffer[4*i+1] = schar(a)
          buffer[4*i+3] = schar(a)
          local b = bit.band(bit.rshift(sample, 8), 0xff)
          buffer[4*i+2] = schar(b)
          buffer[4*i+4] = schar(b)
       end
       device:play(table.concat(buffer), buf_size)

## Installation

Use *LuaRocks* to install the **ao** package:

       luarocks install ao

You need *libao* installed to use lao,
and you need the libao headers to build it from source.   
For example on *debian* :

       aptitude install libao-common
       aptitude install libao-dev

To check it installed successfully, you can download and run
[examples/ao\_live.lua](https://rawgit.com/TheLinx/lao/master/examples/ao_live.lua)   
You should hear a 440-Hz sine wave for about a second.   
And also
[examples/ao\_file.lua](https://rawgit.com/TheLinx/lao/master/examples/ao_file.lua)   
which should generate a file *beep.wav* containing the same sine wave.


## See Also

[thelinx.github.com/lao](http://thelinx.github.com/lao)   
[xiph.org/ao/doc/libao-api.html](https://xiph.org/ao/doc/libao-api.html)   
[luarocks.org/modules/peterbillam/ao](http://luarocks.org/modules/peterbillam/ao)
