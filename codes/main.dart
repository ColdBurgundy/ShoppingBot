import 'dart:convert';
import 'dart:math';
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';
import 'package:collection/collection.dart';
import 'package:permission_handler/permission_handler.dart';
import 'MapGenarator.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'dart:collection';
import 'dart:async';

List<List<int>> realPath = []; // 최단 경로 저장

void runBFS(ShoppingCart myCart, List<List<Slot>> floorMap) {
  List<int> start = [3, 14]; // 시작점

  realPath =[];
  for (var cartItem in myCart.cartItems) {
    int y = cartItem.itemY;
    int x = cartItem.itemX;

    // 상품 주변 8방향 중 최단 경로를 찾기
    List<List<int>> shortestPath = [];
    int shortestLength = double.maxFinite.toInt();
    for (int dy = -1; dy <= 1; dy++) {
      for (int dx = -1; dx <= 1; dx++) {
        var itemPath = bfs(floorMap, start, [y + dy, x + dx]);
        if (itemPath.isNotEmpty && (itemPath.length < shortestLength || shortestPath.isEmpty)) {
          shortestPath = itemPath;
          
          shortestLength = itemPath.length;
          print("realPath에 저장된 전체 경로--초기화유무 : $shortestPath");
        }
      }
    }
    realPath.addAll(shortestPath);

    // 마지막 방문 위치를 다음 상품의 시작점으로 설정
    start = shortestPath.last;
    print("start : $start, shortestPath.last : $shortestPath\n");
  }

  // 카운터까지의 경로 찾기
  var checkoutPath = bfs(floorMap, start, [19, 15]); // 카운터 위치를 가정
  realPath.addAll(checkoutPath);
  // realPath에 저장된 모든 값들을 한 번에 출력
  print("realPath에 저장된 전체 경로: $realPath");

}

List<List<int>> bfs(List<List<Slot>> floorMap, List<int> start, List<int> goal) {
  var visited = <List<int>>{};
  var queue = Queue<List<dynamic>>();
  queue.add([start, [start]]);
  print("BFS 시작: 시작점 - $start, 목표점 - $goal"); // 시작 로그
  while (queue.isNotEmpty) {
    var front = queue.removeFirst();
    var current = front[0] as List<int>;
    var path = front[1] as List<List<int>>;
    // 중간 과정 로그
    print("BFS 탐색 중: 현재 위치 - $current");

    if (ListEquality().equals(current, goal)) {
      print("BFS 완료: 목표점 도달 - $goal"); // 종료 로그
      return path;
    }

    var directions = [-1, 0, 1];
    for (var dy in directions) {
      for (var dx in directions) {
        var ny = current[0] + dy;
        var nx = current[1] + dx;

        // 유효한 위치이며 방문하지 않은 경우에만 큐에 추가
        if (ny >= 0 && ny < floorMap.length && nx >= 0 && nx < floorMap[ny].length && 
            floorMap[ny][nx].productNumber == 0 && !visited.any((v) => ListEquality().equals(v, [ny, nx]))) {
          visited.add([ny, nx]);
          var newPath = List<List<int>>.from(path);
          newPath.add([ny, nx]);
          queue.add([ [ny, nx], newPath ]);
        }
      }
    }
  }
  print("BFS 종료: 목표점에 도달하지 못함"); // 실패 로그
  return [];
}







// 페어링된 블루투스 장치들의 목록을 가져오는 함수
Future<List<BluetoothDevice>> getPairedDevices() async {
  List<BluetoothDevice> devices = [];

  try {
    devices = await FlutterBluetoothSerial.instance.getBondedDevices();
  } catch (e) {
    print("Error: $e");
  }
  return devices;
}
//블루투스를 선택하도록 하는 대화상자 표시하는 함수
Future<BluetoothDevice?> selectBluetoothDevice(BuildContext context) async {
  List<BluetoothDevice> devices = await getPairedDevices();
  // 현재 context가 유효한가 
  if (!context.findRenderObject()!.attached) {
    return null; // Context가 더 이상 유효하지 않음
  }

  return showDialog<BluetoothDevice>(
    context: context,
    builder: (BuildContext context) {
      return AlertDialog(
        title: Text('블루투스 장치 선택'),
        content: SingleChildScrollView(
          child: Column(
            children: devices
                .map((device) => ListTile(
                      onTap: () => Navigator.of(context).pop(device),
                      title: Text(device.name ?? ""),
                      subtitle: Text(device.address),
                    ))
                .toList(),
        ),
      ),
      );
    },
  );
}
//권한요청용 함수
Future<void> requestBluetoothPermission() async {
  var status = await Permission.bluetoothScan.status;
  if (!status.isGranted) {
    await Permission.bluetoothScan.request();
  }
}


