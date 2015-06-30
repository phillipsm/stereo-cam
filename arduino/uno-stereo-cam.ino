// This is the logic to snap pics from four Adafruit VC0706
// cameras and write the camera buffers to an adafruit SD
// breakout.
//
// Thanks, https://github.com/adafruit/Adafruit-VC0706-Serial-Camera-Library

#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>         

// Our SD interface
#define chipSelect 10

// Our files get a common filename prefix, things like 12-A.JPG
int filename_prefix;

// Camera a
SoftwareSerial cam_conn_a = SoftwareSerial(2, 3);
Adafruit_VC0706 cam_a = Adafruit_VC0706(&cam_conn_a);

// Camera b
SoftwareSerial cam_conn_b = SoftwareSerial(4, 5);
Adafruit_VC0706 cam_b = Adafruit_VC0706(&cam_conn_b);

// Camera c
SoftwareSerial cam_conn_c = SoftwareSerial(6, 7);
Adafruit_VC0706 cam_c = Adafruit_VC0706(&cam_conn_c);

// Camera c
SoftwareSerial cam_conn_d = SoftwareSerial(8, 9);
Adafruit_VC0706 cam_d = Adafruit_VC0706(&cam_conn_d);


// Our shutter button and shutter button led
int button_pin = 14;
int button_pin_led = 16;

  
void setup() {
  
  // Send data to our SD
  pinMode(10, OUTPUT);
    
  Serial.begin(9600);
  
  // See if the card is present and can be initialized:
  SD.begin(chipSelect);

  // Shutter
  pinMode(button_pin, INPUT);
  pinMode(button_pin_led, OUTPUT);
  digitalWrite(button_pin_led, 1);
  
}

void loop() {

  if (digitalRead(button_pin)==HIGH){ //if button pressed
      digitalWrite(button_pin_led, 0);
      take_picture();
      digitalWrite(button_pin_led, 1);
  }
}

int get_filname_prefix(SDClass& SD) {
  // Each set of photos gets a common filename prefix.
  // Find a unique one in this function
  
  char filename_candidate[9] = {0};
  for (int i = 0; i < 100; i++) {
    sprintf(filename_candidate, "%d-A.JPG", i);
    if (! SD.exists(filename_candidate)) {
      return i;
    }
  }
}

void write_cam_buffer_to_file(SDClass& SD, char camera_position, Adafruit_VC0706& cam) {
  
  char filename[9] = {0};
  sprintf(filename, "%d-%c.JPG", filename_prefix, camera_position);
  
  // Open the file for writing
  File imgFile = SD.open(filename, FILE_WRITE);

  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();  
  
  // Read all the data up to # bytes!
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);
    jpglen -= bytesToRead;
    if (jpglen % 100 == 0) {
      digitalWrite(button_pin_led, 1);
    } else {
      digitalWrite(button_pin_led, 0);
    }
    
  }
  imgFile.close();
}




void take_picture () {
  
  // Get a new filename prefix for set of photos
  filename_prefix = get_filname_prefix(SD);
  
  // Take pic with camera a
  cam_a.begin();
  cam_a.getVersion(); // getVersion seems to be required?
  cam_a.setImageSize(VC0706_640x480);
  cam_a.takePicture();
  
  // Take pic with camera b
  cam_conn_b.listen();
  cam_b.begin();
  cam_b.getVersion();
  cam_b.setImageSize(VC0706_640x480);
  cam_b.takePicture();
    
  // Take pic with camera c
  cam_conn_c.listen();
  cam_c.begin();
  cam_c.getVersion();
  cam_c.setImageSize(VC0706_640x480);
  cam_c.takePicture();
    
  // Take pic with camera d
  cam_conn_d.listen();
  cam_d.begin();
  cam_d.getVersion();
  cam_d.setImageSize(VC0706_640x480);
  cam_d.takePicture();

  // Cameras should have photos in their buffers. Let's write them to the SD.
  cam_conn_a.listen();
  write_cam_buffer_to_file(SD, 'A', cam_a);

  cam_conn_b.listen();
  write_cam_buffer_to_file(SD, 'B', cam_b);
  
  cam_conn_c.listen();
  write_cam_buffer_to_file(SD, 'C', cam_c);
  
  cam_conn_d.listen();
  write_cam_buffer_to_file(SD, 'D', cam_d);
  
}
