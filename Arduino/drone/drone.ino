//////////////////////// Library ////////////////////////////
#include <MPU6050_tockn.h>
#include <Wire.h>
#include <WiFi101.h>

///////////////////////// TCP //////////////////////////////
// To connect to the server on laptop
char ssid[] = "iPhone";
char pass[] = "00000000";
int status = WL_IDLE_STATUS;

IPAddress server(172,20,10,3);
WiFiClient client;

int twobytes1int(byte high, byte low)
{
  int number;
  int sign;

  sign = high >> 7;
  high = high << 1;
  high = high >> 1;

  number = low | (high << 8);
  number = number*(pow(-1, sign));
  return number;
}

struct byteInt  {
    int number;
    byte high;
    byte low;
};

struct byteInt AngleX_send;
struct byteInt AngleY_send;
struct byteInt AngleZ_send;

void int2bytes(struct byteInt* number_fnc)  {
  int magnituteNumber;
  if (number_fnc->number >= 0)  {
    number_fnc->high = (number_fnc->number >> 8) & 0xFF;
    number_fnc->low = (number_fnc->number & 0xFF);
  } else {
    magnituteNumber = number_fnc->number * -1;
    number_fnc->high = (magnituteNumber >> 8) & 0xFF;
    number_fnc->high = number_fnc->high ^ 0x80;
    number_fnc->low = magnituteNumber & 0xFF;
  }
}

byte buf0[7]; // To store angle XYZ data

////////////////////////// MPU /////////////////////////////
//MPU6050 mpu6050(Wire);
MPU6050 mpu6050(Wire, 0.07, 0.93);

float angleX_offset = 0, angleY_offset = 0, angleZ_offset = 0; 
float angleX = 0, angleY = 0, angleZ = 0;

void update_angle_value() {
  mpu6050.update();
  angleX = mpu6050.getAngleX() - angleX_offset;
  angleY = mpu6050.getAngleY() - angleY_offset;
  angleZ = mpu6050.getAngleZ() - angleZ_offset;

  AngleX_send.number = (int) angleX;
  AngleY_send.number = (int) angleY;
  AngleZ_send.number = (int) angleZ;

  Serial.print("angleX : ");
  Serial.print(angleX);
  Serial.print(", angleY : ");
  Serial.print(angleY);
  Serial.print(", angleZ : ");
  Serial.println(angleZ);
}

///////////////////// Random variable //////////////////////
int i, j, k, a;

///////////////////// Setup and Loop ///////////////////////

void setup() {
  Serial.begin(9600);

  // Setup wifi connection
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
  }

  delay(3000); // If don't include this, it won't work
                // don't know why, can anyone figure out??
  
  // Setup server connection
  j = client.connect(server, 1234);
  while (j != 1)  {
    j = client.connect(server, 1234);
  }

  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  // Calculating offset
  
  for (int i = 0; i<1000; i++)  {
    mpu6050.update();
    angleX_offset = angleX_offset + mpu6050.getAngleX();
    angleY_offset = angleY_offset + mpu6050.getAngleY();
    angleZ_offset = angleZ_offset + mpu6050.getAngleZ();
  }

  angleX_offset = angleX_offset/1000.0;
  angleY_offset = angleY_offset/1000.0;
  angleZ_offset = angleZ_offset/1000.0;
}

void loop() {
  if (client.available()) {
    a = client.read();
    if (a == 0) { // Ask for ID.
      client.write((byte) 0);
    } else if (a == 1)  { // Ask for angle data
      int2bytes(&AngleX_send);
      int2bytes(&AngleY_send);
      int2bytes(&AngleZ_send);

      buf0[0] = AngleX_send.high;
      buf0[1] = AngleX_send.low;
      buf0[2] = AngleY_send.high;
      buf0[3] = AngleY_send.low;
      buf0[4] = AngleZ_send.high;
      buf0[5] = AngleZ_send.low;

      client.write(buf0,6);
    }
  }

  update_angle_value();
}
