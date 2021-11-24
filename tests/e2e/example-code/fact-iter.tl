require "ifj21"

-- Simplified from the example since expressions are not yet complete

function main()
  local a : number
  local vysl : number = 1
  write("Zadejte cislo pro vypocet faktorialu\n")
  a = readi()
  while a do
    vysl = vysl * a
    a = a - 1
  end
  write("Vysledek je: ", vysl, "\n")
end

main()