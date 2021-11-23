require "ifj21"

function hello()
  local hello : string = "hello " .. "world"
  write(hello)
  local math : number = 1+2*3
  write(math)
  local test : number
  local test2 : number
  local test3 : number
  test, test2, test3 = 1, 2, 3
  write(test)
  write(test2)
  write(test3)
end

hello()