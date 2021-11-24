require "ifj21"

function main()
  local num : number
  local int : integer
  local str : string
  local hash : string = chr(35) 
  write(hash, "\n")
  write("Zadejte text: ")
  str = reads()
  write("Zadali jste ", str, "!\n")
  local ordval : integer
  ordval = ord(str, 1)
  write("ASCII hodnota 1. pismene ", ordval, "!\n")
  ordval = ord(str, 0)
  write("ASCII hodnota pismene mimo index retezce ", ordval, "!\n")
  local substring: string = substr(str, 1, 2)
  -- local substring: string = "hehehehe"
  write("Podřetězec ", substring, "!\n")
  write("Zadejte cislo: ")
  num = readn()
  write("Zadali jste ", num, "!\n")
  int = tointeger(num)
  write("Jako cele cislo ", int, "!\n")
end

main()
