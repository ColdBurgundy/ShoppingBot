class Item {
  String serial;
  String name;
  int price;

  Item({this.serial = "", this.name = "", this.price = 0});
}

class Slot {
  int productNumber;
  List<Item> items;

  Slot({this.productNumber = -5, List<Item>? items})
      : this.items = items ?? [];

  void addItem(Item item) {
    items.add(item);
  }
}

class ShoppingCartItem {
  int itemY;
  int itemX;

  ShoppingCartItem(this.itemY, this.itemX);
}
class ShoppingCart {
  List<ShoppingCartItem> cartItems;

  ShoppingCart(this.cartItems);
}
class MapGenerator {
  static const int WIDTH = 25;
  static const int HEIGHT = 25;
  static const int WALL = -5;
  static const int ROAD = 0;
  static const int ENTRANCE = -1;
  static const int CHECKOUT = -2;

  List<List<Slot>> floorMap;
  Map<String, List<int>> productLocations; // 물품 위치 정보를 저장하는 맵
  MapGenerator() : floorMap = List.generate(WIDTH, (_) => List.generate(HEIGHT, (_) => Slot())), productLocations = {};
  
  void setProductLocation(String productName, List<int> location) {
    productLocations[productName] = location;
  }
  void initializeMap() {
    assignProductNumber();
    assignRoad();
    assignEntrance();

    // 물품 위치 정보를 기반으로 floorMap 업데이트
    productLocations.forEach((productName, location) {
      floorMap[location[0]][location[1]].addItem(Item(name: productName));
    });
  }
  void assignProductNumber() {
    /*
    1 : 친환경 자연주의, 2 : 채소, 과일, 3 : 조리식품, 4 : 생선, 5 : 정육,
    6 : 냉동, 냉장, 7 : 유제품, 8 : 주류, 음료, 9 : 와인, 10 : 대용식
    11 : 양속, 12 : 건해산, 13 : 가공식품, 14 : 건강식품, 15 : 베이커리
    16 : 수입식품, 17 : 행사매장
    */
    int i, j;
    for(i=10; i<=13; i++) // 1
    {
      for(j=2; j<=3; j++)
        floorMap[i][j].productNumber = 1;
    }

    for(i=4; i<=9; i++) // 2
      floorMap[i][1].productNumber = 2;
    for(i=4; i<=8; i++)
      floorMap[i][3].productNumber = 2;
    for(i=4; i<=5; i++)
      floorMap[i][5].productNumber = 2;
    for(j=6; j<=11; j++)
      floorMap[5][j].productNumber = 2;
    for(j=4; j<=11; j++)
      floorMap[8][j].productNumber = 2;

    for(i=2; i<=3; i++) // 3
      floorMap[i][1].productNumber = 3;
    for(j=3; j<=5; j++)
      floorMap[3][j].productNumber = 3;
    for(j=12; j<=14; j++)
      floorMap[3][j].productNumber = floorMap[5][j].productNumber = floorMap[6][j].productNumber = 3;
    for(i=1; i<=2; i++)
      floorMap[i][14].productNumber = 3;

    for(j=1; j<=7; j++) // 4
      floorMap[1][j].productNumber = 4;
    for(i=2; i<=3; i++)
      floorMap[i][7].productNumber = 4;
    
    for(j=8; j<=11; j++) // 5
      floorMap[3][j].productNumber = 5;

    for(j=15; j<=24; j++) // 6
      floorMap[1][j].productNumber = 6;
    for(i=2; i<=3; i++)
      floorMap[i][24].productNumber = 6;

    for(i=4; i<=7; i++) // 7
      floorMap[i][24].productNumber = 7;

    for(j=19; j<=22; j++) // 8
      floorMap[3][j].productNumber = floorMap[4][j].productNumber = floorMap[5][j].productNumber = floorMap[7][j].productNumber = 8;
      floorMap[7][23].productNumber = 8;
      floorMap[8][19].productNumber = 8;

    for(i=9; i<=10; i++) // 9
      floorMap[i][19].productNumber = 9;

    for(i=3; i<=8; i++) // 10
    {
      for(j=16; j<=17; j++)
        floorMap[i][j].productNumber = 10;
    }

    for(i=7; i<=8; i++) // 11
    {
      for(j=12; j<=14; j++)
        floorMap[i][j].productNumber = 11;
    }

    for(j=9; j<=11; j++) // 12
      floorMap[12][j].productNumber = 12;
    
    for(j=10; j<=13; j++) // 13
      floorMap[14][j].productNumber = floorMap[16][j].productNumber = floorMap[18][j].productNumber = floorMap[20][j].productNumber = 13;
    floorMap[20][9].productNumber = 13;

    for(i=18; i<=20; i++) // 14
    {
      for(j=15; j<=16; j++)
        floorMap[i][j].productNumber = 14;
    }

    for(i=11; i<=13; i++) {// 15
      floorMap[i][19].productNumber = 15;
    }
    for(j=12; j<=14; j++) {// 16
      floorMap[12][j].productNumber = 16;
    }
    for(i=12; i<=20; i++) // 17
    {
      for(j=7; j<=8; j++)
        floorMap[i][j].productNumber = 17;
    }
  }
  void assignRoad()
  {
    int i,j;
    for(j=2; j<=6; j++) floorMap[2][j].productNumber = 0;
    for(i=3; i<=4; i++) floorMap[i][6].productNumber = 0;
    for(j=7; j<=14; j++) floorMap[4][j].productNumber = 0;
    for(j=15; j<=23; j++) floorMap[2][j].productNumber = 0;
    for(j=18; j<=23; j++) floorMap[6][j].productNumber = 0;
    for(j=2; j<=18; j++) floorMap[9][j].productNumber = 0;
    for(j=7; j<=14; j++) floorMap[11][j].productNumber = 0;
    for(j=10; j<=13; j++) floorMap[13][j].productNumber = floorMap[15][j].productNumber = floorMap[17][j].productNumber = floorMap[19][j].productNumber = 0;
    for(i=3; i<=8; i++) floorMap[i][2].productNumber = floorMap[i][15].productNumber = floorMap[i][18].productNumber = 0;
    for(i=3; i<=5; i++) floorMap[i][23].productNumber = 0;
    for(i=13; i<=19; i++) floorMap[i][9].productNumber = floorMap[i][14].productNumber = 0;
    for(i=10; i<=20; i++)
    {
      for(j=4; j<=6; j++)
        floorMap[i][j].productNumber = 0;
    }
    for(i=10; i<=17; i++)
    {
      for(j=15; j<=18; j++)
        floorMap[i][j].productNumber = 0;
    }
    for(i=18; i<=20; i++)
    {
      for(j=17; j<=18; j++)
        floorMap[i][j].productNumber = 0;
    }
  }
  void assignEntrance()
  {
    /*
    -1 : 입구, -2 : 카운터, 출구
    */
    floorMap[14][3].productNumber = floorMap[14][19].productNumber = -1;
    for(int i=15; i<=20; i++)
      floorMap[i][3].productNumber = floorMap[i][19].productNumber = -2;
  }
}

