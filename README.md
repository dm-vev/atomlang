# Atom

Atom - это бесплатный, открытый и гибкий язык программирования, который предназначен для создания различных видов программного обеспечения, в том числе и для встраивания в автономные системы. Он предлагает простой и интуитивно понятный синтаксис, который позволяет разработчикам быстро и легко создавать высококачественный код.

Atom компилируется в собственную виртуальную машину написанную на Си. Это обеспечивает кроссплатформенность для языка. 

#### Зачем?
Прежде всего, язык ставит перед собой цель стать языком для написания програм для низкопроизводительных устройств, а так-же для микроконтролерров. 

## Примеры кода:
#### 1. Hello world!
```js
io.write("Hello world!\n");
```
#### 2. Ввод от пользователя
```js
io.write('Hello, what is your name? ')
var name = io.read()
io.write('Nice to meet you, ', name, '!\n')
```
#### 3. Структуры
```js
var Car = {
	speed : 0
};

function car::speedTo(v : integer) { // дополнение структуры
	this.speed = v; // изменение локальной переменной speed
}

io.write(Car.speed); // 0
Car->speedTo(12); //вызов функции из структуры
io.write(Car.speed); // 12
```
#### 4. Работа с файлами
```js
var file = io.open("example.txt", "w");
file->write("Hello, world!");
file->close();

file = io.open("example.txt", "r");
var contents = file->read("*all");
file->close();
```

#### 5. Корутины
```js
function fibonacci(n) {
    if( n < 2 ) {
        return n;
    } else {
        return fibonacci(n-1) + fibonacci(n-2);
    }
}

var start_time = os.time();
for( i = 1, 40 ) {
    print(fibonacci(i));
}
var end_time = os.time();
print("(Without coroutines) Execution time: " .. (end_time - start_time) .. " seconds");

function fibonacci_co(n) {
    var function iter(a, b, count) {
        if( count == 0 ) {
            return a;
        } else {
            coroutine.yield(a);
            return iter(b, a + b, count - 1);
        }
    }

    return coroutine.wrap(function() { iter(0, 1, n); });
}

var start_time2 = os.time();
for( fib in fibonacci_co(40) ) {
    print(fib);
}
var end_time2 = os.time();
print("(With coroutines) Execution time: " .. (end_time - start_time) .. " seconds");

```

6. Работа с сетью и внешними библиотеками
```js
require("libnet") // Обратите внимание, требуется отедльная библиотека для работы с сетью. Ее можно найти в lib/libnet/libnet.so

sockfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0) // Создаем сокет
socket.connect("localhost", 80, sockfd) // Подключаемся
socket.send(sockfd, "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n") // Отправляем данные

var response = socket.recv(sockfd) // Читаем данные
socket.close(sockfd) // Закрываем сокет

io.write(response) // Выводим результат
```

Больше примеров Вы сможете найти в папке **examples**.

## Использование

И так, вы решили воспользоваться языком. Как его собрать?
Для начала, разберемся вообще что происходит с вашим скриптом перед компиляцией.

Ваш скрипт проходит через компилятор atomc(Atom compiler), который генерирует байткод(bytecode.bin) для виртуальной машины vm. Без файла bytecode.bin, ваша программа не запустится(!!!).

Собрать програму в байткод можно так:
```bash
./atomc (имя файла) [-о выходной файл(необязательно)]
```

Далее вы можете запустить вашу уже скомпилированную программу так:
```bash
./vm
```

Виртуальная машина не имеет зависимостей и может спокойно запускаться на любых ПК.
