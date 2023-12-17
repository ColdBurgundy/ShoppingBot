#include <iostream>
#include <vector>
#include <queue>
#include <set>
using namespace std;

const int MAX_ITEMS = 10; // 한 칸에 들어갈 수 있는 상품의 최대 개수
const int WIDTH = 25;
const int HEIGHT = 25;
const int CATEGORY_COUNT = 18; // 카테고리 수

// 지도에 사용될 상수 정의
const int WALL = -5;
const int ROAD = 0;
const int ENTRANCE = -1;
const int CHECKOUT = -2;

// 상품 클래스 정의
class Item {
public:
    string serial; // 상품의 고유번호
    string name; // 상품명 (한글 가정)
    int price;

    Item(string s = "", string n = "", int pri = 0) : serial(s), name(n), price(pri) {} // 기본 생성자의 기본 매개변수 추가
};

// 살 물건 항목에 대한 클래스 정의
class willBuy
{
public:
    Item item; // 무슨 상품인지
    int price, itemNum; // price : 1개당 가격, itemNum : 몇개 살건지
    int itemX, itemY; // 지도에서 상품위치 좌표 저장

    willBuy(string s = "", string n = "", int pri = 0, int Num = 0) :item(s, n), price(pri), itemNum(Num) {}
};

class ShoppingCart {
public:
    willBuy** cartItems;  // 동적 할당된 willBuy 포인터 배열
    int itemCount;        // 장바구니에 저장된 상품의 수

    ShoppingCart() : cartItems(nullptr), itemCount(0) {}

    ~ShoppingCart() {
        for (int i = 0; i < itemCount; i++) {
            delete cartItems[i];  // 동적 할당된 willBuy 객체 해제
        }
        delete[] cartItems;  // 장바구니 배열 해제
    }

    // 장바구니에 상품 추가 함수
    void addItem(const Item& item, int quantity, int x, int y) {
        willBuy** newCartItems = new willBuy * [itemCount + 1];
        for (int i = 0; i < itemCount; i++) {
            newCartItems[i] = cartItems[i];
        }
        newCartItems[itemCount] = new willBuy(item.serial, item.name, item.price, quantity);
        newCartItems[itemCount]->itemX = x;
        newCartItems[itemCount]->itemY = y;
        delete[] cartItems;
        cartItems = newCartItems;
        itemCount++;
    }
};
ShoppingCart myCart;  // 사용자의 장바구니 생성


// 칸 클래스 정의
class Slot {
public:
    int productNumber; // 물품번호
    Item** items; // 동적 할당된 상품 포인터 배열
    int itemCount; // 현재 칸에 저장된 상품의 수

    Slot() : productNumber(-5), items(nullptr), itemCount(0) {} // 기본 생성자 추가, -5 : 벽
    Slot(int num) : productNumber(num), items(nullptr), itemCount(0) {}

    ~Slot() {
        for (int i = 0; i < itemCount; i++) {
            delete items[i]; // 동적 할당된 상품 객체 해제
        }
        delete[] items; // 상품 배열 해제
    }

    // 상품 추가 함수
    void addItem(Item* item) {
        Item** newItems = new Item * [itemCount + 1]; // 현재 상품 수보다 하나 더 큰 새 배열 생성
        for (int i = 0; i < itemCount; i++) {
            newItems[i] = items[i]; // 기존 항목들을 새 배열로 복사
        }
        newItems[itemCount] = new Item(*item); // 새 상품 추가
        delete[] items; // 기존 배열 해제
        items = newItems; // 새 배열을 items로 설정
        itemCount++; // 상품 수 증가
    }
};
// 한 층에 대한 지도 생성 (정적 배열 사용)
Slot floorMap[WIDTH][HEIGHT];


// 물품 클래스 정의
class Product {
public:
    string serial; // 시리얼 넘버
    string name; // 상품명
    int price;
    int x, y; // 지도상의 좌표

