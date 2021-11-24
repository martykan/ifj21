require "ifj21"

function hello(x: integer, y: integer)
  if x >= 1 then
    if y >= 1 then
      write("a")
    else
      write("b")
    end
  else
    if y > 0 then
      write("c")
    else
      write("d")
    end
  end
end

function main()
  hello(1,1)
  hello(1,0)
  hello(0,1)
  hello(0,0)
end

main()