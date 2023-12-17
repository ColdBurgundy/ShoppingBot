#include <iostream>
#include <vector>
#include <queue>
#include <set>
using namespace std;

const int MAX_ITEMS = 10; // �� ĭ�� �� �� �ִ� ��ǰ�� �ִ� ����
const int WIDTH = 25;
const int HEIGHT = 25;
const int CATEGORY_COUNT = 18; // ī�װ� ��

// ������ ���� ��� ����
const int WALL = -5;
const int ROAD = 0;
const int ENTRANCE = -1;
const int CHECKOUT = -2;

// ��ǰ Ŭ���� ����
class Item {
public:
    string serial; // ��ǰ�� ������ȣ
    string name; // ��ǰ�� (�ѱ� ����)
    int price;

    Item(string s = "", string n = "", int pri = 0) : serial(s), name(n), price(pri) {} // �⺻ �������� �⺻ �Ű����� �߰�
};

// �� ���� �׸� ���� Ŭ���� ����
class willBuy
{
public:
    Item item; // ���� ��ǰ����
    int price, itemNum; // price : 1���� ����, itemNum : � �����
    int itemX, itemY; // �������� ��ǰ��ġ ��ǥ ����

    willBuy(string s = "", string n = "", int pri = 0, int Num = 0) :item(s, n), price(pri), itemNum(Num) {}
};

class ShoppingCart {
public:
    willBuy** cartItems;  // ���� �Ҵ�� willBuy ������ �迭
    int itemCount;        // ��ٱ��Ͽ� ����� ��ǰ�� ��

    ShoppingCart() : cartItems(nullptr), itemCount(0) {}

    ~ShoppingCart() {
        for (int i = 0; i < itemCount; i++) {
            delete cartItems[i];  // ���� �Ҵ�� willBuy ��ü ����
        }
        delete[] cartItems;  // ��ٱ��� �迭 ����
    }

    // ��ٱ��Ͽ� ��ǰ �߰� �Լ�
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
ShoppingCart myCart;  // ������� ��ٱ��� ����


// ĭ Ŭ���� ����
class Slot {
public:
    int productNumber; // ��ǰ��ȣ
    Item** items; // ���� �Ҵ�� ��ǰ ������ �迭
    int itemCount; // ���� ĭ�� ����� ��ǰ�� ��

    Slot() : productNumber(-5), items(nullptr), itemCount(0) {} // �⺻ ������ �߰�, -5 : ��
    Slot(int num) : productNumber(num), items(nullptr), itemCount(0) {}

    ~Slot() {
        for (int i = 0; i < itemCount; i++) {
            delete items[i]; // ���� �Ҵ�� ��ǰ ��ü ����
        }
        delete[] items; // ��ǰ �迭 ����
    }

    // ��ǰ �߰� �Լ�
    void addItem(Item* item) {
        Item** newItems = new Item * [itemCount + 1]; // ���� ��ǰ ������ �ϳ� �� ū �� �迭 ����
        for (int i = 0; i < itemCount; i++) {
            newItems[i] = items[i]; // ���� �׸���� �� �迭�� ����
        }
        newItems[itemCount] = new Item(*item); // �� ��ǰ �߰�
        delete[] items; // ���� �迭 ����
        items = newItems; // �� �迭�� items�� ����
        itemCount++; // ��ǰ �� ����
    }
};
// �� ���� ���� ���� ���� (���� �迭 ���)
Slot floorMap[WIDTH][HEIGHT];


// ��ǰ Ŭ���� ����
class Product {
public:
    string serial; // �ø��� �ѹ�
    string name; // ��ǰ��
    int price;
    int x, y; // �������� ��ǥ

    Product(string s, string n, int pri, int x, int y) : serial(s), name(n), price(pri), x(x), y(y) {}
};
// ī�װ��� ��ǰ ��� ���� Ŭ����
class ProductList {
public:
    Product** products; // ���� �Ҵ�� ��ǰ ������ �迭
    int count; // ���� ����� ��ǰ�� ��

    ProductList() : products(nullptr), count(0) {}

    ~ProductList() {
        for (int i = 0; i < count; i++) {
            delete products[i]; // ���� �Ҵ�� ��ǰ ��ü ����
        }
        delete[] products; // ��ǰ �迭 ����
    }

    // ��ǰ �߰� �Լ�
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

        // Slot�� Item �߰�
        floorMap[product->y][product->x].addItem(new Item(product->serial, product->name, product->price));
    }
};
ProductList storeProducts[CATEGORY_COUNT]; // ī�װ��� ��ǰ ���

