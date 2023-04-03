-- power.lua

pin_led = 4
pin_pwr = 1
gpio.mode(pin_led, gpio.OUTPUT)
gpio.mode(pin_pwr, gpio.OUTPUT)

cs = coap.Server()
cs:listen(5683)

pwr = false
changed = 0

function pwr_get(payload)
  if pwr then
    return "on"
  else
    return "off"
  end
end

function pwr_get_t(payload)
  if pwr then
    state = "1"
  else
    state = "0"
  end
  s = string.format("%s:%d", state, changed)
  print(s)
  return s
end

function pwr_on(payload)
  print("power->on")
  if payload == nil then
    pwr = true
    return "on"
  end
  if pwr == false then
    changed = math.floor(tonumber(payload))
    pwr = true
  end
  local s = string.format("1:%d", changed)
  print(s)
  return s
end

function pwr_off(payload)
  print("power->off")
  if payload == nil then
    pwr = false
    return "off"
  end
  if pwr == true then
    changed = math.floor(tonumber(payload))
    pwr = false
  end
  local s = string.format("0:%d", changed)
  print(s)
  return s
end

function pwr_set(payload)
  -- remove all whitespace
  payload = string.gsub(payload , "%s", "")

  if payload == "on" then
    print("power->on")
    pwr = true
    return "on"
  elseif payload == "off" then
    print("power->off")
    pwr = false
    return "off"
  else
    print("error")
    return "error"
  end
end

cs:func("pwr_get")
cs:func("pwr_get_t")
cs:func("pwr_on")
cs:func("pwr_off")
cs:func("pwr_set")

if not tmr.create():alarm(5000, tmr.ALARM_AUTO, function ()
  gpio.write(pin_led, pwr and gpio.HIGH or gpio.LOW)
  gpio.write(pin_pwr, pwr and gpio.HIGH or gpio.LOW)
  if pwr then
    print "pwr=on"
  else
    print "pwr=off"
  end
end)
then
  print("WTF, cannot create alarm")
end

-- EOF
