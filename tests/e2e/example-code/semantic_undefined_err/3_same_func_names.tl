-- Test: multiple function definitions of same name

require "ifj21"

function foo()
    local a : integer = 2
    write(a)
end

function foo()
    write("redefinition")
end

foo()
