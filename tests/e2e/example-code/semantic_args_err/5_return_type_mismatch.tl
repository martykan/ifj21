-- Test: returned vals dont match declared types

require "ifj21"

function foo() : number, string
    return "a", 55
end

foo()
