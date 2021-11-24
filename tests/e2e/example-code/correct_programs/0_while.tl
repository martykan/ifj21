require "ifj21"

function main()
  local x: number = 5
  --    v this isn't possible in teal/lua - there has to be a comp. op
  while x do
    write("Hello ", x, "\n")
    x = x - 1
  end
end

main()
