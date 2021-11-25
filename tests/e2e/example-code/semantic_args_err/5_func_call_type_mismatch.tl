-- Test: function call args dont match declared types

require "ifj21"

function foo(str : string, i : integer) : number, string
    return 5, "a"
end

foo("a", "b")