// 선택된 블루투스 장치에 알고리즘을 통해 얻은 좌표값 리스트를 전송하는 함수
// sendViaBluetooth 함수 수정
Future<void> sendViaBluetooth(BluetoothConnection connection, String dataToSend) async {
  try {
    // 문자열 데이터 전송
    connection.output.add(Uint8List.fromList(utf8.encode(dataToSend)));

    await connection.output.allSent;
    // 데이터 전송 완료 메시지 출력
    print("데이터 전송 완료: $dataToSend");
    connection.finish();
  } catch (e) {
    print("블루투스 데이터 전송 에러: $e");
  } finally {
    connection.close(); // 전송 완료 후 연결 종료
  }
}




void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await requestBluetoothPermission();

  // MapGenerator 인스턴스 생성 및 맵 초기화
  MapGenerator mapGenerator = MapGenerator();

  // 물품 위치 설정
  mapGenerator.setProductLocation("유기농 샐러드", [3,13 ]);
  mapGenerator.setProductLocation("천연 잼", [3,12 ]);
  mapGenerator.setProductLocation("오가닉 주스", [3,10 ]);

  mapGenerator.setProductLocation("신선한 토마토", [1,7 ]);
  mapGenerator.setProductLocation("브로콜리", [3,5 ]);
  mapGenerator.setProductLocation("당근", [7,5 ]);

  mapGenerator.setProductLocation("김밥", [1,3 ]);
  mapGenerator.setProductLocation("컵라면", [13,5 ]);
  mapGenerator.setProductLocation("도시락", [14,1 ]);

  mapGenerator.setProductLocation("연어 스테이크", [2,1 ]);
  mapGenerator.setProductLocation("참치 스테이크", [7,2 ]);

  mapGenerator.setProductLocation("소고기 스테이크", [8,3 ]);
  mapGenerator.setProductLocation("돼지고기 바베큐", [11,3 ]);

  mapGenerator.setProductLocation("냉동 새우", [24,2 ]);
  mapGenerator.setProductLocation("냉장 치킨", [20,1 ]);

  mapGenerator.setProductLocation("우유", [24,4 ]);
  mapGenerator.setProductLocation("치즈", [24,5 ]);
  mapGenerator.setProductLocation("버터", [24,6 ]);

  mapGenerator.setProductLocation("맥주", [19,3 ]);
  mapGenerator.setProductLocation("청량음료", [22,5 ]);
  mapGenerator.setProductLocation("생수", [20,7 ]);

  mapGenerator.setProductLocation("샤르도네", [19,9 ]);
  mapGenerator.setProductLocation("카베르네 소비뇽", [19,10 ]);

  mapGenerator.setProductLocation("프로틴 바", [16,4 ]);
  mapGenerator.setProductLocation("곡물 시리얼", [17,4 ]);
  mapGenerator.setProductLocation("에너지 드링크", [17,8 ]);

  mapGenerator.setProductLocation("현미", [12,8 ]);
  mapGenerator.setProductLocation("보리", [14, 7]);

  mapGenerator.setProductLocation("말린 오징어", [9,12 ]);
  mapGenerator.setProductLocation("말린 새우", [11,12 ]);

  mapGenerator.setProductLocation("라면", [11,14 ]);
  mapGenerator.setProductLocation("스파게티 소스", [11,16 ]);
  mapGenerator.setProductLocation("캔 햄", [14,20 ]);

  mapGenerator.setProductLocation("오메가3", [15,18 ]);
  mapGenerator.setProductLocation("비타민 C", [16,20 ]);

  mapGenerator.setProductLocation("식빵", [19,11 ]);
  mapGenerator.setProductLocation("크루아상", [19,12 ]);
  mapGenerator.setProductLocation("치즈케이크", [19,13 ]);

  mapGenerator.setProductLocation("수입 치즈", [13,12 ]);
  mapGenerator.setProductLocation("올리브 오일", [14,12 ]);

  mapGenerator.setProductLocation("프로모션 와인", [7,14 ]);
  mapGenerator.setProductLocation("할인 과일", [7,18 ]);

  // 맵 초기화
  mapGenerator.initializeMap();
  runApp(MyApp(mapGenerator));
}
//메인 widget - app

