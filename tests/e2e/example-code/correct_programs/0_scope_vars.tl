require "ifj21"

function main()
  local x : integer = 4
  write(x, "\n")

  local i : integer = 3
  while i > 0 do
    local x : integer = x + 4 + i
    write("new x: ", x, "\n")
    i = i - 1
  end

  write("old x: ", x, "\n")
end

main()
