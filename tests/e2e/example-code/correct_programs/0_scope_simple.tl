require "ifj21"

function foo(n : integer)
  local x : integer = 556
  write("foo: ", n, " ", x, "\n")
end

function main()
  -- scope 0
  local x : integer = 4
  write(x, "\n")

  if x == 4 then
    write("first if ", x, "\n")
  else
  end

  if x == 4 then
    -- scope 1
    local x : integer = x + 4
    write(x, "\n")
    x = 3

    foo(x)

    if x > 4 then
      -- scope 2 ----> we are here <----
      local x : number = 4.4
      write("scope scope", x, "\n")
    else
      local x : integer = 6
      write("else scope ", x, "\n")
    end
    write("3: ", x, "\n")

  else
    -- scope 3
  end

  write("top: ", x, "\n")
end

main()