void assign_productNumber()
{
    /*
    1 : ģȯ�� �ڿ�����, 2 : ä��, ����, 3 : ������ǰ, 4 : ����, 5 : ����,
    6 : �õ�, ����, 7 : ����ǰ, 8 : �ַ�, ����, 9 : ����, 10 : ����
    11 : ���, 12 : ���ػ�, 13 : ������ǰ, 14 : �ǰ���ǰ, 15 : ����Ŀ��
    16 : ���Խ�ǰ, 17 : ������
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
    -1 : �Ա�, -2 : ī����, �ⱸ
    */
    floorMap[14][3].productNumber = floorMap[14][19].productNumber = -1;
    for (int i = 15; i <= 20; i++)
        floorMap[i][3].productNumber = floorMap[i][19].productNumber = -2;
}
void make_floorMap() //��Ʈ ���� �����
{
    assign_productNumber();
    assign_road();
    assign_entrance();
}

void makeStore()
{
    // ex) storeProducts[0].addProduct(new Product("0001", "���", 1000, 10, 10)); // ī�װ� 1, ��ǥ (10, 10)�� ��� �߰�
    // 1Sector ��ǰ �߰� (storeProducts[1])
    storeProducts[1].addProduct(new Product("A001", "����� ������", 8000, 2, 10));
    storeProducts[1].addProduct(new Product("A002", "õ�� ��", 5000, 3, 10));
    storeProducts[1].addProduct(new Product("A003", "������ �ֽ�", 6000, 3, 11));

    // 2Sector ��ǰ �߰� (storeProducts[2])
    storeProducts[2].addProduct(new Product("B001", "�ż��� �丶��", 3000, 1, 4));
    storeProducts[2].addProduct(new Product("B002", "����ݸ�", 2500, 1, 5));
    storeProducts[2].addProduct(new Product("B003", "���", 2000, 1, 6));

    // 3Sector ��ǰ �߰� (storeProducts[3])
    storeProducts[3].addProduct(new Product("C001", "���", 2500, 12, 3));
    storeProducts[3].addProduct(new Product("C002", "�Ŷ��", 1500, 1, 2));
    storeProducts[3].addProduct(new Product("C003", "���ö�", 8000, 14, 2));

    // 4Sector ��ǰ �߰� (storeProducts[4])
    storeProducts[4].addProduct(new Product("D001", "���� ������ũ", 12000, 2, 1));
    storeProducts[4].addProduct(new Product("D002", "��ġ ������ũ", 10000, 7, 3));

    // 5Sector ��ǰ �߰� (storeProducts[5])
    storeProducts[5].addProduct(new Product("E001", "�Ұ�� ������ũ", 15000, 9, 3));
    storeProducts[5].addProduct(new Product("E002", "������� �ٺ�ť", 12000, 10, 3));

    // 6Sector ��ǰ �߰� (storeProducts[6])
    storeProducts[6].addProduct(new Product("F001", "�õ� ����", 10000, 16, 1));
    storeProducts[6].addProduct(new Product("F002", "���� ġŲ", 9000, 22, 1));

    // 7Sector ��ǰ �߰� (storeProducts[7])
    storeProducts[7].addProduct(new Product("G001", "����", 2500, 24, 5));
    storeProducts[7].addProduct(new Product("G002", "ġ��", 4000, 24, 6));
    storeProducts[7].addProduct(new Product("G003", "����", 3500, 24, 7));

    // 8Sector ��ǰ �߰� (storeProducts[8])
    storeProducts[8].addProduct(new Product("H001", "����", 2000, 19, 3));
    storeProducts[8].addProduct(new Product("H002", "û������", 1500, 22, 4));
    storeProducts[8].addProduct(new Product("H003", "����", 1000, 21, 7));

    // 9Sector ��ǰ �߰� (storeProducts[9])
    storeProducts[9].addProduct(new Product("I001", "��������", 30000, 19, 9));
    storeProducts[9].addProduct(new Product("I002", "ī������ �Һ�", 35000, 19, 10));

    // 10Sector ��ǰ �߰� (storeProducts[10])
    storeProducts[10].addProduct(new Product("J001", "����ƾ ��", 3000, 16, 3));
    storeProducts[10].addProduct(new Product("J002", "� �ø���", 5000, 16, 7));
    storeProducts[10].addProduct(new Product("J003", "������ �帵ũ", 2500, 17, 5));

    // 11Sector ��ǰ �߰� (storeProducts[11])
    storeProducts[11].addProduct(new Product("K001", "����", 8000, 12, 8));
    storeProducts[11].addProduct(new Product("K002", "����", 6000, 14, 8));

    // 12Sector ��ǰ �߰� (storeProducts[12])
    storeProducts[12].addProduct(new Product("L001", "���� ��¡��", 12000, 9, 12));
    storeProducts[12].addProduct(new Product("L002", "���� ����", 10000, 10, 12));

    // 13Sector ��ǰ �߰� (storeProducts[13])
    storeProducts[13].addProduct(new Product("M001", "���", 1500, 10, 14));
    storeProducts[13].addProduct(new Product("M002", "���İ�Ƽ �ҽ�", 5500, 13, 16));
    storeProducts[13].addProduct(new Product("M003", "ĵ ��", 3000, 11, 20));

    // 14Sector ��ǰ �߰� (storeProducts[14])
    storeProducts[14].addProduct(new Product("N001", "���ް�3", 20000, 15, 19));
    storeProducts[14].addProduct(new Product("N002", "��Ÿ�� c", 15000, 16, 18));

    // 15Sector ��ǰ �߰� (storeProducts[15])
    storeProducts[15].addProduct(new Product("O001", "�Ļ�", 2000, 19, 11));
    storeProducts[15].addProduct(new Product("O002", "ũ��ƻ�", 3000, 19, 12));
    storeProducts[15].addProduct(new Product("O003", "ġ������ũ", 7000, 19, 13));

    // 16Sector ��ǰ �߰� (storeProducts[16])
    storeProducts[16].addProduct(new Product("P001", "���� ġ��", 10000, 12, 12));
    storeProducts[16].addProduct(new Product("P002", "�ø��� ����", 9000, 14, 12));

    // 17Sector ��ǰ �߰� (storeProducts[17])
    storeProducts[17].addProduct(new Product("Q001", "���θ�� ����", 30000, 7, 15));
    storeProducts[17].addProduct(new Product("Q002", "���� ����", 5000, 8, 18));
}

