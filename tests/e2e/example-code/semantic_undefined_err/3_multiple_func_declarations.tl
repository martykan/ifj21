-- Test: more than one declaration

require "ifj21"

global foo : function(string, number) : string, string
global foo : function(string, number) : string, string

function foo(str : string, num : number) : string
    local a : integer = 2
    write(a)
end

foo("a", 5)