    Product(string s, string n, int pri, int x, int y) : serial(s), name(n), price(pri), x(x), y(y) {}
};
// 카테고리별 물품 목록 관리 클래스
class ProductList {
public:
    Product** products; // 동적 할당된 물품 포인터 배열
    int count; // 현재 저장된 물품의 수

    ProductList() : products(nullptr), count(0) {}

    ~ProductList() {
        for (int i = 0; i < count; i++) {
            delete products[i]; // 동적 할당된 물품 객체 해제
        }
        delete[] products; // 물품 배열 해제
    }

    // 물품 추가 함수
    void addProduct(Product* product)
    {
        Product** newProducts = new Product * [count + 1];
        for (int i = 0; i < count; i++)
        {
            newProducts[i] = products[i];
        }
        newProducts[count] = product;
        delete[] products;
        products = newProducts;
        count++;

        // Slot에 Item 추가
        floorMap[product->y][product->x].addItem(new Item(product->serial, product->name, product->price));
    }
};
ProductList storeProducts[CATEGORY_COUNT]; // 카테고리별 물품 목록

void assign_productNumber()
{
    /*
    1 : 친환경 자연주의, 2 : 채소, 과일, 3 : 조리식품, 4 : 생선, 5 : 정육,
    6 : 냉동, 냉장, 7 : 유제품, 8 : 주류, 음료, 9 : 와인, 10 : 대용식
    11 : 양속, 12 : 건해산, 13 : 가공식품, 14 : 건강식품, 15 : 베이커리
    16 : 수입식품, 17 : 행사매장
    */
    int i, j;
    for (i = 10; i <= 13; i++) // 1
    {
        for (j = 2; j <= 3; j++)
            floorMap[i][j].productNumber = 1;
    }

    for (i = 4; i <= 9; i++) // 2
        floorMap[i][1].productNumber = 2;
    for (i = 4; i <= 8; i++)
        floorMap[i][3].productNumber = 2;
    for (i = 4; i <= 5; i++)
        floorMap[i][5].productNumber = 2;
    for (j = 6; j <= 11; j++)
        floorMap[5][j].productNumber = 2;
    for (j = 4; j <= 11; j++)
        floorMap[8][j].productNumber = 2;

    for (i = 2; i <= 3; i++) // 3
        floorMap[i][1].productNumber = 3;
    for (j = 3; j <= 5; j++)
        floorMap[3][j].productNumber = 3;
    for (j = 12; j <= 14; j++)
        floorMap[3][j].productNumber = floorMap[5][j].productNumber = floorMap[6][j].productNumber = 3;
    for (i = 1; i <= 2; i++)
        floorMap[i][14].productNumber = 3;

    for (j = 1; j <= 7; j++) // 4
        floorMap[1][j].productNumber = 4;
    for (i = 2; i <= 3; i++)
        floorMap[i][7].productNumber = 4;

    for (j = 8; j <= 11; j++) // 5
        floorMap[3][j].productNumber = 5;

    for (j = 15; j <= 24; j++) // 6
        floorMap[1][j].productNumber = 6;
    for (i = 2; i <= 3; i++)
        floorMap[i][24].productNumber = 6;

    for (i = 4; i <= 7; i++) // 7
        floorMap[i][24].productNumber = 7;

    for (j = 19; j <= 22; j++) // 8
        floorMap[3][j].productNumber = floorMap[4][j].productNumber = floorMap[5][j].productNumber = floorMap[7][j].productNumber = 8;
    floorMap[7][23] = floorMap[8][19] = 8;

    for (i = 9; i <= 10; i++) // 9
        floorMap[i][19].productNumber = 9;

    for (i = 3; i <= 8; i++) // 10
    {
        for (j = 16; j <= 17; j++)
            floorMap[i][j].productNumber = 10;
    }

    for (i = 7; i <= 8; i++) // 11
    {
        for (j = 12; j <= 14; j++)
            floorMap[i][j].productNumber = 11;
    }

    for (j = 9; j <= 11; j++) // 12
        floorMap[12][j].productNumber = 12;

    for (j = 10; j <= 13; j++) // 13
        floorMap[14][j].productNumber = floorMap[16][j].productNumber = floorMap[18][j].productNumber = floorMap[20][j].productNumber = 13;
    floorMap[20][9].productNumber = 13;

    for (i = 18; i <= 20; i++) // 14
    {
        for (j = 15; j <= 16; j++)
            floorMap[i][j].productNumber = 14;
    }

    for (i = 11; i <= 13; i++) // 15
        floorMap[i][19] = 15;

    for (j = 12; j <= 14; j++) // 16
        floorMap[12][j] = 16;

    for (i = 12; i <= 20; i++) // 17
    {
        for (j = 7; j <= 8; j++)
            floorMap[i][j].productNumber = 17;
    }
}
void assign_road()
{
    int i, j;
    for (j = 2; j <= 6; j++) floorMap[2][j].productNumber = 0;
    for (i = 3; i <= 4; i++) floorMap[i][6].productNumber = 0;
    for (j = 7; j <= 14; j++) floorMap[4][j].productNumber = 0;
    for (j = 15; j <= 23; j++) floorMap[2][j].productNumber = 0;
    for (j = 18; j <= 23; j++) floorMap[6][j].productNumber = 0;
    for (j = 2; j <= 18; j++) floorMap[9][j].productNumber = 0;
    for (j = 7; j <= 14; j++) floorMap[11][j].productNumber = 0;
    for (j = 10; j <= 13; j++) floorMap[13][j].productNumber = floorMap[15][j].productNumber = floorMap[17][j].productNumber = floorMap[19][j].productNumber = 0;
    for (i = 3; i <= 8; i++) floorMap[i][2].productNumber = floorMap[i][15].productNumber = floorMap[i][18].productNumber = 0;
    for (i = 3; i <= 5; i++) floorMap[i][23].productNumber = 0;
    for (i = 13; i <= 19; i++) floorMap[i][9].productNumber = floorMap[i][14].productNumber = 0;
    for (i = 10; i <= 20; i++)
    {
        for (j = 4; j <= 6; j++)
            floorMap[i][j].productNumber = 0;
    }
    for (i = 10; i <= 17; i++)
    {
        for (j = 15; j <= 18; j++)
            floorMap[i][j].productNumber = 0;
    }
    for (i = 18; i <= 20; i++)
    {
        for (j = 17; j <= 18; j++)
            floorMap[i][j].productNumber = 0;
    }
}
void assign_entrance()
{
    /*
    -1 : 입구, -2 : 카운터, 출구
    */
    floorMap[14][3].productNumber = floorMap[14][19].productNumber = -1;
    for (int i = 15; i <= 20; i++)
        floorMap[i][3].productNumber = floorMap[i][19].productNumber = -2;
}
void make_floorMap() //마트 지도 만들기
{
    assign_productNumber();
    assign_road();
    assign_entrance();
}