class MyApp extends StatelessWidget {
  final MapGenerator mapGenerator;

  MyApp(this.mapGenerator); // 생성자에 mapGenerator 추가
  @override
  Widget build(BuildContext context) {

    return MaterialApp(
      title: '장바구니',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: ProductSelectionPage(mapGenerator), // mapGenerator를 ProductSelectionPage에 전달
    );
  }
}

// 상품 선택 페이지
class ProductSelectionPage extends StatefulWidget {
  final MapGenerator mapGenerator;
  ProductSelectionPage(this.mapGenerator); // 생성자에 mapGenerator 추가
  @override
  _ProductSelectionPageState createState() => _ProductSelectionPageState();
}
// 상품 선택 페이지 관리
class _ProductSelectionPageState extends State<ProductSelectionPage> {
final Map<String, List<Map<String, dynamic>>> products = {
  '1Sector': [
    {'name': '유기농 샐러드', 'price': 8000, 'serial': 'A001'},
    {'name': '천연 잼', 'price': 5000, 'serial': 'A002'},
    {'name': '오가닉 주스', 'price': 6000, 'serial': 'A003'}
  ],
  '2Sector': [
    {'name': '신선한 토마토', 'price': 3000, 'serial': 'B001'},
    {'name': '브로콜리', 'price': 2500, 'serial': 'B002'},
    {'name': '당근', 'price': 2000, 'serial': 'B003'}
  ],
  '3Sector': [
    {'name': '김밥', 'price': 2500, 'serial': 'C001'},
    {'name': '컵라면', 'price': 1500, 'serial': 'C002'},
    {'name': '도시락', 'price': 8000, 'serial': 'C003'}
  ],
  '4Sector': [
    {'name': '연어 스테이크', 'price': 12000, 'serial': 'D001'},
    {'name': '참치 스테이크', 'price': 10000, 'serial': 'D002'}
  ],
  '5Sector': [
    {'name': '소고기 스테이크', 'price': 15000, 'serial': 'E001'},
    {'name': '돼지고기 바베큐', 'price': 12000, 'serial': 'E002'}
  ],
  '6Sector': [
    {'name': '냉동 새우', 'price': 10000, 'serial': 'F001'},
    {'name': '냉장 치킨', 'price': 9000, 'serial': 'F002'}
  ],
  '7Sector': [
    {'name': '우유', 'price': 2500, 'serial': 'G001'},
    {'name': '치즈', 'price': 4000, 'serial': 'G002'},
    {'name': '버터', 'price': 3500, 'serial': 'G003'}
  ],
  '8Sector': [
    {'name': '맥주', 'price': 2000, 'serial': '5600168028837'},
    {'name': '청량음료', 'price': 1500, 'serial': 'H002'},
    {'name': '생수', 'price': 1000, 'serial': '8808244201014'}
  ],
  '9Sector': [
    {'name': '샤르도네', 'price': 30000, 'serial': 'I001'},
    {'name': '카베르네 소비뇽', 'price': 35000, 'serial': 'I002'}
  ],
  '10Sector': [
    {'name': '프로틴 바', 'price': 3000, 'serial': 'J001'},
    {'name': '곡물 시리얼', 'price': 5000, 'serial': 'J002'},
    {'name': '에너지 드링크', 'price': 2500, 'serial': 'J003'}
  ],
  '11Sector': [
    {'name': '현미', 'price': 8000, 'serial': 'K001'},
    {'name': '보리', 'price': 6000, 'serial': 'K002'}
  ],
  '12Sector': [
    {'name': '말린 오징어', 'price': 12000, 'serial': 'L001'},
    {'name': '말린 새우', 'price': 10000, 'serial': 'L002'}
  ],
  '13Sector': [
    {'name': '라면', 'price': 1500, 'serial': '8801043015394'},
    {'name': '스파게티 소스', 'price': 5500, 'serial': 'M002'},
    {'name': '캔 햄', 'price': 3000, 'serial': 'M003'}
  ],
  '14Sector': [
    {'name': '오메가3', 'price': 20000, 'serial': 'N001'},
    {'name': '비타민 C', 'price': 15000, 'serial': '8809220622007'}
  ],
  '15Sector': [
    {'name': '식빵', 'price': 2000, 'serial': 'O001'},
    {'name': '크루아상', 'price': 3000, 'serial': 'O002'},
    {'name': '치즈케이크', 'price': 7000, 'serial': 'O003'}
  ],
  '16Sector': [
    {'name': '수입 치즈', 'price': 10000, 'serial': 'P001'},
    {'name': '올리브 오일', 'price': 9000, 'serial': 'P002'}
  ],
  '17Sector': [
    {'name': '프로모션 와인', 'price': 30000, 'serial': 'Q001'},
    {'name': '할인 과일', 'price': 5000, 'serial': 'Q002'}
  ]
};

