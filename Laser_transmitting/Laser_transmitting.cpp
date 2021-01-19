// Laser_transmitting.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
// Есть небольшая проблема символ табуляции - исправлено 
/* Небольшое ответвление
1 - код пакета начала сообщения - 1001 = 9
2 - код пакета конца сообщения - 1111 = 15
3 - код пакета подтверждения сообщения - 1010 = 10
4 - код пакета запроса связи - 1101 = 13
5 - kод пакета подтверждения связи - 1011 - 11
6 - код пакета данных - 1100 - 12
*/
#define STANDART_WAITING_TIME_MS 2000
#define EXTENDED_WAITING_TIME_MS 3000
#include <string>
#include <future>
#include <chrono>
#include <vector>
#include <iostream>
#include <Windows.h>
#include "/Users/User/source/repos/Laser_transmitting/Laser_transmitting/core.h"

/* Классы*/
class Interface {
public:
    virtual bool byte_catch(std::string& return_strng) = 0;
    virtual bool recieve_the_packet(Packet& recieved, int time, bool correct_conection_break) = 0;
    virtual bool send_the_packet(const Packet& sent) = 0;
   // virtual bool wait_for_confirmation() = 0;// z  Эта функция попросту не нужна
};
class COM : public Interface {
private : 
    HANDLE serial_port;
    _OVERLAPPED overlap; // структура работы с асинхронным портом
    bool mode = false;
public:
    bool success = true;
    COM(LPCTSTR port);
    bool recieve_the_packet(Packet& recieved, int time, bool correct_conection_break);
    bool send_the_packet(const Packet& sent);
    bool byte_catch(std::string &return_strng); // COM + консольныйй ввод
   // bool wait_for_confirmation();
};
class Intermediary {
private:
    Transmitter &transmitter;
    Interface* _interface;
public:
    bool success = true;

    Intermediary(Transmitter& transmitter_arg, Interface &_interface_arg) : transmitter(transmitter_arg), _interface(&_interface_arg) {}
    bool recieve_the_message(); // Начинает принимать сообщения
    bool send_the_message(); // Начинает отправлть сообщения
    void wait_for_event();
    bool establish_connection_for_input(); // Восстанавливает соединение для приемника (в эту функцию включу наводку по камере, когда придет вемя
};
/*Вспомогательные функции*/
bool error_message(int num) {
    std::cout << "Something went wrong - error # " << GetLastError() << " Packet # " << num << " was't deliverd " << std::endl <<
        "Do you want to try again?";
    int k;// То мы пробуем еще раз отправить пакет
    std::cin >> k;
    if (!k)  return false;
    else return true;
}
std::string console_in() { // Функция вводит строку в консоль
    std::string ret_value;
    std::cin >> ret_value;
    return ret_value;
}
void showmessage(const std::string message) {
    std::cout << message;
}
void confirmation_error(int i) {
    std::cout << "Something went wrong while recieving confirmation to packen #" << i << " last error is " << GetLastError;
}
void io_error() {
    std::cout << "We have recieved data while transmitting message, your string has been deleted? here's recieved message ";
}


