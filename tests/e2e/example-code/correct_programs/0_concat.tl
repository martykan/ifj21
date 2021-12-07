require "ifj21"

function not_main()
  local s1 : string = "hello there general \"kenobi\" "
  local s2 : string = "oh hi mark!"

  local len : integer = 69
  len = #(s1..s2)

  if len ~= 40 then
    write("Oh no! incorrect lenght!\n")
  else
  end

  local i : integer = #(s1)
  local new_s : string = ""
  local tmp_i : integer = 0
  local tmp_s : string = ""

  while i > 0 do
    tmp_i = i -1
    tmp_s = substr(s1, i, tmp_i)
    new_s = new_s..tmp_s
    i = i -1
  end

end

not_main()
