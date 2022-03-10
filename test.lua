local file = require "file"
local test = require "file.test"

local function preopen(filename)
	print("Open", filename)
	return filename
end

local factory = file.factory { preopen = preopen }

local f = test.open(factory, "test.lua", "r")
local s = f:read(100)
print(s)
f:close()