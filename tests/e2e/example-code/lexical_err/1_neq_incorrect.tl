
-- Program 2: Vypocet faktorialu (rekurzivne)
-- tests detection of invalid not equal lexeme
require "ifj21"
function factorial(n : integer) : integer
  if n ~ 2 then
    return 1
  local n1 : integer = n - 1
  else
    local tmp : integer = factorial(n1)
    return n * tmp
  end
end

--[[
function not_a_token()
write("hello") end
]]

function main()
  write("Zadejte cislo pro vypocet faktorialu: ")
  local a : integer = readi()
  if a ~= nil then
        if a < 0 then
              write("Faktorial nejde spocitat!", "\n")
        else
                local vysl : integer = factorial(a)
                write("Vysledek je ", vysl, "\n")
    end
  else
                    write("Chyba pri nacitani celeho cisla!\n")
  end
end

main()
