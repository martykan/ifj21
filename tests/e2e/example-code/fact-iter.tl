require "ifj21"

-- Simplified from the example since expressions are not yet complete

function main()
  local a : integer
  local vysl : integer = 0
  write("Zadejte cislo pro vypocet faktorialu\n")
  a = readi()
  if a < 0 then
    write("Faktorial nelze spocitat\n")
  else
    vysl = 1
    while a > 0 do
      vysl = vysl * a
      a = a - 1
    end
    write("Vysledek je: ", vysl, "\n")
  end
end

main()