void makeStore()
{
    // ex) storeProducts[0].addProduct(new Product("0001", "사과", 1000, 10, 10)); // 카테고리 1, 좌표 (10, 10)에 사과 추가
    // 1Sector 상품 추가 (storeProducts[1])
    storeProducts[1].addProduct(new Product("A001", "유기농 샐러드", 8000, 2, 10));
    storeProducts[1].addProduct(new Product("A002", "천연 잼", 5000, 3, 10));
    storeProducts[1].addProduct(new Product("A003", "오가닉 주스", 6000, 3, 11));

    // 2Sector 상품 추가 (storeProducts[2])
    storeProducts[2].addProduct(new Product("B001", "신선한 토마토", 3000, 1, 4));
    storeProducts[2].addProduct(new Product("B002", "브로콜리", 2500, 1, 5));
    storeProducts[2].addProduct(new Product("B003", "당근", 2000, 1, 6));

    // 3Sector 상품 추가 (storeProducts[3])
    storeProducts[3].addProduct(new Product("C001", "김밥", 2500, 12, 3));
    storeProducts[3].addProduct(new Product("C002", "컵라면", 1500, 1, 2));
    storeProducts[3].addProduct(new Product("C003", "도시락", 8000, 14, 2));

    // 4Sector 상품 추가 (storeProducts[4])
    storeProducts[4].addProduct(new Product("D001", "연어 스테이크", 12000, 2, 1));
    storeProducts[4].addProduct(new Product("D002", "참치 스테이크", 10000, 7, 3));

    // 5Sector 상품 추가 (storeProducts[5])
    storeProducts[5].addProduct(new Product("E001", "소고기 스테이크", 15000, 9, 3));
    storeProducts[5].addProduct(new Product("E002", "돼지고기 바베큐", 12000, 10, 3));

    // 6Sector 상품 추가 (storeProducts[6])
    storeProducts[6].addProduct(new Product("F001", "냉동 새우", 10000, 16, 1));
    storeProducts[6].addProduct(new Product("F002", "냉장 치킨", 9000, 22, 1));

    // 7Sector 상품 추가 (storeProducts[7])
    storeProducts[7].addProduct(new Product("G001", "우유", 2500, 24, 5));
    storeProducts[7].addProduct(new Product("G002", "치즈", 4000, 24, 6));
    storeProducts[7].addProduct(new Product("G003", "버터", 3500, 24, 7));

    // 8Sector 상품 추가 (storeProducts[8])
    storeProducts[8].addProduct(new Product("H001", "맥주", 2000, 19, 3));
    storeProducts[8].addProduct(new Product("H002", "청량음료", 1500, 22, 4));
    storeProducts[8].addProduct(new Product("H003", "생수", 1000, 21, 7));

    // 9Sector 상품 추가 (storeProducts[9])
    storeProducts[9].addProduct(new Product("I001", "샤르도네", 30000, 19, 9));
    storeProducts[9].addProduct(new Product("I002", "카베르네 소비뇽", 35000, 19, 10));

    // 10Sector 상품 추가 (storeProducts[10])
    storeProducts[10].addProduct(new Product("J001", "프로틴 바", 3000, 16, 3));
    storeProducts[10].addProduct(new Product("J002", "곡물 시리얼", 5000, 16, 7));
    storeProducts[10].addProduct(new Product("J003", "에너지 드링크", 2500, 17, 5));

    // 11Sector 상품 추가 (storeProducts[11])
    storeProducts[11].addProduct(new Product("K001", "현미", 8000, 12, 8));
    storeProducts[11].addProduct(new Product("K002", "보리", 6000, 14, 8));

    // 12Sector 상품 추가 (storeProducts[12])
    storeProducts[12].addProduct(new Product("L001", "말린 오징어", 12000, 9, 12));
    storeProducts[12].addProduct(new Product("L002", "말린 새우", 10000, 10, 12));

    // 13Sector 상품 추가 (storeProducts[13])
    storeProducts[13].addProduct(new Product("M001", "라면", 1500, 10, 14));
    storeProducts[13].addProduct(new Product("M002", "스파게티 소스", 5500, 13, 16));
    storeProducts[13].addProduct(new Product("M003", "캔 햄", 3000, 11, 20));

    // 14Sector 상품 추가 (storeProducts[14])
    storeProducts[14].addProduct(new Product("N001", "오메가3", 20000, 15, 19));
    storeProducts[14].addProduct(new Product("N002", "비타민 c", 15000, 16, 18));

    // 15Sector 상품 추가 (storeProducts[15])
    storeProducts[15].addProduct(new Product("O001", "식빵", 2000, 19, 11));
    storeProducts[15].addProduct(new Product("O002", "크루아상", 3000, 19, 12));
    storeProducts[15].addProduct(new Product("O003", "치즈케이크", 7000, 19, 13));

    // 16Sector 상품 추가 (storeProducts[16])
    storeProducts[16].addProduct(new Product("P001", "수입 치즈", 10000, 12, 12));
    storeProducts[16].addProduct(new Product("P002", "올리브 오일", 9000, 14, 12));

    // 17Sector 상품 추가 (storeProducts[17])
    storeProducts[17].addProduct(new Product("Q001", "프로모션 와인", 30000, 7, 15));
    storeProducts[17].addProduct(new Product("Q002", "할인 과일", 5000, 8, 18));
}

