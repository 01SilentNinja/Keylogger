# Keylogger
A basic keylogger code written in C. 

Features:
  1. Records every keyboard input in a file called log.txt
  2. Each input is logged with its own timestamp that includes both time(24 hour format) and date (dd/mm/yy)
  3. Inputs modified by holding down shift and using caps lock are logged as the modified output (Shift+2 ----> @)
  4. If keys that would normally not have an ascii output are pressed, they are logged with their key name(pressing backspace -----> Backspace)
  5. Holding down a key doesn't log the key multiple times, each key press is only logged once until the key is let go.
  6. To stop the logger, press ctrl+alt+e while caps lock is enabled.
  7. After the logger is exited, it shows basic session stats, such as, time of session, total key presses, Avg typing speed(wpm), Backspace error percentage and the top 5 keys pressed with their press counts.


Requirements:
  Only works for windows devices
  Any C compiler

Usage:
  Compile and run the logger.c code, the code will run in the background until it is quit by closing the .exe process from task manager or using the intended exit sequence. 
