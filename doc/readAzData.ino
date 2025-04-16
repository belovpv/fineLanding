//Код примера чтения (Arduino совместимый, порядок Big Endian):
//Serial2 - входящий порт данных
//Serial - вывод информации

//Если данные не поступают в течение 500 мс или значения выходят за допустимые пределы, буфер очищается, и процесс чтения начинается заново.

#include <limits.h> // Подключаем заголовочный файл для ULONG_MAX

HardwareSerial Serial2(PA3,PA2); //для плат на базе STM32F103, например

// Определение структуры AzData
struct AzData {
  uint16_t DistanceInM;   // Расстояние в метрах (тип Word, 2 байта)
  uint16_t AzimuthX100;   // Азимут, умноженный на 100 (тип Word, 2 байта)
};

// Функция чтения данных из Serial2 с таймаутом и проверкой значений
void readSerialData() {
  static uint8_t buffer[4]; // Буфер для хранения 4 байт данных
  static uint8_t index = 0; // Индекс для записи в буфер
  static unsigned long lastByteTime = 0; // Время получения последнего байта

  while (Serial2.available()) { // Пока есть данные в Serial2
    if (index>3) {
		Serial.println("Ошибка размера буфера");
        index = 0; // Очищаем буфер
        return;    // Начинаем заново
	}
	buffer[index] = Serial2.read(); // Читаем байт и записываем в буфер
    index++;
	lastByteTime = millis(); // Обновляем время получения последнего байта
	


    if (index == 4) { // Если набралось 4 байта
      AzData InDat;
      // Заполняем поля структуры через High Byte и Low Byte (или memcpy(&InDat, buffer, sizeof(InDat)); при совместимости)
      InDat.DistanceInM = ((uint16_t)buffer[0] << 8) | buffer[1]; // Первые 2 байта - DistanceInM
      InDat.AzimuthX100 = ((uint16_t)buffer[2] << 8) | buffer[3]; // Последние 2 байта - AzimuthX100

      // Проверка допустимых значений, 360 градусов и больше 20 километров расстояние
      if (InDat.AzimuthX100 > 36000 || InDat.DistanceInM > 20000) {
        Serial.println("Ошибка диапазона данных");
        index = 0; // Очищаем буфер
        return;    // Начинаем заново
      }

      // Вывод полученных данных в человекочитаемом виде
      Serial.print("Получены данные: Дистанция = ");
      Serial.print(InDat.DistanceInM);
      Serial.print(" м, Азимут = ");
      Serial.print(InDat.AzimuthX100 / 100.0); // Переводим обратно в градусы
      Serial.println(" град");

      index = 0; // Сбрасываем индекс для следующего пакета
      return;    // Выходим из функции
    }
  }

  // Проверка таймаута (500 мс), учитывая переполнение millis()
  if (index > 0 && (millis() - lastByteTime > 500 || (lastByteTime > millis() && (ULONG_MAX - lastByteTime + millis() > 500)))) {
    Serial.println("Слишком долгое чтение данных, ошибка приема");
    index = 0; // Очищаем буфер
  }
}

void setup() {  // Инициализация последовательных портов
  Serial.begin(9600);  // Для отладки
  Serial2.begin(9600); // Для приема данных
}

void loop() {
  readSerialData(); // Чтение данных из Serial2
}