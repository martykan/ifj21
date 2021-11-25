-- Test: too many args entered in function call

require "ifj21"

function foo(i : integer, b : number, c : integer)
    write("........")
end

foo(5, 5.0, 5, 5, 5, 5)
