-- Test: multiple variable definitions of same name in one scope

require "ifj21"

function main()
    local a : integer = 2
    write(a)
    local a : string
end

main()
