-- Test: function returns too many values

require "ifj21"

function foo(i : integer) : string, string
    return "a", "b", "c", "d"
end

foo()
