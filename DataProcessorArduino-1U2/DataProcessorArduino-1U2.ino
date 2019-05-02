// Class:       ECE 4970W - Electrical and Computer Engineering Capstone
// Project:     An auto-stabilizing knee brace for physcial therapy and disability applications
// Written by:  John Walter

#include <SoftwareSerial.h>
#include <Servo.h>
#include <SoftwareServo.h>
#include "Wire.h"
// The two libraries belong to Jeff Rowberg and were taken from
// his GitHub repository: https://github.com/jrowberg/i2cdevlib.git
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

#define TCAADDR 0x70                // I2C address for the multiplexer
#define FORCE_SENSOR_REFERENCE 171  // Force sensor analog reading with no load

SoftwareSerial HB(10,11);           // Configures the bluetooth module on pins 10 and 11

int initialize = 0;
// ########### MULTIPLEXER SLOT SELECT ###########
// This function is used to select a slot on the multiplexer (1-8)
void tcaselect(uint8_t i) {
  if(i > 7){
    return;
  }
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}
// ###############################################

// ################## MPU CLASS ##################
// The MPU class creates an object for an MPU-6050 IMU module
// to allow interaction with the digital motion processor (DMP)
// on the module.
class MPU {
  MPU6050 *mpu;
  int dmp_status = 1;
  uint16_t dmp_packet_size = 0x0;
  uint16_t fifo_count = 0x0;
  uint8_t fifo_buffer[64];
  VectorFloat gravity;
  Quaternion q;
  uint8_t SLOT;
  int first=1;
  public:
    float ypr[3];
    int configure(uint8_t MPXR_SLOT);
    void getDMPData();
    void outputQuat();
    void outputYPR();
};

// This method configures an MPU-6050 module connected to
// a slot on the multiplexer (MPXR_SLOT) and sets it up.
int MPU::configure(uint8_t MPXR_SLOT){
  // save the slot within the object for use later
  SLOT=MPXR_SLOT;
  // select the corresponding slot on the multiplexer so that
  // we can interract with the desired device
  tcaselect(MPXR_SLOT);
  // make a new MPU6050 object
  mpu = new MPU6050();
  Serial.print("Initializing MPU: ");
  Serial.println(MPXR_SLOT);
  // initialize the object
  mpu->initialize();
  // verify that we can connect to it
  if(mpu->testConnection()){
    Serial.print(F("MPU initialization SUCCESSFULL: "));
    Serial.println(MPXR_SLOT);
    // initialize the DMP on the device
    dmp_status = mpu->dmpInitialize();
    if(dmp_status == 0){
      Serial.print(F("DMP initialization SUCCESSFUL: "));
      Serial.println(MPXR_SLOT);
      // Below configures the offsets for the gyroscope and accelerometer
      // specific to the device's datasheet 
      mpu->setDMPEnabled(true);
      mpu->setXGyroOffset(220);
      mpu->setYGyroOffset(76);
      mpu->setZGyroOffset(-85);
      mpu->setZAccelOffset(1788);
      // save the DMP's packet FIFO packet size for use later
      dmp_packet_size = mpu->dmpGetFIFOPacketSize();
    }else{
      Serial.print(F("DMP initialization SUCCESSFUL: "));
      Serial.println(MPXR_SLOT);
    }
  }else{
    Serial.print(F("MPU initialization UNSUCCESSFUL: "));
    Serial.println(MPXR_SLOT);
  }
}