/*Конструктор для ком порта*/
COM::COM(LPCTSTR port) {
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
bool COM::recieve_the_packet(Packet& recieved, int time, bool correct_conection_break) {
    DWORD DWbytes_to_read = sizeof(int);
    DWORD DWread_bytes = 0;
    bool check = ReadFile(serial_port, &recieved.header, DWbytes_to_read, &DWread_bytes, &overlap); // Асинхронное чтение
    int time = clock();
    while (!recieved.header) {// Пока не произошла запись
        if ((time - clock()) > time) {
            if (!correct_conection_break) { return false; }// Если на мне нужно восстанавливать канал связи(отправка в 1 сторону)
            // Остановка чтения
            // вызываю функцию обработки прирыва канала
        }
    }
    if (!check || (DWread_bytes != DWbytes_to_read)) { return false; } // здесь вызвать функцию повторного запроса сообщения
    check = ReadFile(serial_port, &recieved.number, DWbytes_to_read, &DWread_bytes, &overlap); // Сравнение по номеру
    while (!recieved.number) {// Пока не произошла запись
        if ((time - clock()) > time) {
            if (!correct_conection_break) { return false; }// Если на мне нужно восстанавливать канал связи(отправка в 1 сторону)
            // Осановка чтения
            // вызываю функцию обработки прирыва канала
        }
    }
    if (!check || (DWread_bytes != DWbytes_to_read)) { return false; }
    DWbytes_to_read = sizeof(recieved.includes);
    time = clock();
    check = ReadFile(serial_port, &recieved.includes, DWbytes_to_read, &DWread_bytes, &overlap);
    while (!recieved.includes[15]) {// Пока не произошла запись
        if ((time - clock()) > time) {
            if (!correct_conection_break) { return false; }// Если на мне нужно восстанавливать канал связи(отправка в 1 сторону)
            // Остановка чтения
            // вызываю функцию обработки прирыва канала
        }
    }
    if (!check || (DWread_bytes != DWbytes_to_read)) { return false; };
    return true;
}
bool COM::send_the_packet(const Packet& sent) { // Синхронно или асинхронно
    DWORD DWsize = sizeof(sent.header);
    DWORD DWwritten_bytes = 0;
    int number = sent.header;
    bool check = WriteFile(serial_port, &number, DWsize, &DWwritten_bytes, NULL); // Запись типа и имени
    if ((!check) || (DWwritten_bytes != DWsize)) return false;
    number = sent.number;
    check = WriteFile(serial_port, &number, DWsize, &DWwritten_bytes, NULL); // Запись номера
    if ((!check) || (DWwritten_bytes != DWsize)) return false;
    DWsize = sizeof(sent.includes);
    check = WriteFile(serial_port, sent.includes, DWsize, &DWwritten_bytes, NULL);// Запись содержимого
    if ((!check) || (DWwritten_bytes != DWsize)) return false;
    return true; // Все отправилось в целости и сохранности
}
bool COM::byte_catch(std::string &return_string) { // Добавить обработки технических неполадок 
    DWORD  mask = 0;
    std::string message;
    bool check = 0;
    SetCommMask(serial_port, EV_RXCHAR); // Реакция на появления значения в буффере
    WaitCommEvent(serial_port, &mask, &overlap);
    std::future<std::string> thread = std::async(std::launch::async, console_in);
    while (true) { // просто бесконечный перебор
        if (mask == EV_RXCHAR) {
            return_string = "";
            return true;
        } // Поток для ввода с буфера(реакция на событие
        if (thread.wait_for(std::chrono::milliseconds(100)) == std::future_status::ready) {//Поток для ввода с клавиатуры
            return_string = thread.get();
            return false; 
        }
    }
}





bool Intermediary::recieve_the_message() { // Нужно добавить таймер для выхода из цикла и восстановления подключения 
    Packet current;
    bool check = true;
    while (transmitter.pack_return_type(current) != 15) { // Пока не пришел пакет конца сообщения
        check = (*_interface).recieve_the_packet(current, STANDART_WAITING_TIME_MS, true); // Прием пакета 
        if (!check) {// Если произошел разрыв  соединения, то мы просто пропускаем текщую итерацию(следовательно пробуем его принять еще раз)
            check = true; 
            continue;
        } 
        transmitter.lib_add_pack(current); // Добавление пакета
        (*_interface).send_the_packet(transmitter.pack_create_confirm_pack()); // Отправка пакета подтверждения
    }
    return true;
}
bool Intermediary::send_the_message() { 
    unsigned int i = 0;
    bool check = true;
    Packet confirmation;
    while (i < transmitter.lib_get_lib_size()) {
        check = (*_interface).send_the_packet(transmitter.lib_take_one()); // Отправляем пакет
        if (!check) { // Проверка на ошибки
            check = true; // Уводим флаг из положения ошибки
            error_message(i); // Выводим ошибку
            continue; // Пытаемся еще раз отправить этот пакет
        }
        check = (*_interface).recieve_the_packet(confirmation, STANDART_WAITING_TIME_MS, true); // Ждем подтверждения принятия
        if (!check) { // Проблемы с подтверждением
            check = true;
            confirmation_error(i); // Лучше для этого свою процедуру написать
            continue;
        }
        if (transmitter.pack_return_type(confirmation) != 10) {
            // вся программа писалась в приоритете к чтению, поэтому если такая проблема возникает, то
            while(transmitter.lib_get_lib_size() != 0 ){
                transmitter.lib_delete_previous(); // Полностью очищаем библиотеку
            }
            transmitter.lib_add_pack(confirmation); // Добавляем считанный пакет
            check = recieve_the_message();// Дальнейшая обработка ошибок уже будет паранойей
            io_error(); // процедура с текстом - во время передачи сообщения были приняты некоторые данные, поэтому передача была приостановлена, введите сообщение еще раз
            return true;
        }
        transmitter.lib_delete_previous();
    }
    return true;
}
void Intermediary::wait_for_event() {
    std::string message = "";
    bool check = true;
    while (true) {
        if ((*_interface).byte_catch(message)) {
            check = recieve_the_message(); // Прием сообщения 
            showmessage(transmitter.unpack_lib()); // Отображение сообщения
        }
        else { check = send_the_message(); }
    }
}
bool Intermediary::establish_connection_for_input() { // Каждые 30 сек отправлять пакет 
    Packet recieved;
    bool check = true;
    int start_time = clock();
    while (true) {
        check = (*_interface).send_the_packet(transmitter.pack_create_request_pack()); 
        if ((*_interface).recieve_the_packet(recieved, EXTENDED_WAITING_TIME_MS, false)) {
            return true; // Соединение восстановлено
            // Можно сюда добавить несколько итераций для проверки
        }
        if (start_time - clock() == EXTENDED_WAITING_TIME_MS * 3) {
            return false;
        }
    }

}





int main()
{
    Handler hand;
    Librarian libr;
    Transmitter trans(hand,libr);
    LPCTSTR Port = L"COM5";
    COM com(Port);
    Intermediary inter(trans, com);
    hand.counter = 0;
    inter.wait_for_event();
    return 0;
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
