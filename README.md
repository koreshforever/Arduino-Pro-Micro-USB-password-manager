# Arduino-Pro-Micro-USB-password-manager
Password manager with Arduino-based encryption with hardware USB

Used components
1. Arduino Pro Micro
2. Buttons
3. Display on the SSD1306 controller with a resolution of 128x32

Connecting the display
1. SDA - D2
2. SCL - D3

Connecting buttons
1. Up - D6
2. Down - D7
3. Left - D8
4. Right - D9
5. Common Ground

This manager uses a set of characters A-Z uppercase and lowercase, a space character and the end of the line. Such a set is described by 6 bits, which allows you to store four characters in three EEPROM cells. When reading, passwords from cells are unpacked and decrypted, when writing, they are packed and decrypted. A randomly generated key encrypted with a master password is used for encryption. This algorithm allows you to have a complex sequence at the output if the master password is entered incorrectly.

The device types the password by emulating the keyboard. Four buttons are used to control the device. To enter into the fields, you can use both a set from the buttons of the device and via the serial port. Description fields are not encrypted, and their contents do not depend on the master password.

Used libraries

1. Arduino Pro Micro - Keyboard
2. GyverOLED
3. GyverButton
4. Arduino Cryptography Library
