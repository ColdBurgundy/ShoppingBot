#include <usbhid.h>
#include <usbhub.h>
#include <hiduniversal.h>
#include <hidboot.h>
#include "SPI.h"
//#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <SoftwareSerial.h>


String incomingData = "";  // 수신된 데이터를 저장할 문자열
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
const int gridWidth = 10;
const int gridHeight = 12;

//격자 크기
const int NUM_COLS = 25; // 열 수
const int NUM_ROWS = 25; // 행 수

// 격자 상태를 저장하는 2차원 배열
bool gridBlocked[NUM_ROWS][NUM_COLS] = { false };

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

```
// If enter signal emitted, scan finished
if (buf[i] == UHS_HID_BOOT_KEY_ENTER) {
  OnScanFinished();
}

// If not, continue normally
else {

  OnKeyScanned(i > 2, buf, buf[i]);
}

return;

```

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
int gridX = 3;
int gridY = 14;

// 길 정보를 격자에 적용하는 함수
void applyRoadMap() {
int roadMap[25][25];
int i, j;
for(i=0; i<25; i++)
{
for(j=0; j<25; j++) roadMap[i][j] = -1;
}
for (j = 2; j <= 6; j++) roadMap[2][j] = 0;
for (i = 3; i <= 4; i++) roadMap[i][6] = 0;
for (j = 7; j <= 14; j++) roadMap[4][j] = 0;
for (j = 15; j <= 23; j++) roadMap[2][j] = 0;
for (j = 18; j <= 23; j++) roadMap[6][j] = 0;
for (j = 2; j <= 18; j++) roadMap[9][j] = 0;
for (j = 7; j <= 14; j++) roadMap[11][j] = 0;
for (j = 10; j <= 13; j++) roadMap[13][j] = roadMap[15][j] = roadMap[17][j] = roadMap[19][j] = 0;
for (i = 3; i <= 8; i++) roadMap[i][2] = roadMap[i][15] = roadMap[i][18] = 0;
for (i = 3; i <= 5; i++) roadMap[i][23] = 0;
for (i = 13; i <= 19; i++) roadMap[i][9] = roadMap[i][14] = 0;
for (i = 10; i <= 20; i++)
{
for (j = 4; j <= 6; j++)
roadMap[i][j] = 0;
}
for (i = 10; i <= 17; i++)
{
for (j = 15; j <= 18; j++)
roadMap[i][j] = 0;
}
for (i = 18; i <= 20; i++)
{
for (j = 17; j <= 18; j++)
roadMap[i][j] = 0;
}
roadMap[14][3] =0;
// MapGenerator 클래스에서 생성된 길 정보를 아래와 같이 배열로 변환

for (int i = 0; i < 25; i++) {
    for (int j = 0; j < 25; j++) {
        if (roadMap[i][j] == 0) {
            gridBlocked[i][j] = false; // 길은 이동 가능
        } else {
            gridBlocked[i][j] = true; // 길이 아니면 이동 불가능
        }
    }
}

}
void changeGridCellColor(int x, int y, uint16_t color) {
tft_joystick.fillRect(y * gridWidth, x * gridHeight, gridWidth, gridHeight, color);
tft_joystick.drawRect(y * gridWidth, x * gridHeight, gridWidth, gridHeight, ILI9341_WHITE);
}

