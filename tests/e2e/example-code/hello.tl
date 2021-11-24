require "ifj21"

-- TODO
global writetwice : function(string)

function writetwice(str: string)
  write(str)
  write(str)
end

function hello()
  local helo : string = "hello " .. "world"
  write(helo)
  local math : number = 1+2*3
  write(math)
  local test : number
  local test2 : number
  local test3 : number
  test, test2, test3 = 1, 2, 3
  write(test)
  write(test2)
  write(test3)
  writetwice("twice")
end

hello()
