# import struct

# class HTS221:
#     def _init_(self, i2c, data_rate=1, address=0x5F):
#         self.bus = i2c
#         self.odr = data_rate
#         self.slv_addr = address

#         # Set configuration register
#         # Humidity and temperature average configuration
#         self.bus.writeto_mem(self.slv_addr, 0x10, b"\x1B")

#         # Set control register
#         # PD | BDU | ODR
#         cfg = 0x80 | 0x04 | (self.odr & 0x3)
#         self.bus.writeto_mem(self.slv_addr, 0x20, bytes([cfg]))

#         # Read Calibration values from non-volatile memory of the device
#         # Humidity Calibration values
#         self.H0 = self._read_reg(0x30, 1) / 2
#         self.H1 = self._read_reg(0x31, 1) / 2
#         self.H2 = self._read_reg(0x36, 2)
#         self.H3 = self._read_reg(0x3A, 2)

#         # Temperature Calibration values
#         raw = self._read_reg(0x35, 1)
#         self.T0 = ((raw & 0x03) * 256) + self._read_reg(0x32, 1)
#         self.T1 = ((raw & 0x0C) * 64) + self._read_reg(0x33, 1)
#         self.T2 = self._read_reg(0x3C, 2)
#         self.T3 = self._read_reg(0x3E, 2)

#     def _read_reg(self, reg_addr, size):
#         fmt = "B" if size == 1 else "H"
#         reg_addr = reg_addr if size == 1 else reg_addr | 0x80
#         return struct.unpack(fmt, self.bus.readfrom_mem(self.slv_addr, reg_addr, size))[0]

#     def humidity(self):
#         rH = self._read_reg(0x28, 2)
#         return (self.H1 - self.H0) * (rH - self.H2) / (self.H3 - self.H2) + self.H0

#     def temperature(self):
#         temp = self._read_reg(0x2A, 2)
#         if temp > 32767:
#             temp -= 65536
#         return ((self.T1 - self.T0) / 8.0) * (temp - self.T2) / (self.T3 - self.T2) + (
#             self.T0 / 8.0 )

#include <dht.h>



#define DHT22_PIN 5

struct {
  uint32_t total;
  uint32_t ok;
  uint32_t crc_error;
  uint32_t time_out;
  uint32_t connect;
  uint32_t ack_l;
  uint32_t ack_h;
  uint32_t unknown;
} stat = { 0, 0, 0, 0, 0, 0, 0, 0};

void setup() {
  Serial.begin(115200);
  Serial.println("dht22_test.ino");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHT_LIB_VERSION);
  Serial.println();
  Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)\tTime (us)");
}

void loop() {
  // READ DATA
  Serial.print("DHT22, \t");

  uint32_t start = micros();
  int chk = DHT.read22(DHT22_PIN);
  uint32_t stop = micros();

  stat.total++;
  switch (chk)
  {
    case DHTLIB_OK:
      stat.ok++;
      Serial.print("OK,\t");
      break;
    case DHTLIB_ERROR_CHECKSUM:
      stat.crc_error++;
      Serial.print("Checksum error,\t");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      stat.time_out++;
      Serial.print("Time out error,\t");
      break;
    case DHTLIB_ERROR_CONNECT:
      stat.connect++;
      Serial.print("Connect error,\t");
      break;
    case DHTLIB_ERROR_ACK_L:
      stat.ack_l++;
      Serial.print("Ack Low error,\t");
      break;
    case DHTLIB_ERROR_ACK_H:
      stat.ack_h++;
      Serial.print("Ack High error,\t");
      break;
    default:
      stat.unknown++;
      Serial.print("Unknown error,\t");
      break;
  }
  // DISPLAY DATA
  Serial.print(DHT.humidity, 1);
  Serial.print(",\t");
  Serial.print(DHT.temperature, 1);
  Serial.print(",\t");
  Serial.print(stop - start);
  Serial.println();

  if (stat.total % 20 == 0)
  {
    Serial.println("\nTOT\tOK\tCRC\tTO\tCON\tACK_L\tACK_H\tUNK");
    Serial.print(stat.total);
    Serial.print("\t");
    Serial.print(stat.ok);
    Serial.print("\t");
    Serial.print(stat.crc_error);
    Serial.print("\t");
    Serial.print(stat.time_out);
    Serial.print("\t");
    Serial.print(stat.connect);
    Serial.print("\t");
    Serial.print(stat.ack_l);
    Serial.print("\t");
    Serial.print(stat.ack_h);
    Serial.print("\t");
    Serial.print(stat.unknown);
    Serial.println("\n");
  }
  delay(2000);
}