const int NUM_POINTS_1 = 61; // 좌표 개수
int points_1[NUM_POINTS_1][2] = {
{14, 3}, {14, 4}, {14, 5}, {13, 5}, {12, 5}, {11, 5}, {10, 5}, {9, 5}, {9, 6},
{9, 7}, {9, 8}, {9, 9}, {9, 10}, {9, 11}, {9, 12}, {9, 13}, {9, 14}, {9, 15},
{9, 16}, {9, 17}, {9, 18}, {8, 18}, {7, 18}, {6, 18}, {5, 18}, {4, 18}, {3, 18},
{4, 18}, {5, 18}, {6, 18}, {6, 19}, {6, 20},
{6, 19}, {6, 18}, {7, 18}, {8, 18}, {9, 18}, {10, 18}, {11, 18}, {12, 18}, {13, 18},
{13, 17}, {13, 16}, {13, 15}, {13, 14}, {13, 13}, {13, 12}, {13, 11},
{13, 12},{13, 13},{13, 14},{13, 15},{13, 16},{13, 17},{14, 17},{15, 17},{16, 17},{17, 17},{18, 17},{19, 17},{20, 17}
};
const int NUM_POINTS_2 = 47; // 2번 데이터의 좌표 개수
int points_2[NUM_POINTS_2][2] = {
{14,3}, {14,4}, {13,4}, {12,4}, {11,4}, {10,4}, {9,4}, {9,5}, {9,6}, {9,7}, {9,8}, {9,9}, {9,10}, {9,11}, {9,12}, {9,13}, {9,14}, {9,15}, {9,16}, {9,17}, {9,18}, {8,18}, {7,18}, {6,18}, {6,19}, {6,20}, {6,21}, {6,22}, {6,23}, {6,22}, {6,21}, {6,20}, {6,19}, {6,18}, {7,18}, {8,18}, {9,18}, {10,18}, {10,17}, {11,17}, {12,17}, {13,17}, {14,17}, {15,17}, {16,17}, {17,17}, {18,17} /* 나머지 좌표들도 이곳에 넣으세요 */
};
const int NUM_POINTS_3 = 57; // 좌표 개수
int points_3[NUM_POINTS_3][2] = {{14,3}, {14,4}, {13,4}, {12,4}, {11,4}, {10,4}, {9,4}, {9,3}, {9,2}, {8,2}, {7,2}, {6,2}, {5,2}, {4,2}, {3,2}, {2,2}, {2,3}, {2,4}, {2,5}, {2,6}, {3,6}, {4,6}, {4,7}, {4,8}, {4,9}, {4,10}, {4,11}, {4,12}, {4,13}, {4,14}, {4,15}, {3,15}, {2,15}, {2,16}, {2,17}, {2,18}, {3,18}, {4,18}, {5,18}, {6,18}, {7,18}, {8,18}, {9,18}, {10,18}, {11,18}, {12,18}, {13,18}, {13,17}, {13,16}, {13,15}, {13,14}, {13,13}, {13,12}};
void processReceivedData(String data) {

Serial.print("수신된 데이터: ");
Serial.println(data);
// 1번 데이터에 대한 경로 그리기
if (data == "8 H003 1 8 H001 1 13 M001 1 14 N002 1") {
delay(5000);
clearScreenAndRedrawGrid();
for (int i = 0; i < NUM_POINTS_1; i++) {
int x = points_1[i][0];
int y = points_1[i][1];
changeGridCellColor(x, y, ILI9341_YELLOW);
}
}
// 2번 데이터에 대한 경로 그리기
else if (data == "11 K002 1 7 G001 1 14 N001 1 9 I001 1") {
delay(5000);
clearScreenAndRedrawGrid();
for (int i = 0; i < NUM_POINTS_2; i++) {
int x = points_2[i][0];
int y = points_2[i][1];
changeGridCellColor(x, y, ILI9341_BLUE);
}
}
else {
delay(5000);
clearScreenAndRedrawGrid();
for (int i = 0; i < NUM_POINTS_3; i++) {
int x = points_3[i][0];
int y = points_3[i][1];
changeGridCellColor(x, y, ILI9341_GREEN);
}
}
// 5초간 대기

// 화면 초기화 및 다시 그리드 그리기

}
// 화면을 초기화하고 격자를 다시 그리는 함수
void clearScreenAndRedrawGrid() {
tft_joystick.fillScreen(ILI9341_BLACK); // 화면을 검은색으로 채움
drawGrid(); // 격자 다시 그리기
}
void drawGrid() {
for (int i = 0; i < 25; i++) {
for (int j = 0; j < 25; j++) {
tft_joystick.drawRect(i * gridWidth, j * gridHeight, gridWidth, gridHeight, ILI9341_WHITE);
}

}

blockGridCells();

}

