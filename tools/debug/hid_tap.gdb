set pagination off
set confirm off

define hidtap
  if $argc == 0
    error usage: hidtap MASK [FRAMES]
  end
  set $hid_mask = $arg0
  if $argc >= 2
    set $hid_count = $arg1
  else
    set $hid_count = 8
  end
  break *0x0203dd70
  commands
    silent
    set {unsigned int}($r4+0x0c) = $hid_mask
    set {unsigned int}($r4+0x10) = $hid_mask
    set {unsigned int}($r4+0x14) = $hid_mask
    set {unsigned int}($r4+0x18) = $hid_mask
    set {unsigned int}($r4+0x1c) = $hid_mask
    set {unsigned int}($r4+0x20) = $hid_mask
    set {unsigned int}($r4+0x24) = $hid_mask
    set {unsigned int}($r4+0x28) = $hid_mask
    set {unsigned int}($r4+0x2c) = $hid_mask
    set $hid_count = $hid_count - 1
    if $hid_count <= 0
      delete $bpnum
    end
    continue
  end
  signal 0
end

document hidtap
Inject a Nintendo DS key mask through the live HID keypad state.
Usage: hidtap MASK [FRAMES]
Examples:
  hidtap 0x1      # A for 8 HID update frames
  hidtap 0x20 16  # Left for 16 HID update frames
end

define hidhold
  if $argc == 0
    error usage: hidhold MASK
  end
  set $hid_manager = *(unsigned int*)0x021418c4
  set $hid_keypad = *(unsigned int*)$hid_manager
  set {unsigned int}($hid_keypad+0x0c) = $arg0
  set {unsigned int}($hid_keypad+0x10) = $arg0
  set {unsigned int}($hid_keypad+0x14) = $arg0
  set {unsigned int}($hid_keypad+0x18) = $arg0
  set {unsigned int}($hid_keypad+0x1c) = $arg0
  set {unsigned int}($hid_keypad+0x20) = $arg0
  set {unsigned int}($hid_keypad+0x24) = $arg0
  set {unsigned int}($hid_keypad+0x28) = $arg0
  set {unsigned int}($hid_keypad+0x2c) = $arg0
end

define hidclear
  hidhold 0
end
