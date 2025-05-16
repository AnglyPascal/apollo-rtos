   _____         _ _        _____ _____ _____ _____
  |  _  |___ ___| | |___   | __  |_   _|     |   __|
  |     | . | . | | | . |  |    -| | | |  |  |__   |
  |__|__|  _|___|_|_|___|  |__|__| |_| |_____|_____|
        |_|

  Welcome to Apollo RTOS!

  boot level: boot
  stats: n_flash = 124, n_boot = 7, n_power = 6, n_reset = 99

  >> help
  pkill   htop    help    prof    accel   show
  calc    clear   echo    cat     ls      touch
  rm      heart

  >> accel &
  started [4] output to stdout

  >> echo hello, world > 10

  >> cat 10
  hello, world

  >> htop
  boot level: boot
  curr_proc: htop
    |  0. idle : (idle), [runnable]
    |  1. shell : (high), [asleep]
    |  2. display : (low), [runnable]
    |  3. accel_bg : (high), [runnable]
    |  4. accel : (high), [runnable]
    |  5. htop : (medium), [running]
  waitlist: NONE

  >> rm 10

  >> ls 10
  file 10 not found

  >> prof
  procs:
    |  shell: (0.745%)
    |  display: (61.756%)
    |  accel_bg: (0.22%)
    |  prof: (0.674%)
    |  accel: (0.3%)
    |  cat: (0.11%)
    |  ls: (0.138%)
    |  rm: (0.3%)
    |  htop: (5.75%)
    |  idle: (31.567%)

  >> 

