// ConsoleApplication1.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
/*
 Transiver ----> Запускается wait_for_event
  >в нем опрашивается в отдельном потоке io_manager и com_порт
  > в зависимости от произошедшего срабатывает нужный механизм
*/
#define STANDART_WAITING_TIME_MS 2000
#define EXTENDED_WAITING_TIME_MS 3000
#include <iostream>
#include <future>
#include <chrono>
#include <Windows.h>
#include <C:\Users\v.kaluga\source\repos\ConsoleApplication1\ConsoleApplication1\Header.h>
class Coder { // Кодировщик
public:
    std::string code(std::string& str) {
        return str;
    }
};
class INTERFACE_DESCRIPTION { // Класс отвечающий за базоые методы для интерфейса передачи
public:
    bool success;
    virtual bool catch_the_event() = 0;
    virtual bool send_packet(Packet& packet) = 0;
    virtual bool recieve_packet(Packet& packet, int time) = 0;
};
class _COM :INTERFACE_DESCRIPTION {
private:
    HANDLE serial_port;
    _OVERLAPPED overlap; // структура работы с асинхронным портом
    template<typename input>
    input packet_part(input& data, int time);
public:
    _COM(LPCTSTR port);
    bool recieve_packet(Packet& packet, int time);
    bool send_packet(Packet& packet);
    bool catch_the_event();
};
class IO_manager { // Отвечает за ввод от пользователя(любой способ, просто подкручиваемый модуль) 
    virtual std::string input() = 0; // ВВод строки
    virtual bool output(std::string& string) = 0; // Вывод строки
};
class _Console : IO_manager {
public:
    std::string input() {
        std::string str;
        std::cin >> str;
        return str;
    }
    bool output(std::string& string) {
        std::cout << string;
        return true;
    }
};
class Aimer { // Тоже подключенный модуль наводки
public:
    bool check = true;
};
class Transiver { //Класс связи всего со всем
private:
    INTERFACE_DESCRIPTION* _interface; // ИСпользуемый интерфейс
    IO_manager& io_manager; // средство ввода и вывода
    Aimer& aimer; // средство наводки
    Coder code;
    Handler handl;// средство работы с пакетами
public:
    bool wait_for_event(); // Ожидание события либо от интерфейса либо от IO 
    bool send_message(std::string message);
    bool recieve_message();
};


_COM::_COM(LPCTSTR port) {
    serial_port = CreateFile(port,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
    if (serial_port == INVALID_HANDLE_VALUE) {// Если неправильно объявлен порт
        success = false;
        exit(10);
    }
    DCB serial_parameters;
    if (!GetCommState(serial_port, &serial_parameters)) { // Ошибка в тукущей настройке порта + выборка уже существующих
        success = false;
        exit(10);
    }
    serial_parameters.BaudRate = CBR_9600;// БОД
    serial_parameters.ByteSize = 8;// 8 бит в 1 байте (???)
    serial_parameters.StopBits = 0;// 1 последний бит
    serial_parameters.Parity = 0;// нет проверки на четность
    if (!SetCommState(serial_port, &serial_parameters)) {
        success = false;
        exit(10);
    }
    overlap.Internal = 0;
    overlap.InternalHigh = 0;
    overlap.Offset = 0;
    overlap.OffsetHigh = 0;
    overlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}
bool Transiver::wait_for_event() {
    while (true) {
        std::future<bool> bytes = std::async(std::launch::async, (*_interface).catch_the_event);
        while (true) {

        }
    }
}
bool _COM::catch_the_event() {
    return true;
}
bool  Transiver::send_message(std::string message) {
    /* Добавить проверку по номеру*/
    code.code(message); // Кодировка
    handl.pack_message(message); // Упаковка
    while (handl.data_reciever.data_storage.size() != 0) { // Пока хранилище заполнено
        if (!aimer.check) { continue; }// Если не наводится
        Packet packet = handl.data_reciever.take_current();
        (*_interface).send_packet(packet);
        if (!(*_interface).success) {
            continue; // Попытка еще раз отправить пакет
        }
        Packet packet; // Пакет подтверждения сообщения
        if (!(*_interface).recieve_packet(packet, STANDART_WAITING_TIME_MS)) { continue; } // Если паекет не пришел
        if (handl.check_for_integrity(packet) && handl.return_packet_type(packet) == MESSAGE_CONFIRMPTION) { // Это пакет приема сообщений и он целый
            handl.data_reciever.delete_previous(); // удаление пакета
            continue; // Переход к следующему пакету
        }
        /* Здесь обработка исключений, при которых мы плохо приняли паке
        можно изменить цик(чтобы не передавать пакет несколько раз*/
    }
} 
bool Transiver::recieve_message() {
    /* Сделать создание именных паетов подтверждения
       Сделать проверку по имени у пакетов подтверждения*/
    Packet buffer;
    while (handl.return_packet_type(buffer) != MESSAGE_END) { // Пока не пакет конца сообщения 
        (*_interface).recieve_packet(buffer, STANDART_WAITING_TIME_MS);
        if (!handl.check_for_integrity(buffer)) { // Пакет не полный
            continue;
        }
        Packet conf = handl.data_reciever.service_storage[0];// Хранилище сервисных пакетов
        (*_interface).send_packet(conf);
    }
}
int main()
{

}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