// This method retrieves the FIFO packet from the DMP once full
// and then uses the DMP to process the packet and get the
// Quaternion and Euler angle data.
void MPU::getDMPData(){
  // If only one MPU object exists in the code, the code works as expected.
  // However, once more than one MPU object is created (two), the second 
  // created object experiences an initial buffer overflow in the DMP FIFO packet
  // and therefore will not process the data. To get around this, I force an initial
  // full packet (1023) during the first execution of this method to avoid the buffer
  // overflow. 
  if(first == 1){
    fifo_count = 1023;
    first++;
  }
  // select the device's multiplexer slot
  tcaselect(SLOT);
  // if the FIFO size is bigger than the DMP's packet size
  // (data is available)
  if(!(fifo_count < dmp_packet_size)){
    // if the FIFO size is 1024 (overflow) then reset the FIFO
    if(fifo_count == 1024){
      mpu->resetFIFO();
    // otherwise read the packet
    }else{
      // get FIFO count
      fifo_count = mpu->getFIFOCount();
      // reterive packet from DMP FIFO buffer
      mpu->getFIFOBytes(fifo_buffer,dmp_packet_size);
      // reset FIFO 
      mpu->resetFIFO();
      // reset fifo_count
      fifo_count -= dmp_packet_size;
      // use the fusion algorithms running in the firmware on the DMP
      // to convert the raw packet data to Quaternions and Euler angles
      // relative to gravity
      mpu->dmpGetQuaternion(&q, fifo_buffer);
      mpu->dmpGetGravity(&gravity, &q);
      mpu->dmpGetYawPitchRoll(ypr, &q, &gravity);
    }
  // otherise packet is not full, so retrieve buffer size and wait
  }else{
    fifo_count = mpu->getFIFOCount();
  }
}
// this method prints the Euler angles of the device
void MPU::outputYPR(){
  Serial.print(ypr[0] * 180/M_PI);
  Serial.print(F(",\t"));
  Serial.print(ypr[1] * 180/M_PI);
  Serial.print(F(",\t"));
  Serial.print(ypr[2] * 180/M_PI);
}
// this method sends the Quaternion vector of the device
// over the bluetooth serial connection
void MPU::outputQuat(){
  HB.print(q.w);
  HB.print(",");
  HB.print(q.x);
  HB.print(",");
  HB.print(q.y);
  HB.print(",");
  HB.print(q.z);
}
// ###############################################


// ############ FORCE SESNOR CLASS ###############
// This class configures and calibrates the force sensor object for accurate 
// force readings.
class ForceSensor{
  float actual_0 = 0; // known weight in LBS
  float actual_1 = 5; // known weight in LBS
  float measured_0 = FORCE_SENSOR_REFERENCE; // analog reading of for sensor
  float measured_1 = 185; // analog reading of force sensor
  float m=0;
  float b=0;
  int floating_ground = 0;
  int pin;
  public:
    void calibrateForceSensor(int pin);
    int measureLoad();
    int getStatus();
};

// This method calibrates the force sensor using analog readings of known weight
// to calculate the slope and y-intercept of the force sensor. It should be noted
// that this assumes linear functionality of the sensor. 
void ForceSensor::calibrateForceSensor(int p){
  pin = p;
  // To calibrate, there can be no load on the sensor, therefore, this
  // waits until there is no load on the sensor. 
  while(analogRead(pin) > FORCE_SENSOR_REFERENCE+50){
    Serial.println("Please remove all load to calibrate...");
    delay(1000);
  }
  delay(1000);
  Serial.println("Calibrating...");
  // calculate the slope of the sensor using the known values
  m = (actual_1 - measured_1) / (actual_0 - measured_0);
  // calculate the y-intercept 
  b = actual_1 - (m * actual_0);
  // because the sensor doesn't actual read 0 at no load, this must be subtracted off,
  // so this takes a reading of the sensor, calculates its actual value using m & b,
  // and then saves it relative to the object
  floating_ground = (analogRead(pin) * m) - b;
  Serial.println("Done.");
}
// This method reads the sensor and calculates the actual weight on the load
// using the slope and y-intercept. Also, instead of sampling once, this method
// takes 20 samples and then calculates the average to provide a more stable reading
int ForceSensor::measureLoad(){
  int i = 0;
  int total = 0;
  while(i<20){
    float m_force = analogRead(pin);
    total = total+((m * m_force) - b - floating_ground);
    i++;
  }
  return total/20;
}

// Specific to the project, it is important to know when the load on the sensor
// exceeds 10 lbs, therefore, this method returns a 1 is this is true. Otherwise
// it returns a 0
int ForceSensor::getStatus(){
  if(measureLoad() > 10){
    return 1;
  }else{
    return 0;
  }
}
// ###############################################

MPU mpu1, mpu2;
ForceSensor fs;
SoftwareServo serv1, serv2;
char data = 0;
int compress_level = 0;

