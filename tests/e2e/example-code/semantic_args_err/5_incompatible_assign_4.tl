-- Test: type of left values and right values dont match

require "ifj21"

function ret_vals() : number, number
    return 1, 2
end

function main()
    local a : number
    local b : string

    a, b = ret_vals()
end

main()
