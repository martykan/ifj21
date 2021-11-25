-- Test: type of left values and right values dont match

require "ifj21"

function ret_string(str: string) : string
    return str
end

function main()
    local a : number

    a = ret_string("str")
end

main()
