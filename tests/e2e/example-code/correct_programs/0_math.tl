require "ifj21"

function hello()
  local concat : string = "aaa".."bbb"
  write(concat, "\n")
  local math1 : integer = 1+2*3
  write(math1, "\n")
  local math2 : number = 1.3+2*3
  write(math2, "\n")
  local math3 : integer = math1 * 2
  write(math3, "\n")
  local math4 : integer = #"asd" + 5
  write(math4, "\n")
end

hello()
