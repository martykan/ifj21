require "ifj21"

function hello()
  local concat : string = "ahoj \12 hello"
  write(concat, "\n")
  local math1 : integer = 1+2*3
  write(math1, "\n")
  local math3 : integer = math1 * 2
  write(math3, "\n")
  local math4 : integer = #"asd" + 5
  write(math4, "\n")
end

hello()
