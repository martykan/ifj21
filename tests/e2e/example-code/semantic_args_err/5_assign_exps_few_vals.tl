-- Test: too few expressions assigned

require "ifj21"

function main()
    local str1: string
    local str2: string
    
    str1, str2 = "a"
end

main()