void setup() {
  serv1.attach(5); // left servo pin 
  serv2.attach(6); // right servo pin
  serv1.write(0);
  serv2.write(0);
  // wait for serial connection setup
  while(!Serial);
  delay(1000);
  // begin wire transmission 
  Wire.begin();
  // begin serial communication with baud rate of 9600
  Serial.begin(9600);
  // begin bluetooth serial connection with default baud rate of 9600
  HB.begin(9600);
  // configure both MPU6050 devices
  mpu1.configure(4); // lower leg
  mpu2.configure(1); // upper leg
  // calibrate the force sensor on analog pin A0
  fs.calibrateForceSensor(A0);

  // the status pin of the bluetooth module is connected to pin 7 on the arduino
  // and tells the arduino whether or not the module is connected to another device,
  // therefore this waits until the module is connected
  pinMode(7, INPUT);
    while (digitalRead(7) == LOW){
    Serial.println("ERROR - No bluetooth connection. Waiting...");
  }
  // below waits for two things over bluetooth from the main program running on a computer.
  // It frist waits to be sent the compression level (0-9), and then it waits for an 'S' which
  // tells this program to begin execution (the main loop)
  while(1){
    data = HB.read();
    if(data == 'S'){
      //HB.println("Executing...");
      break;
    }
    switch(data){
      case '1':
        compress_level = 1; // 10% min compression
        break;
      case '2':
        compress_level = 2; // 20%
        break;
      case '3':
        compress_level = 3;
        break;
      case '4':
        compress_level = 4;
        break;
      case '5':
        compress_level = 5;
        break;
      case '6':
        compress_level = 6;
        break;
      case '7':
        compress_level = 7;
        break;
      case '8':
        compress_level = 8; // 80%
        break;
      case '9':
        compress_level = 9; // 90%
        break;
      case '0':
        compress_level = 10; // 100% max compression
        break;
    }
    delay(100);
  }
}

// This function sends a packet of data containing everything important over bluetooth
// to the external device. This includes each device's Euler angles, quaternion values,
// the angle difference of the rolls of the devices, and the load on the force sensor
void dispAllOut(){
  mpu1.outputQuat();
  HB.print(",");
  mpu2.outputQuat();
  HB.print(",");
  HB.print((mpu1.ypr[0]*180/M_PI));
  HB.print(",");
  HB.print((mpu2.ypr[0]*180/M_PI));
  HB.print(",");
  HB.print((mpu1.ypr[1]*180/M_PI));
  HB.print(",");
  HB.print((mpu2.ypr[1]*180/M_PI));
  HB.print(",");
  HB.print((mpu1.ypr[2]*180/M_PI));
  HB.print(",");
  HB.print((mpu2.ypr[2]*180/M_PI));
  HB.print(",");
  HB.print((mpu2.ypr[2]*180/M_PI)-(mpu1.ypr[2]*180/M_PI)); // calculate angle between rolls
  HB.print(",");
  HB.println(fs.measureLoad());
}

void loop() {
  // calculate the angle to position the servos during compression. This uses the received compression level
  // as a percentage of the maximum angle the servos can rotate (180 degrees) -> 0 degrees = no compression &
  // 180 degrees = maximum compression.
  float angle = 180*compress_level/10; 
  // calculate the angle difference of the rolls of the two IMU modules. This tells us the angle bend of the knee.
  float d_angle = (mpu2.ypr[2]*180/M_PI)-(mpu1.ypr[2]*180/M_PI);
  // get the new DMP data for both devices
  mpu1.getDMPData(); // lower leg MPU6050
  mpu2.getDMPData(); // upper leg MPU6050
  // send the new packet of data over bluetooth
  dispAllOut();
  
  // It should be noted that because the servos are positioned with the same orientation but on opposite
  // sides of the brace, the angle of rotation differs by 180 degrees between the two.
  // # Serv1 rotates from 180 degrees (min compression) to 0 degrees (max compression)
  // # Serv2 rotates from 0 degrees (min compression) to 180 degrees (max compression)

  // if the load on the force sensor is greater than 10lbs, actuate compression by positioining the servos to the angle
  // calculated above. 
  if(fs.getStatus() == 1){
    serv1.write(180-angle); // right side servo when looking at the brace
    serv2.write(angle);     // left side
  }
  // otherwise, if the user is sitting and extending the leg outward (force=0, bend of leg < 80 degrees (extending leg outward), upper leg's
  // roll < 50 degrees) then actuate compression.
  // # When standing (the two IMU sensors are in parallel) the d_angle ~0 and the roll of the upper leg's IMU is ~-100 degrees
  // # When sitting with bent knee (the two IMU sensors are perpendicular) the d_angle is ~90 and the roll of the upper leg's IMU is ~0 degrees
  // # When sitting with extended knee (the two IMU sensors are parallel) the d_angle is ~0 and the roll of the upper leg's IMU is ~0 degrees
  else if((fs.getStatus() == 0) & (d_angle < 80) & ((mpu2.ypr[2]*180/M_PI)*-1 < 50)){
    serv1.write(180-angle); // right side servo when looking at the brace
    serv2.write(angle);
  // otherwise, no compression is needed
  }else{
    serv1.write(180);
    serv2.write(0);
  }
  // refresh servo positions for more accurate results
  SoftwareServo::refresh();
  // Sampling rate of 100ms, or 10Hz
  delay(100);
}