// ��ǰ ������ ã�� ��ٱ��Ͽ� �߰��ϴ� �Լ�
void addProductToCart(const string& serial, int quantity) {
    for (int i = 0; i < CATEGORY_COUNT; i++) {
        for (int j = 0; j < storeProducts[i].count; j++) {
            Product* product = storeProducts[i].products[j];
            if (product->serial == serial) {
                // ��ǰ�� ã������ ��ٱ��Ͽ� �߰��մϴ�.
                myCart.addItem(Item(product->serial, product->name, product->price), quantity, product->x, product->y);
                return; // ��ǰ�� �߰������� �Լ��� �����մϴ�.
            }
        }
    }
    // ��ǰ�� ã�� ������ ���� ó���� �߰��� �� �ֽ��ϴ�.
}

void set_myCart() // ��ٱ��Ͽ� ��ǰ�� �߰�
{
    // myCart�� ��ǰ�� �߰��մϴ�.
    // ���÷� "����� ������", "�ż��� �丶��", "���"�� ��ٱ��Ͽ� �߰��ϰڽ��ϴ�.
    // ��ǰ�� ��ǥ�� assign_productNumber �Լ����� ������ productNumber�� ������� �����մϴ�.
    addProductToCart("A001", 1); // ����� ������ 1��
    addProductToCart("B001", 2); // �ż��� �丶�� 2��
    addProductToCart("C001", 3); // ��� 3��
    addProductToCart("M001", 2); // ��� 2��
    addProductToCart("N002", 1); // ��Ÿ��C 1��
    addProductToCart("Q002", 7); // ���ΰ��� 7��
}
void setup()
{
    make_floorMap();
    makeStore();
    set_myCart();
}

// BFS�� ����Ͽ� �ִ� ��� ã��
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

        cout << "���� ��ġ : " << current.first << "," << current.second << endl;

        for (int i = 0; i < 8; ++i) {
            int ny = current.first + dy[i];
            int nx = current.second + dx[i];


            if (ny >= 0 && ny < HEIGHT && nx >= 0 && nx < WIDTH && floorMap[ny][nx].productNumber == ROAD && !visited.count({ ny, nx })) 
            {
                // ���� Ž�� ��ġ �α� ���
                cout << "Ž�� ��ġ: (" << ny << ", " << nx << ")" << endl;

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
    setup(); // ��Ʈ ������ ��ǰ ����Ʈ ����

    pair<int, int> start = { 14, 3 }; // ������
    vector<pair<int, int>> path;

    // ��ٱ��Ͽ� �ִ� ��� ��ǰ�� ���� ��� ã��
    for (int i = 0; i < myCart.itemCount; ++i) {
        int y = myCart.cartItems[i]->itemX;
        int x = myCart.cartItems[i]->itemY;
        vector<pair<int, int>> itemPath = bfs(start, { y, x });
        path.insert(path.end(), itemPath.begin(), itemPath.end());
        start = { y, x }; // ���� ��ǰ�� ���� ������ ������Ʈ
    }

    // ī���ͱ����� ��� ã��
    vector<pair<int, int>> checkoutPath = bfs(start, { 15, 19 }); // ī������ ��ġ�� ����
    path.insert(path.end(), checkoutPath.begin(), checkoutPath.end());

    // ��� ���
    for (auto p : path) {
        cout << "(" << p.first << ", " << p.second << ")\n";
    }

    return 0;
}`      `