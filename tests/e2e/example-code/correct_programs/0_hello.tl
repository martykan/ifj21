require "ifj21"

global hello2 : function()

function foo(param : string)
  write("test")
end

function hello()
  write("Hello World")
  hello2()
  return
end

function hello2()
  write("Hello", "World", 2.0)
end

hello()