  final Map<String, int> cart = {}; //장바구니
  final Map<String, int> quantities = {};  //수량
  String searchQuery = '';  //검색 기능 

//상품 추가 -> 장바구니 함수
  void addToCart(String product, int quantity) {
    if (quantity > 0) {
      setState(() {
        cart[product] = (cart[product] ?? 0) + quantity;
        quantities[product] = 0; // 상품 추가 후 수량 초기화
      });
    }
  }

//ui/ux 관리
@override
Widget build(BuildContext context) {
  // 모든 섹터의 상품을 하나의 리스트로 결합
  List<Map<String, dynamic>> allProducts = products.values.expand((sector) => sector).toList();

  // 검색 쿼리에 따라 상품 필터링
  List<Map<String, dynamic>> filteredProducts = searchQuery.isEmpty
    ? allProducts
    : allProducts.where((product) => product['name'].toLowerCase().contains(searchQuery.toLowerCase())).toList();

  return Scaffold(
    appBar: AppBar(
      title: Text('E-MART 가야지'),
      actions: <Widget>[
        IconButton(
          icon: Icon(Icons.shopping_cart),
          onPressed: () {
            Navigator.push(
              context,
              MaterialPageRoute(
                builder: (context) => ShoppingCartPage(cart, products, widget.mapGenerator)), // mapGenerator 추가
            );
          },
        ),
      ],
    ),
    body: Column(
      children: [
        Padding(
          padding: EdgeInsets.all(8.0),
          child: TextField(
            decoration: InputDecoration(hintText: '검색...'),
            onChanged: (value) {
              setState(() {
                searchQuery = value;
              });
            },
          ),
        ),
        Expanded(
          child: ListView.builder(
            itemCount: filteredProducts.length,
            itemBuilder: (context, index) {
              var product = filteredProducts[index];
              int quantity = quantities[product['name']] ?? 0;
              int totalPrice = product['price'] * quantity;

              return ListTile(
                title: Text(product['name']),
                subtitle: Text(quantity > 0 ? '$totalPrice 원' : '${product['price']} 원'),
                trailing: Row(
                  mainAxisSize: MainAxisSize.min,
                  children: <Widget>[
                    IconButton(
                      icon: Icon(Icons.remove),
                      onPressed: () => setState(() {
                        quantities[product['name']] = max(0, quantity - 1);
                      }),
                    ),
                    Text('$quantity'),
                    IconButton(
                      icon: Icon(Icons.add),
                      onPressed: () => setState(() {
                        quantities[product['name']] = quantity + 1;
                      }),
                    ),
                    IconButton(
                      icon: Icon(Icons.add_shopping_cart),
                      onPressed: () => addToCart(product['name'], quantity),
                    ),
                  ],
                ),
              );
            },
          ),
        ),
      ],
    ),
  );
}



}

// 장바구니 페이지
class ShoppingCartPage extends StatefulWidget {
  final Map<String, int> cart;
  final Map<String, List<Map<String, dynamic>>> products;
  final MapGenerator mapGenerator; // mapGenerator 추가

  ShoppingCartPage(this.cart, this.products, this.mapGenerator); // 생성자 수정


  @override
  _ShoppingCartPageState createState() => _ShoppingCartPageState();
}
// 장바구니 페이지 관리
class _ShoppingCartPageState extends State<ShoppingCartPage> {
  void updateQuantity(String product, int change) {
    setState(() {
      if (widget.cart[product]! + change <= 0) {
        widget.cart.remove(product);
      } else {
        widget.cart[product] = widget.cart[product]! + change;
      }
    });
  }
  // 장바구니 데이터를 로컬 저장소에 저장
  Future<void> saveCartData() async {
    final prefs = await SharedPreferences.getInstance();
    
    // 문자열 데이터 생성
    List<Map<String, dynamic>> cartDataList = [];
    widget.cart.forEach((productName, quantity) {
      for (var sector in widget.products.entries) {
        var product = sector.value.firstWhereOrNull((p) => p['name'] == productName);
        if (product != null && product['serial'] != null) {
          String sectorNumber = sector.key.replaceAll(RegExp(r'[^0-9]'), '');
          cartDataList.add({
            'sectorNumber': sectorNumber,
            'serial': product['serial'],
            'quantity': quantity
          });
        }
      }
    });
    // 데이터를 JSON 문자열로 변환하여 저장
    String cartDataJson = jsonEncode(cartDataList);
    await prefs.setString('cartData', cartDataJson);
  }


