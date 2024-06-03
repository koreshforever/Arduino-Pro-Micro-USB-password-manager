//Настроим клавиатуру
#include <Keyboard.h>


//Настроим дисплей, и определим набор символов
#include <GyverOLED.h>
const unsigned char bmpPusheen_2X_[] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 0x3f, 0x3f, 0xc3, 0xc3, 0xfc, 0xfc, 0xf3, 0xf3, 0xcf, 0xcf, 0xcf, 0xcf,
  0xcf, 0xcf, 0xf3, 0xf3, 0xfc, 0xfc, 0xf3, 0xf3, 0xcf, 0xcf, 0xcf, 0xcf, 0xcf, 0xcf, 0xcf, 0xcf,
  0xcf, 0xcf, 0x3f, 0x3f, 0x3f, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcc, 0xcc, 0x0c, 0x0c,
  0xf0, 0xf0, 0xff, 0xff, 0xf3, 0xf3, 0xff, 0xff, 0xcf, 0xcf, 0xc3, 0xc3, 0xcf, 0xcf, 0xff, 0xff,
  0xf3, 0xf3, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0xcc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xfc, 0xfc, 0xc3, 0xc3, 0x3f, 0x3f, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x3f, 0xff, 0xff,
  0xff, 0xff, 0x00, 0x00, 0x3f, 0x3f, 0x3f, 0x3f, 0x3c, 0x3c, 0x33, 0x33, 0x0f, 0x0f, 0x0f, 0x0f,
  0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
  0x03, 0x03, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0f, 0x0f, 0x33, 0x33, 0x3c, 0x3c, 0x3f, 0x3f
};
GyverOLED<SSD1306_128x32, OLED_NO_BUFFER> oled;
const char charSet[] = { '*', ' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                         'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                         'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };
//Найти номер символа в наборе
uint8_t charInSet(char ch) {
  for (uint8_t i = 0; i < sizeof(charSet); i++) {
    if (ch == charSet[i]) return i;
  }
  return 0;
}
//Найти полезную длинну строки в массиве
uint8_t chSeqLen(char seq[], uint8_t len) {
  for (uint8_t i = 0; i < len; i++) {
    if (seq[i] == charSet[0]) len = i;
  }
  return len;
}

//ПАРОЛЬ
char password[42];  //Хранение пароля
//очистка пароля
void clearPassword() {
  for (uint8_t i = 0; i < sizeof(password); i++) password[i] = charSet[0];
}

//Выбранная ячейка
uint8_t cell = 0;

//Настроим кнопочки
#include <GyverButton.h>
GButton bt_u(6);
GButton bt_d(7);
GButton bt_l(8);
GButton bt_r(9);
void tickButtons() {
  bt_u.tick();
  bt_d.tick();
  bt_l.tick();
  bt_r.tick();
}
void resetButtons() {
  bt_u.resetStates();
  bt_d.resetStates();
  bt_l.resetStates();
  bt_r.resetStates();
}


//Очистка дисплея и кнопок
void obReset() {
  oled.clear();
  resetButtons();
}

//Подключим криптографию
#include <AES.h>
#include <SHA256.h>
SHA256 hasher;
AES256 coder;
AES256 cryptor;
uint8_t hash[32];     //хранение хеша
uint8_t code[2][32];  //хранение кода: 1 - зашифрованный
char sec_val[42];     //Хранение наименованиия или пароля
//ШИфрование и дешифрование
void encryptKey() {
  updateHash();
  coder.encryptBlock(code[1], code[0]);
  coder.encryptBlock(code[1] + 16, code[0] + 16);
  cryptor.setKey(code[0], 32);
}
void decryptKey() {
  updateHash();
  coder.decryptBlock(code[0], code[1]);
  coder.decryptBlock(code[0] + 16, code[1] + 16);
  cryptor.setKey(code[0], 32);
}
void updateHash() {
  hasher.reset();
  hasher.update(password, chSeqLen(password, sizeof(password)));
  hasher.finalize(hash, 32);
  coder.setKey(hash, 32);
}


//Используем EEPROM
#include <EEPROM.h>
//сохранение и восстановление ключа
void loadCode() {
  for (uint8_t i = 0; i < 32; i++) code[1][i] = EEPROM.read(i);
  decryptKey();
}
void saveCode() {
  encryptKey();
  for (uint8_t i = 0; i < 32; i++) EEPROM.write(i, code[1][i]);
}
//Получить и положить значение
void loadValue(uint8_t num, bool pass = false) {
  uint8_t value[2][32];
  for (uint8_t i = 0; i < 32; i++) value[1][i] = charSet[0];
  for (uint8_t i = 0; i < (pass ? 32 : 30); i++) value[1][i] = EEPROM.read(32 + (pass ? 30 : 0) + 62 * num + i);
  if (pass) {
    cryptor.decryptBlock(value[0], value[1]);
    cryptor.decryptBlock(value[0] + 16, value[1] + 16);
  } else
    for (uint8_t i = 0; i < 32; i++) value[0][i] = value[1][i];

  //распаковка sec_val
  for (uint8_t i = 0; i < (pass ? 11 : 10); i++) {
    for (uint8_t k = 0; k < ((i > 9) ? 2 : 3); k++) sec_val[i * 4 + k] = charSet[value[0][i * 3 + k] & 0b111111];
    if (i < 10) sec_val[i * 4 + 3] = charSet[(value[0][i * 3] & 0b11000000) >> 6 | (value[0][i * 3 + 1] & 0b11000000) >> 4 | (value[0][i * 3 + 2] & 0b11000000) >> 2];
  }
}
void saveValue(uint8_t num, bool pass = false) {
  uint8_t value[2][32];
  //упаковка sec_val
  for (uint8_t i = 0; i < 32; i++) value[0][i] = 0x00;
  for (uint8_t i = 0; i < (pass ? 11 : 10); i++) {
    value[0][i * 3] |= charInSet(sec_val[i * 4]);
    value[0][i * 3 + 1] |= charInSet(sec_val[i * 4 + 1]);
    if (i < 10) {
      value[0][i * 3 + 2] |= charInSet(sec_val[i * 4 + 2]) | ((charInSet(sec_val[i * 4 + 3]) & 0b110000) << 2);
      value[0][i * 3] |= ((charInSet(sec_val[i * 4 + 3]) & 0b11) << 6);
      value[0][i * 3 + 1] |= ((charInSet(sec_val[i * 4 + 3]) & 0b1100) << 4);
    } else {
      uint8_t filler = random(16);
      value[0][30] |= (filler & 0b11) << 6;
      value[0][31] |= (filler & 0b1100) << 6;
    }
  }
  if (pass) {
    cryptor.encryptBlock(value[1], value[0]);
    cryptor.encryptBlock(value[1] + 16, value[0] + 16);
  } else
    for (uint8_t i = 0; i < 32; i++) value[1][i] = value[0][i];
  for (uint8_t i = 0; i < (pass ? 32 : 30); i++) EEPROM.write(32 + (pass ? 30 : 0) + 62 * num + i, value[1][i]);
}


//Редактор поля
bool editField(char field[], bool pass, String message, bool enter) {
  uint8_t pointer = 0;
  oled.clear();
  oled.home();
  oled.invertText(false);
  oled.println((String)(enter ? "Enter" : "Edit") + " " + message + ":");
  for (uint8_t j = 0; j < (pass ? 21 : 20); j++) oled.print('-');

  resetButtons();

  for (;;) {
    oled.setCursor(0, 2);
    bool endl = false;
    for (uint8_t i = 0; i < (pass ? 42 : 40); i++) {
      if (i == (pass ? 21 : 20)) oled.setCursor(0, 3);
      if (field[i] == charSet[0]) endl = true;

      if (endl) {
        oled.invertText(true);
        if (pointer == i && millis() % 1024 < 512) oled.print(' ');
        else {
          if (field[i] == ' ') oled.print('_');
          else oled.print(field[i]);
        }
      } else {
        if (pointer == i && millis() % 1024 < 512) oled.invertText(true);
        else oled.invertText(false);
        oled.print(field[i]);
      }
    }

    oled.invertText(false);

    tickButtons();

    while (Serial.available()) {
      char c = Serial.read();
      bool f = false;
      if (c == 0x1B) {
        delay(1);
        if (Serial.read() == '[') {
          char ar = Serial.read();
          if (ar == 'D') {
            if (pointer) pointer--;
            else pointer = (pass ? 42 : 40) - 1;
            Serial.write("\xE2\x86\x90");
          } else if (ar == 'C') {
            if (pointer < (pass ? 42 : 40) - 1) pointer++;
            else pointer = 0;
            Serial.write("\xE2\x86\x92");
          } else if (ar == 'A' || ar == 'B') {
            if (pointer < (pass ? 21 : 20)) pointer += (pass ? 21 : 20);
            else pointer -= (pass ? 21 : 20);
            Serial.write("\xE2\x86\x95");
          }
        } else {
          pointer = 0;
          Serial.write("\xE2\x96\xA0");
        }
      } else if (c == 0x7F) {
        if (pointer) pointer--;
        Serial.write("\xE2\x86\x90");
      } else if (c == 0x0D) {
        Serial.write(field[pointer] = charSet[0]);
        if (pointer < (pass ? 42 : 40) - 1) pointer++;
        else pointer = 0;
      } else
        for (uint8_t i = 0; i < sizeof(charSet); i++) {
          if (c == charSet[i]) {
            field[pointer] = c;
            if (pass) Serial.write('*');
            else Serial.write(c);
            if (pointer < (pass ? 42 : 40) - 1) pointer++;
            else pointer = 0;
          }
        }
    }

    if (bt_u.isClick() || bt_u.isHold()) {
      uint8_t sym_num = charInSet(field[pointer]);
      if (sym_num < sizeof(charSet) - 1) sym_num++;
      else sym_num = 0;
      field[pointer] = charSet[sym_num];
    }
    if (bt_d.isClick() || bt_d.isHold()) {
      uint8_t sym_num = charInSet(field[pointer]);
      if (sym_num) sym_num--;
      else sym_num = sizeof(charSet) - 1;
      field[pointer] = charSet[sym_num];
    }
    if (bt_l.isClick()) {
      if (pointer) pointer--;
      else pointer = (pass ? 42 : 40) - 1;
    }
    if (bt_r.isClick()) {
      if (pointer < (pass ? 42 : 40) - 1) pointer++;
      else pointer = 0;
    }
    if (bt_l.isHolded()) return false;
    if (bt_r.isHolded()) return true;
  }
}

//Меню
uint8_t showMenu(char items[][15], uint8_t count) {
  uint8_t choice = 1;
  obReset();
  for (;;) {
    oled.home();
    for (uint8_t i = 1; i <= count; i++) {
      if (i == choice) oled.invertText(true);
      else oled.invertText(false);
      for (uint8_t j = 0; j < 15; j++) oled.print(items[i - 1][j]);
      oled.println();
    }

    delay(100);

    if (bt_u.isClick() || bt_u.isHold()) {
      if (choice - 1) choice--;
      else choice = count;
    }
    if (bt_d.isClick() || bt_d.isHold()) {
      if (choice >= count) choice = 1;
      else choice++;
    }
    if (bt_l.isClick() || bt_l.isHold()) return 0;
    if (bt_r.isClick() || bt_r.isHold()) return choice;
  }
}

//Подтверждение
bool confirm() {
#define CONFIRMATION_CODE_LENGTH 10

  obReset();
  oled.invertText(false);
  oled.home();
  oled.println("Сonfirmation");
  oled.print("sequence: ");

  uint8_t code_counter;
  char code[CONFIRMATION_CODE_LENGTH];
  char confirmation[CONFIRMATION_CODE_LENGTH];
  for (uint8_t i = 0; i < CONFIRMATION_CODE_LENGTH; i++) {
    oled.print(code[i] = charSet[random(2, 64)]);
    confirmation[i] = charSet[1];
  }
  char achar = charSet[2];

  oled.setCursor(0, 2);
  oled.println("Typed");
  oled.print("sequence: ");

  for (;;) {
    oled.setCursor(60, 3);
    oled.invertText(true);

    while (Serial.available()) {
      char c = Serial.read();
      if (charInSet(c) > 1) {
        for (uint8_t i = 0; i < (CONFIRMATION_CODE_LENGTH - 1); i++) {
          confirmation[i] = confirmation[i + 1];
        }
        Serial.write(confirmation[CONFIRMATION_CODE_LENGTH - 1] = c);
      }
      code_counter = 0;
      for (uint8_t i = 0; i < CONFIRMATION_CODE_LENGTH; i++) {
        if (code[i] == confirmation[i]) code_counter++;
      }
      if (code_counter == CONFIRMATION_CODE_LENGTH) return true;
    }

    for (uint8_t i = 0; i < CONFIRMATION_CODE_LENGTH; i++) oled.print(confirmation[i]);
    if (millis() % 1024 < 512) oled.invertText(true);
    else oled.invertText(false);
    oled.setCursor(62 + 6 * CONFIRMATION_CODE_LENGTH, 3);
    oled.print(achar);
    delay(20);

    if (bt_u.isClick() || bt_u.isHold()) {
      uint8_t sym_num = charInSet(achar);
      if (sym_num < sizeof(charSet) - 1) sym_num++;
      else sym_num = 2;
      achar = charSet[sym_num];
    }
    if (bt_d.isClick() || bt_d.isHold()) {
      uint8_t sym_num = charInSet(achar);
      if (sym_num > 2) sym_num--;
      else sym_num = sizeof(charSet) - 1;
      achar = charSet[sym_num];
    }

    if (bt_l.isClick() || bt_l.isHold()) return false;
    if (bt_r.isHold()) return false;

    if (bt_r.isClick()) {
      for (uint8_t i = 0; i < (CONFIRMATION_CODE_LENGTH - 1); i++) confirmation[i] = confirmation[i + 1];
      confirmation[CONFIRMATION_CODE_LENGTH - 1] = achar;
      code_counter = 0;
      for (uint8_t i = 0; i < CONFIRMATION_CODE_LENGTH; i++) {
        if (code[i] == confirmation[i]) code_counter++;
      }
      if (code_counter == CONFIRMATION_CODE_LENGTH) return true;
    }
  }
}


//Меню ячеек
bool cellMenu() {
  //int8_t choice = cell;
  obReset();
  for (;;) {
    oled.home();

    loadValue(cell);
    oled.invertText(false);
    oled.print("Cell:");
    oled.println(cell, HEX);
    //oled.invertText(true);
    for (uint8_t i = 0; i < 20; i++) oled.print('-');

    oled.setCursor(0, 2);
    for (uint8_t i = 0; i < chSeqLen(sec_val, 40); i++) {
      if (i == 20) oled.setCursor(0, 3);
      if (sec_val[i] == ' ') oled.print(' ');
      else oled.print(sec_val[i]);
    }

    delay(100);

    if (bt_d.isClick() || bt_d.isHold()) {
      if (cell >= 15) cell = 0;
      else cell++;
      oled.clear();
    }
    if (bt_u.isClick() || bt_u.isHold()) {
      if (cell) cell--;
      else cell = 15;
      oled.clear();
    }
    if (bt_l.isClick() || bt_l.isHold()) return false;
    if (bt_r.isClick() || bt_r.isHold()) return true;
  }
}


void setup() {
  randomSeed(analogRead(A0) /* | (analogRead(A1) << 10)*/);

  clearPassword();

  Keyboard.begin();

  oled.init();
  oled.setContrast(0);
  oled.flipH(true);
  oled.flipV(true);

  oled.clear();
  oled.home();
  /*
  oled.autoPrintln(true);
  for (uint8_t i = 1; i < sizeof(charSet); i++) oled.print(charSet[i]);
  oled.autoPrintln(false);
*/
  oled.setScale(2);
  oled.setCursor(18, 0);
  oled.print("MEOW!");
  oled.drawBitmap(84, 2, bmpPusheen_2X_, 44, 30, BITMAP_INVERT, BUF_ADD);
  oled.setScale(0);
  delay(2000);

  Serial.begin(9600);
}


const char mainMenu[][15] = { "Select cell", "Change password", "Wipe data" };
const char passwordMenu[][15] = { "Type sequence", "Edit name", "Edit sequence" };


//ГЛАВНОЕ ДЕЙСТВИЕ
void mainAction() {
  if (!editField(password, true, "password", true)) {
    clearPassword();
    return;
  }
  loadCode();

  uint8_t choice = 1;
  while (choice) {
    switch (choice = showMenu(mainMenu, 3)) {
      case 1:
        for (;;) {
          if (cellMenu()) {
            uint8_t choice = 1;
            while (choice) {
              choice = showMenu(passwordMenu, 3);
              switch (choice) {
                case 1:
                  //Набрать последовательность
                  loadValue(cell, true);
                  oled.clear();
                  for (uint8_t i = 0; i < chSeqLen(sec_val, sizeof(sec_val)); i++) {
                    Keyboard.write(sec_val[i]);
                    oled.home();
                    oled.print(100 * (i + 1) / chSeqLen(sec_val, sizeof(sec_val)));
                    oled.print('%');
                    delay(50);
                  }
                  delay(500);
                  loadValue(cell);
                  break;
                case 2:
                  //Редактировать имя
                  {
                    uint8_t temp_val[40];
                    for (uint8_t i = 0; i < 40; i++) temp_val[i] = sec_val[i];
                    if (editField(sec_val, false, "name", false)) saveValue(cell);
                    else
                      for (uint8_t i = 0; i < 40; i++) sec_val[i] = temp_val[i];
                  }
                  break;
                case 3:
                  //Редактировать последовательность
                  {
                    loadValue(cell, true);
                    if (confirm() && editField(sec_val, true, "sequence", false)) saveValue(cell, true);
                    loadValue(cell);
                  }
                  break;
              }
            }
          } else break;
        }
        break;
      case 2:
        //Смена пароля с перешифровкой ключа
        {
          uint8_t pass_buf[sizeof(password)];
          for (uint8_t i = 0; i < sizeof(password); i++) pass_buf[i] = password[i];
          if (confirm() && editField(password, true, "password", false)) {
            saveCode();
          } else
            for (uint8_t i = 0; i < sizeof(password); i++) password[i] = pass_buf[i];
        }
        break;
      case 3:
        //Затирание всего
        if (confirm()) {
          oled.clear();
          oled.invertText(false);
          for (uint8_t i = 0; i < 32; i++) code[0][i] = random(256);

          uint8_t counter = 0;
          for (uint8_t i = 0; i < 16; i++) {
            for (uint8_t k = 0; k < 62; k++) {
              if (k < 30) EEPROM.write(32 + 62 * i + k, 0x00);
              else EEPROM.write(32 + 62 * i + k, random(256));
            }
            oled.home();
            oled.print((++counter) * 100 / 16);
            oled.print('%');
          }

          saveCode();
          delay(500);
        }
        break;
    }
  }
}


void loop() {
  mainAction();
}

void yield() {
  tickButtons();
}