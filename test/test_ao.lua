#!/usr/local/bin/lua
---------------------------------------------------------------------
--     This Lua5 script is Copyright (c) 2017, Peter J Billam      --
--                       www.pjb.com.au                            --
--  This script is free software; you can redistribute it and/or   --
--         modify it under the same terms as Lua5 itself.          --
---------------------------------------------------------------------
local Synopsis = [[
lua test_ao.lua
]]
local iarg=1; while arg[iarg] ~= nil do
    if not string.find(arg[iarg], '^-[a-z]') then break end
    local first_letter = string.sub(arg[iarg],2,2)
    if first_letter == 'v' then
        local n = string.gsub(arg[0],"^.*/","",1)
        print(n.." version "..Version.."  "..VersionDate)
        os.exit(0)
    elseif first_letter == 'c' then
        whatever()
    else
        local n = string.gsub(arg[0],"^.*/","",1)
        print(n.." version "..Version.."  "..VersionDate.."\n\n"..Synopsis)
        os.exit(0)
    end
    iarg = iarg+1
end
-- local R  = require 'randomdist'
-- require 'DataDumper'

----------------------------- infrastructure

local function round(x) return math.floor(x+0.5) end

function dump(x)
    local function tost(x)
        if type(x) == 'table' then return 'table['..tostring(#x)..']' end
        if type(x) == 'string' then return "'"..x.."'" end
        if type(x) == 'function' then return 'function' end
        if x == nil then return 'nil' end
        return tostring(x)
    end
    if type(x) == 'table' then
        local n = 0 ; for k,v in pairs(x) do n=n+1 end
        if n == 0 then return '{}' end
        local a = {}
        if n == #x then for i,v in ipairs(x) do a[i] = tost(v) end
        else for k,v in pairs(x) do a[#a+1] = tostring(k)..'='..tost(v) end
        end
        return '{ '..table.concat(a, ', ')..' }'
    end
    return tost(x)
end

function warn(...)
    local a = {}
    for k,v in pairs{...} do table.insert(a, tostring(v)) end
    io.stderr:write(table.concat(a),'\n') ; io.stderr:flush()
end
function eq (a,b,eps)
    if not eps then eps = .000001 end
    return math.abs(a-b) < eps
end
function die(...) warn(...);  os.exit(1) end
function avabs (a)
    local tot = 0.0
    for i,v in ipairs(a) do tot = tot + math.abs(a[i]) end
    return tot / #a
end
function rms (arr)
    local tot = 0.0
    for i,v in ipairs(arr) do tot = tot + v*v end
    return math.sqrt(tot / #arr)
end

i_test = 0; Failed = 0;
function ok(b,s)
    i_test = i_test + 1
    if b then
        io.write('ok '..i_test..' - '..s.."\n")
        return true
    else
        io.write('not ok '..i_test..' - '..s.."\n")
        Failed = Failed + 1
        return false
    end
end

---------------------------------- start testing
 
Tests = 1
local ao = require "ao"

-- test default numberType = "float"

local bufstr = ao.array2string(
  { 0.0, 1/30000, -1/30000, 1/512, -1/512, 0.1, -0.1, 1.0, -1.0 }
)

-- this depends on ones-complement or two's-complement arithmetic ...
-- in C, -1==~0 means one's-complement; in Lua, it's always twos-complement


local i = 1
local lsb = string.byte(bufstr,i) ; i = i + 1
local msb = string.byte(bufstr,i) ; i = i + 1
ok(lsb==0 and msb==0,
  "array2string({0}) returns "..tostring(lsb)..", "..tostring(msb))

local lsb = string.byte(bufstr,i) ; i = i + 1
local msb = string.byte(bufstr,i) ; i = i + 1
ok(lsb==1 and msb==0,
  "array2string({1/30000}) returns "..tostring(lsb)..", "..tostring(msb))

local lsb = string.byte(bufstr,i) ; i = i + 1
local msb = string.byte(bufstr,i) ; i = i + 1
ok(lsb==255 and msb==255,
  "array2string({-1/30000}) returns "..tostring(lsb)..", "..tostring(msb))

lsb = string.byte(bufstr,i) ; i = i + 1
msb = string.byte(bufstr,i) ; i = i + 1
ok(lsb==64 and msb==0,
  "array2string({1/512}) returns "..tostring(lsb)..", "..tostring(msb))

lsb = string.byte(bufstr,i) ; i = i + 1
msb = string.byte(bufstr,i) ; i = i + 1
ok(lsb==192 and msb==255,
  "array2string({-1/512}) returns "..tostring(lsb)..", "..tostring(msb))

lsb = string.byte(bufstr,i) ; i = i + 1
msb = string.byte(bufstr,i) ; i = i + 1
ok(lsb==205 and msb==12,
  "array2string({0.1}) returns "..tostring(lsb)..", "..tostring(msb))

lsb = string.byte(bufstr,i) ; i = i + 1
msb = string.byte(bufstr,i) ; i = i + 1
ok(lsb==51 and msb==243,
  "array2string({-0.1}) returns "..tostring(lsb)..", "..tostring(msb))

lsb = string.byte(bufstr,i) ; i = i + 1
msb = string.byte(bufstr,i) ; i = i + 1
ok(lsb==255 and msb==127,
  "array2string({1.0}) returns "..tostring(lsb)..", "..tostring(msb))

lsb = string.byte(bufstr,i) ; i = i + 1
msb = string.byte(bufstr,i) ; i = i + 1
ok(lsb==1 and msb==128,
  "array2string({-1.0}) returns "..tostring(lsb)..", "..tostring(msb))

-- test numberType = "unsigned"

bufstr = ao.array2string(
  { 0+32768, 93+32768, 97+32768, },
  { numberType="unsigned" }
)

i = 1
lsb = string.byte(bufstr,i) ; i = i + 1
msb = string.byte(bufstr,i) ; i = i + 1
ok(lsb==0 and msb==0,
  "array2string({0+32768},  {numberType='unsigned' }) returns "..
  tostring(lsb)..", "..tostring(msb))

lsb = string.byte(bufstr,i) ; i = i + 1
msb = string.byte(bufstr,i) ; i = i + 1
ok(lsb==93 and msb==0,
  "array2string({93+32768}, {numberType='unsigned' }) returns "..
  tostring(lsb)..", "..tostring(msb))

lsb = string.byte(bufstr,i) ; i = i + 1
msb = string.byte(bufstr,i) ; i = i + 1
ok(lsb==97 and msb==0,
  "array2string({97+32768}, {numberType='unsigned' }) returns "..
  tostring(lsb)..", "..tostring(msb))

-- test numberType = "signed"

bufstr = ao.array2string(
  { 0, 93, 97, },
  { numberType="signed" }
)

i = 1
lsb = string.byte(bufstr,i) ; i = i + 1
msb = string.byte(bufstr,i) ; i = i + 1
ok(lsb==0 and msb==0,
 "array2string({0},  {numberType='signed' }) returns "..
  tostring(lsb)..", "..tostring(msb))

lsb = string.byte(bufstr,i) ; i = i + 1
msb = string.byte(bufstr,i) ; i = i + 1
ok(lsb==93 and msb==0,
  "array2string({93}, {numberType='signed' }) returns "..
  tostring(lsb)..", "..tostring(msb))

lsb = string.byte(bufstr,i) ; i = i + 1
msb = string.byte(bufstr,i) ; i = i + 1
ok(lsb==97 and msb==0,
  "array2string({97}, {numberType='signed' }) returns "..
  tostring(lsb)..", "..tostring(msb))


--- byteFormat

bufstr = ao.array2string( { 1/30000, 0 }, { byteFormat="big" } )
i = 1
local b1 = string.byte(bufstr,i) ; i = i + 1
local b2 = string.byte(bufstr,i) ; i = i + 1
ok(b1==0 and b2==1, "array2string({1/30000}, {byteFormat='big'} returns "..
  tostring(b1)..", "..tostring(b2))

bufstr = ao.array2string( { 1/30000, 0 }, { byteFormat="native" } )
i = 1
b1 = string.byte(bufstr,i) ; i = i + 1
b2 = string.byte(bufstr,i) ; i = i + 1
-- print("b1 =",b1," b2 =",b2)
if ao.isBigEndian() then
	ok(b1==0 and b2==1, "byteFormat='native' means big-endian")
else
	ok(b1==1 and b2==0, "byteFormat='native' means little-endian")
end


