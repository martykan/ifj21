require "ifj21"

-- global foo : function(integer) : integer

function main()
  local x : integer = 4
  write(x, "\n")

  if x == 4 then
    local x : integer = x + 69
    write("if x: ", x, "\n")

    if x == 73 then
      local x : integer = x + 420
      write("if.if x: ", x, "\n")
    else
      local x : integer = x - 69
      write("if.else x: ", x, "\n")
    end
  else
  end

  -- if x == 4 then
  --   local x : integer = x - 420
  --   write("if2 x: ", x, "\n")
  -- else
  -- end

--   local i : integer = 3
--   while i > 0 do
--     local x : integer = x + 4 + i
--     write("new x: ", x, "\n")
--     i = i - 1
--   end

--   write("old x: ", x, "\n")
-- end

-- function foo(n : integer) : integer
--   local x : integer = 12
--   write("foo ", x, "\n")

--   local i : integer = 5
--   while i > 0 do
--     local x : integer = x + i
--     write("foo new x: ", x, "\n")
--     i = i - 1
--   end
end

main()