void highlightGrid(int x, int y, bool highlight) {
uint16_t color = highlight ? ILI9341_RED : ILI9341_BLACK;
tft_joystick.fillRect(x * gridWidth, y * gridHeight, gridWidth, gridHeight, color);
tft_joystick.drawRect(x * gridWidth, y * gridHeight, gridWidth, gridHeight, ILI9341_WHITE);
}



//그리드 차단 코드 추가
bool updateGridPosition(int xVal, int yVal) {
int newX = gridX;
int newY = gridY;

if (xVal < 300) {
newX = max(gridX - 1, 0);
} else if (xVal > 700) {
newX = min(gridX + 1, NUM_COLS - 1);
}

if (yVal < 300) {
newY = max(gridY - 1, 0);
} else if (yVal > 700) {
newY = min(gridY + 1, NUM_ROWS - 1);
}

// 이동하려는 셀이 차단되지 않았는지 확인
if (!gridBlocked[newY][newX]) {
if (newX != gridX || newY != gridY) {
highlightGrid(gridX, gridY, false); // 이전 위치의 색을 지움
gridX = newX;
gridY = newY;
return true;
}
}

return false;
}

// //추가
// // 격자 중 일부를 흰색으로 채워 차단하는 함수
void blockGridCells() {
int roadMap[25][25];
int i, j;
for(i=0; i<25; i++)
{
for(j=0; j<25; j++) roadMap[i][j] = -1;
}
for (j = 2; j <= 6; j++) roadMap[2][j] = 0;
for (i = 3; i <= 4; i++) roadMap[i][6] = 0;
for (j = 7; j <= 14; j++) roadMap[4][j] = 0;
for (j = 15; j <= 23; j++) roadMap[2][j] = 0;
for (j = 18; j <= 23; j++) roadMap[6][j] = 0;
for (j = 2; j <= 18; j++) roadMap[9][j] = 0;
for (j = 7; j <= 14; j++) roadMap[11][j] = 0;
for (j = 10; j <= 13; j++) roadMap[13][j] = roadMap[15][j] = roadMap[17][j] = roadMap[19][j] = 0;
for (i = 3; i <= 8; i++) roadMap[i][2] = roadMap[i][15] = roadMap[i][18] = 0;
for (i = 3; i <= 5; i++) roadMap[i][23] = 0;
for (i = 13; i <= 19; i++) roadMap[i][9] = roadMap[i][14] = 0;
for (i = 10; i <= 20; i++)
{
for (j = 4; j <= 6; j++)
roadMap[i][j] = 0;
}
for (i = 10; i <= 17; i++)
{
for (j = 15; j <= 18; j++)
roadMap[i][j] = 0;
}
for (i = 18; i <= 20; i++)
{
for (j = 17; j <= 18; j++)
roadMap[i][j] = 0;
}
roadMap[14][3] =0;

// 차단된 셀을 흰색으로 채움
for (int i = 0; i < 25; i++) {
for (int j = 0; j < 25; j++) {
if (roadMap[i][j]==0) {
tft_joystick.fillRect(j * gridWidth, i * gridHeight,gridWidth, gridHeight, ILI9341_WHITE);
}
else {
// 길이 아닌 부분은 흰색으로 채움
tft_joystick.fillRect(j * gridWidth, i * gridHeight, gridWidth, gridHeight, ILI9341_BLACK);
}
}
}
}
void setup() {
Serial.begin(9600);  // 시리얼 모니터 통신 속도 설정
Serial1.begin(9600); // 블루투스 통신 속도 설정
Serial.println("블루투스 대기 중...");
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
applyRoadMap(); // 격자에 길 정보 적용

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

// 블루투스 연결 상태 확인
if (Serial1.available()) {
Serial.println("블루투스 장치 연결됨");
String receivedData = "";
while (Serial1.available()) {
char receivedChar = Serial1.read(); // 데이터 읽기
receivedData += receivedChar;
Serial.print(receivedChar); // 시리얼 모니터에 출력
}
// 수신된 데이터를 가지고 함수 호출
processReceivedData(receivedData);
}
}