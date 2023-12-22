#include <usbhid.h>
#include <usbhub.h>
#include <hiduniversal.h>
#include <hidboot.h>
#include "SPI.h"
//#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
 
// 바코드 스캐너 LCD 핀 설정
#define TFT_DC_BARCODE 9
#define TFT_CS_BARCODE 7
#define TFT_RST_BARCODE 8
#define TFT_MOSI_BARCODE 3
#define TFT_MISO_BARCODE 4
#define TFT_CLK_BARCODE 5

// 조이스틱 LCD 핀 설정
#define TFT_CS_JOYSTICK 28
#define TFT_RST_JOYSTICK 32  // 다른 리셋 핀 사용
#define TFT_DC_JOYSTICK 26
#define TFT_MOSI_JOYSTICK 3  // 공통 MOSI 핀 사용
#define TFT_MISO_JOYSTICK 4  // 공통 MISO 핀 사용
#define TFT_CLK_JOYSTICK 5  // 공통 CLK 핀 사용
#define JOY_VRX    A8
#define JOY_VRY    A9
#define JOY_SW     30

// 격자 크기
const int gridWidth = 60;
const int gridHeight = 80;

// 두 LCD 인스턴스 생성
Adafruit_ILI9341 tft_barcode = Adafruit_ILI9341(TFT_CS_BARCODE, TFT_DC_BARCODE, TFT_MOSI_BARCODE, TFT_CLK_BARCODE, TFT_RST_BARCODE, TFT_MISO_BARCODE);
Adafruit_ILI9341 tft_joystick = Adafruit_ILI9341(TFT_CS_JOYSTICK, TFT_DC_JOYSTICK, TFT_MOSI_JOYSTICK, TFT_CLK_JOYSTICK, TFT_RST_JOYSTICK, TFT_MISO_JOYSTICK);

// 바코드 데이터 변수
String barcodeData = "";

// 여기에 MyParser 클래스와 기타 바코드 스캐너 관련 함수들을 포함시킵니다.
class MyParser : public HIDReportParser {
  public:
    MyParser();
    void Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);
  protected:
    uint8_t KeyToAscii(bool upper, uint8_t mod, uint8_t key);
    virtual void OnKeyScanned(bool upper, uint8_t mod, uint8_t key);
    virtual void OnScanFinished();
};

MyParser::MyParser() {}

void MyParser::Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
  // If error or empty, return
  if (buf[2] == 1 || buf[2] == 0) return;

  for (uint8_t i = 7; i >= 2; i--) {
    // If empty, skip
    if (buf[i] == 0) continue;

    // If enter signal emitted, scan finished
    if (buf[i] == UHS_HID_BOOT_KEY_ENTER) {
      OnScanFinished();
    }

    // If not, continue normally
    else {
      // If bit position not in 2, it's uppercase words
      OnKeyScanned(i > 2, buf, buf[i]);
    }

    return;
  }
}

uint8_t MyParser::KeyToAscii(bool upper, uint8_t mod, uint8_t key) {
  // Letters
  if (VALUE_WITHIN(key, 0x04, 0x1d)) {
    if (upper) return (key - 4 + 'A');
    else return (key - 4 + 'a');
  }

  // Numbers
  else if (VALUE_WITHIN(key, 0x1e, 0x27)) {
    return ((key == UHS_HID_BOOT_KEY_ZERO) ? '0' : key - 0x1e + '1');
  }

  return 0;
}

void MyParser::OnKeyScanned(bool upper, uint8_t mod, uint8_t key) {
  uint8_t ascii = KeyToAscii(upper, mod, key);
  Serial.print((char)ascii);

  if (ascii != 0) {
    barcodeData += ((char)ascii);
  }

}

void MyParser::OnScanFinished() {
  Serial.println(" - Finished");
  
  delay(100);
  tft_barcode.println("Barcode: ");
  tft_barcode.println(barcodeData);
  Serial.println("LCD Displayed");
  barcodeData = ""; // 데이터 리셋
}
USB          Usb;
USBHub       Hub(&Usb);
HIDUniversal Hid(&Usb);
MyParser     Parser;

// 격자 위치 변수
int gridX = 0;
int gridY = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Start");
  
  // USB 초기화
  if (Usb.Init() == -1) {
    Serial.println("OSC did not start.");
  }
  delay(1000);

  // 바코드 스캐너 LCD 초기화
  tft_barcode.begin();
  tft_barcode.setCursor(0, 0);
  tft_barcode.setTextSize(3);
  tft_barcode.setTextColor(ILI9341_BLACK);

  // 조이스틱 LCD 초기화
  tft_joystick.begin();
  tft_joystick.fillScreen(ILI9341_BLACK);
  drawGrid();

  Hid.SetReportParser(0, &Parser);
}

void loop() {
  // 바코드 스캐너 작업
  Usb.Task();

  // 조이스틱 입력 처리
  int xVal = analogRead(JOY_VRX);
  int yVal = analogRead(JOY_VRY);
  if (updateGridPosition(xVal, yVal)) {
    highlightGrid(gridX, gridY, true);
  }
  delay(100);
}

void drawGrid() {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      tft_joystick.drawRect(i * gridWidth, j * gridHeight, gridWidth, gridHeight, ILI9341_WHITE);
    }
  }
}

void highlightGrid(int x, int y, bool highlight) {
  uint16_t color = highlight ? ILI9341_RED : ILI9341_BLACK;
  tft_joystick.fillRect(x * gridWidth, y * gridHeight, gridWidth, gridHeight, color);
  tft_joystick.drawRect(x * gridWidth, y * gridHeight, gridWidth, gridHeight, ILI9341_WHITE);
}

bool updateGridPosition(int xVal, int yVal) {
  int prevX = gridX;
  int prevY = gridY;

  if (xVal < 300) {
    gridX = max(gridX - 1, 0);
  } else if (xVal > 700) {
    gridX = min(gridX + 1, 3);
  }

  if (yVal < 300) {
    gridY = max(gridY - 1, 0);
  } else if (yVal > 700) {
    gridY = min(gridY + 1, 3);
  }

  if (gridX != prevX || gridY != prevY) {
    highlightGrid(prevX, prevY, false); // 이전 위치의 색을 지움
    return true;
  }
  
  return false;
}