// 상품 정보를 찾고 장바구니에 추가하는 함수
void addProductToCart(const string& serial, int quantity) {
    for (int i = 0; i < CATEGORY_COUNT; i++) {
        for (int j = 0; j < storeProducts[i].count; j++) {
            Product* product = storeProducts[i].products[j];
            if (product->serial == serial) {
                // 상품을 찾았으니 장바구니에 추가합니다.
                myCart.addItem(Item(product->serial, product->name, product->price), quantity, product->x, product->y);
                return; // 상품을 추가했으니 함수를 종료합니다.
            }
        }
    }
    // 상품을 찾지 못했을 때의 처리를 추가할 수 있습니다.
}

void set_myCart() // 장바구니에 상품을 추가
{
    // myCart에 상품을 추가합니다.
    // 예시로 "유기농 샐러드", "신선한 토마토", "김밥"을 장바구니에 추가하겠습니다.
    // 상품의 좌표는 assign_productNumber 함수에서 지정된 productNumber를 기반으로 가정합니다.
    addProductToCart("A001", 1); // 유기농 샐러드 1개
    addProductToCart("B001", 2); // 신선한 토마토 2개
    addProductToCart("C001", 3); // 김밥 3개
    addProductToCart("M001", 2); // 라면 2개
    addProductToCart("N002", 1); // 비타민C 1개
    addProductToCart("Q002", 7); // 할인과일 7개
}
void setup()
{
    make_floorMap();
    makeStore();
    set_myCart();
}