  void clearCart() {
    setState(() {
      widget.cart.clear();
    });
  }
//상품 가격 불러오기
int getProductPrice(String product) {
  for (var category in widget.products.entries) {
    for (var p in category.value) {
      if (p['name'] == product) {
        return p['price'] as int? ?? 0;
      }
    }
  }
  return 0; // 상품을 찾지 못했을 경우 기본값으로 0을 반환
}





  @override
Widget build(BuildContext context) {
  int totalPrice = widget.cart.keys.fold(0, (previousValue, element) => previousValue + (getProductPrice(element) * widget.cart[element]!));

  return Scaffold(
    appBar: AppBar(
      title: Text('장바구니'),
    ),
    body: Column(
      children: <Widget>[
        Expanded(
          child: ListView.builder(
            itemCount: widget.cart.keys.length,
            itemBuilder: (context, index) {
              String product = widget.cart.keys.elementAt(index);
              int quantity = widget.cart[product]!;
              int pricePerProduct = getProductPrice(product);

              return ListTile(
                title: Text(product),
                subtitle: Text('수량: $quantity, 가격: ${pricePerProduct * quantity} 원'),
                trailing: Row(
                  mainAxisSize: MainAxisSize.min,
                  children: <Widget>[
                    IconButton(
                      icon: Icon(Icons.remove),
                      onPressed: () => updateQuantity(product, -1),
                    ),
                    IconButton(
                      icon: Icon(Icons.add),
                      onPressed: () => updateQuantity(product, 1),
                    ),
                  ],
                ),
              );
            },
          ),
        ),
        Padding(
          padding: EdgeInsets.symmetric(vertical: 16.0),
          child: Text('Total Price: $totalPrice 원'),
        ),
        ElevatedButton(
          onPressed: () async {

            // 장바구니 데이터 저장
            await saveCartData();


            // 블루투스 권한 요청
            await requestBluetoothPermission();

            // 페어링된 블루투스 장치 목록을 가져옵니다.
            List<BluetoothDevice> devices = await FlutterBluetoothSerial.instance.getBondedDevices();

            // 사용자에게 블루투스 장치 선택 대화상자를 표시합니다.
            BluetoothDevice? selectedDevice = await selectBluetoothDevice(context);

            // 선택된 장치가 HC-06 모듈인 "blue"인 경우
            if (selectedDevice != null && selectedDevice.name == "blue") {
              print("선택된 장치: ${selectedDevice.name}, ${selectedDevice.address}");


              // 장바구니 아이템 위치 설정
              var myCart = ShoppingCart(widget.cart.entries.map((e) => ShoppingCartItem(widget.mapGenerator.productLocations[e.key]?[0] ?? 0, widget.mapGenerator.productLocations[e.key]?[1] ?? 0)).toList());

              // BFS 알고리즘 실행
              runBFS(myCart, widget.mapGenerator.floorMap);
              // realPath 인코딩
              String encodedPath = encodePath(realPath);

              // 경로 데이터 준비
              String pathData = jsonEncode(realPath);

              // 블루투스로 경로 데이터 전송
              if (selectedDevice != null && selectedDevice.name == "blue") {
                try {
                  BluetoothConnection connection = await BluetoothConnection.toAddress(selectedDevice.address);
                  await sendViaBluetooth(connection, encodedPath);

                } catch (e) {
                  print("블루투스 연결 실패: $e");
                }
              }
              // 장바구니 초기화
              clearCart();
            } else if (selectedDevice != null) {
              // HC-06 모듈이 아닌 다른 장치가 선택된 경우
              print("잘못된 장치가 선택되었습니다.");
            } else {
              // 장치가 선택되지 않았을 경우의 처리
              print("블루투스 장치가 선택되지 않았습니다.");
            }
          },
          child: Text('장바구니 확정'),
        ),
      ],
    ),
  );
}
  // realPath 인코딩 함수
  String encodePath(List<List<int>> path) {
    return path.map((point) => "${point[0]},${point[1]}").join(';');
  }
}


