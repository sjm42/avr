-- app.lua

local mytmr = tmr.create()
mytmr:register(90000, tmr.ALARM_AUTO, function (t) dofile("temp.lua") end)
mytmr:start()

-- EOF