// BFS를 사용하여 최단 경로 찾기
vector<pair<int, int>> bfs(pair<int, int> start, pair<int, int> goal) {
    set<pair<int, int>> visited;
    queue<pair<pair<int, int>, vector<pair<int, int>>>> q;
    q.push({ start, {start} });

    while (!q.empty()) {
        auto front = q.front();
        pair<int, int> current = front.first;
        vector<pair<int, int>> path = front.second;
        q.pop();

        if (current == goal) {
            return path;
        }

        const int dx[] = { -1, -1, 0, 1, 1, 1, 0, -1 };
        const int dy[] = { 0, 1, 1, 1, 0, -1, -1, -1 };

        cout << "현재 위치 : " << current.first << "," << current.second << endl;

        for (int i = 0; i < 8; ++i) {
            int ny = current.first + dy[i];
            int nx = current.second + dx[i];


            if (ny >= 0 && ny < HEIGHT && nx >= 0 && nx < WIDTH && floorMap[ny][nx].productNumber == ROAD && !visited.count({ ny, nx })) 
            {
                // 현재 탐색 위치 로그 출력
                cout << "탐색 위치: (" << ny << ", " << nx << ")" << endl;

                visited.insert({ ny, nx });
                vector<pair<int, int>> new_path = path;
                new_path.push_back({ ny, nx });
                q.push({ {ny, nx}, new_path });
            }
        }

        cout << endl;
    }

    return {};
}


int main()
{
    setup(); // 마트 지도와 상품 리스트 설정

    pair<int, int> start = { 14, 3 }; // 시작점
    vector<pair<int, int>> path;

    // 장바구니에 있는 모든 상품에 대해 경로 찾기
    for (int i = 0; i < myCart.itemCount; ++i) {
        int y = myCart.cartItems[i]->itemX;
        int x = myCart.cartItems[i]->itemY;
        vector<pair<int, int>> itemPath = bfs(start, { y, x });
        path.insert(path.end(), itemPath.begin(), itemPath.end());
        start = { y, x }; // 다음 상품을 위한 시작점 업데이트
    }

    // 카운터까지의 경로 찾기
    vector<pair<int, int>> checkoutPath = bfs(start, { 15, 19 }); // 카운터의 위치를 가정
    path.insert(path.end(), checkoutPath.begin(), checkoutPath.end());

    // 경로 출력
    for (auto p : path) {
        cout << "(" << p.first << ", " << p.second << ")\n";
    }

    return 0;
}`      `