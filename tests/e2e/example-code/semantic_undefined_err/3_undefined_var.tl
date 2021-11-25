-- Test: undefined variable use

require "ifj21"

function main()
    local a : integer = 2
    a = b + 5
end

main()
