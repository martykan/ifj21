-- Test: too few valus returned from function in assignment

require "ifj21"

function foo() : string, string
    return "a"
end

function main()
    local str1 : string
    local str2 : string

    str1, str2 = foo()
end

main()
