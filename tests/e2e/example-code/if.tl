require "ifj21"

function hello(x: number, y: number)
  if x then
    if y then
      write("a")
    else
      write("b")
    end
  else
    if y then
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