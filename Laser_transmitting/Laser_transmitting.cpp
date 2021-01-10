// Laser_transmitting.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
// Есть небольшая проблема символ табуляции - исправлено 
/* Небольшое ответвление
1 - код пакета начала сообщения - 1001 = 9
2 - код пакета конца сообщения - 1111 = 15
3 - код пакета подтверждения сообщения - 1010 = 10
4 - код 
*/

#include <string>
#include <future>
#include <chrono>
#include <vector>
#include <iostream>
#include <Windows.h>

class Core {
public:
    static const  int name = 2345;
};


class Packet : public Core {
public:
    // static int counter ; // Можно вызывать как Packet::counter (статическая переменная принадлежит классу)
    int header = 0;
    int number = 0;
    char includes[16] = { 0 };
};


class Librarian :public Core {
public:
    std::vector <Packet> send_packs;
    Packet take_one(); // Выдает верхний пакет из массива
    void delete_previous(); // Удаляет верхний пакет из массива
    void get_info(const Packet& pack); // Показывает информацио о пакете
    void add_pack(const Packet& pack); // Добавляет пакет в библиотеку
};


class Handler : public Core {
public:
    int counter = 0;
    const int data_pack = 1;
    const int commit_pack = 10;
    void pack_the_message(std::string& message, std::vector <Packet>& send_message);
    Packet create_start_pack();
    Packet create_end_pack();
    Packet create_confirm_pack( int num );
    std::string unpack_the_message(Librarian& libr); // Функция возвращающая строку
};


class Transmitter : public Core {
private:
    Handler& _handler;
    Librarian& _librarian;
public:
    Transmitter(Handler& hander, Librarian& librarian_arg): _handler(hander) ,_librarian(librarian_arg){ // Инициализация переменных

    }
    Packet packs_create_start_pack(); // Пакет начала сообщения
    Packet packs_create_end_pack(); // Пакет конца сообщения
    Packet packs_create_confirm_pack(); // Пакет подтверждения сообщения

    void libs_make_the_library(std::string& message);
    void libs_add_pack(const Packet& pack); // Процедура, нужная для взаимодействия с библиотекарем(чтобы библиотекаря полностью  скрыть из main)
    void libs_get_info(const Packet& pack); // Процедура, выводящая первый пакет заданного библиотекаря
    void libs_delete_previous(); // Процедура, удаляющая первый пакет заданного библиотекаря
    unsigned int libs_get_lib_size(); // Функция, выдающая размер библиотеки заданного библиотекаря
    Packet libs_take_one(); // Функция, выдающая пакет из заданного библиотекаря
    std::string unpack_lib();
};


class Intermediary {
private:
    HANDLE serial_port;
    _OVERLAPPED overlap; // структура работы с асинхронным портом
    Transmitter &transmitter;
    bool mode = false;
public:
    bool success = true;

    Intermediary(LPCTSTR port, Transmitter& transmitter_arg) : transmitter(transmitter_arg) {
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
        serial_parameters.Parity   = 0;// нет проверки на четность
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
    bool recieve_the_message(); // Начинает принимать сообщения
    int recieve_the_packet(Packet& empty);// Принятие пакета
    bool send_the_message(); // Начинает отправлть сообщения
    bool send_the_packet(Packet& packet); // Отправка пакета
    bool wait_for_confirm(); // 
    void wait_for_event();
};
bool Intermediary::recieve_the_message() {
    Packet empty, confirmption;
    int check = 1; 
    
    while (check) {
       check = recieve_the_packet(empty);
       if (!check) { return false; } // сделать функцию вывода ошибки
       transmitter.libs_add_pack(empty); //  Добавляем пакет 
       confirmption = transmitter.packs_create_confirm_pack(); // Отправка пакета подтверждения после заполнения пакета
       send_the_packet(confirmption);
       if (check == 2) { break; } // Если сообщение закончено
    }
    return true;
}
int Intermediary::recieve_the_packet(Packet& empty) { // провести отладку времени приема / передачи
    DWORD DWbytes_to_read = sizeof(int);
    DWORD DWread_bytes = 0;
    bool check = ReadFile(serial_port, &empty.header, DWbytes_to_read, &DWread_bytes, &overlap); // Асинхронное чтение
    int time = clock();
    while (!empty.header) {// Пока не произошла запись
        if ((time - clock()) > 2000) {
            // вызываю функцию обработки прирыва канала
        }
    }
    if (!check || (DWread_bytes != DWbytes_to_read)) { return false; } // здесь вызвать функцию повторного запроса сообщения
    check = ReadFile(serial_port, &empty.number, DWbytes_to_read, &DWread_bytes, &overlap); // Сравнение по номеру
    while (!empty.number) {// Пока не произошла запись
        if ((time - clock()) > 2000) {
            // вызываю функцию обработки прирыва канала
        }
    }
    if (!check || (DWread_bytes != DWbytes_to_read)) { return false; }
    DWbytes_to_read = sizeof(empty.includes);
    check = ReadFile(serial_port, &empty.includes, DWbytes_to_read, &DWread_bytes, &overlap); 
    while (!empty.includes[0]) {// Пока не произошла запись
        if ((time - clock()) > 2000) {
            // вызываю функцию обработки прирыва канала
        }
    }
    if (!check || (DWread_bytes != DWbytes_to_read)) { return false; };
    if ((empty.header >> 12) == 15) { return 2; } // Если сообщение закончилось*/
    return true;
}

bool Intermediary::send_the_message() { // Вставить циклы while
        Packet empty;
        // ожидпние подтверждения
        unsigned int Lib_size = transmitter.libs_get_lib_size();
        for (unsigned int i = 0; i < Lib_size; i++) {
            empty = transmitter.libs_take_one();
            while (!send_the_packet(empty)) { // Если не получилось отправить (с учетомвывода в консоль) 
                std::cout << "Something went wrong - error # "<<GetLastError() << " Packet # " << i << " was't deliverd " << std::endl <<
                    "Do you want to try again?";
                int k;// То мы пробуем еще раз отправить пакет
                std::cin >> k;
                if (!k) { exit(10); }
            }
            while (!wait_for_confirm()) {
                std::cout << "Something went wrong - error # " << GetLastError() << " waiting for confimtion for packet # " << i << std::endl <<
                    "Do you want to try again?";
                int k;// То мы пробуем еще раз отправить пакет
                std::cin >> k;
                if (!k) { exit(10); }
            }
            transmitter.libs_delete_previous();
        }


    return true;
}
bool Intermediary::wait_for_confirm() { // Без проверки по времени 
    DWORD DWbytes_to_read = sizeof(int);
    DWORD DWread_bytes = 0;
    int number, header;
    char message[16];
    bool check = ReadFile(serial_port, &header,DWbytes_to_read , &DWread_bytes, NULL); // Чтение заголовка
    if (!check || (DWread_bytes != DWbytes_to_read )) { return false; }
    check = ReadFile(serial_port, &number, DWbytes_to_read, &DWread_bytes, NULL); // Сравнение по номеру
    if (!check || (DWread_bytes != DWbytes_to_read)) { return false; }
    DWbytes_to_read = sizeof(message);
    check = ReadFile(serial_port, &message, DWbytes_to_read, &DWread_bytes, NULL); // Очистка буфера
    if ((header >> 12) != 10) { return false; }
    return true; // Если подтвкрдилось принятие
}
bool Intermediary::send_the_packet(Packet &packet) {
    DWORD DWsize = sizeof(packet.header);
    DWORD DWwritten_bytes = 0;
    int number = packet.header;
    bool check = WriteFile(serial_port, &number, DWsize, &DWwritten_bytes, NULL); // Запись типа и имени
    if ((!check) || (DWwritten_bytes != DWsize)) return false;
    number = packet.number;
    check = WriteFile(serial_port, &number, DWsize, &DWwritten_bytes, NULL); // Запись номера
    if ((!check) || (DWwritten_bytes != DWsize)) return false;
    DWsize = sizeof(packet.includes);
    check = WriteFile(serial_port, packet.includes, DWsize, &DWwritten_bytes, NULL);// Запись содержимого
    if ((!check) || (DWwritten_bytes != DWsize)) return false;
    return true; // Все отправилось в целости и сохранности
}
std::string console_in() { // Функция вводит строку в консоль
    std::string ret_value;
    std::cin >> ret_value;
    return ret_value;
}
void Intermediary::wait_for_event() {
    DWORD Read_bytes = sizeof(int);
    DWORD Readed_bytes = 0 , mask = 0;
    std::string message;
    bool check = 0; 
    SetCommMask(serial_port, EV_RXCHAR); // Реакция на появления значения в буффере
    WaitCommEvent(serial_port, &mask, &overlap);
    std::future<std::string> thread = std::async(std::launch::async, console_in);
    while (true) { // просто бесконечный перебор
        if (mask != EV_RXCHAR) {
            check = recieve_the_message();
            std::cout << transmitter.unpack_lib(); // Вывод строки(вкинуь в отдельную функцию)
        } // Поток для ввода с буфера(реакция на событие
        if (thread.wait_for(std::chrono::milliseconds(100)) == std::future_status::ready) {//Поток для ввода с клавиатуры
            send_the_message();
        }
    }
}


/*       Ненужные фунцкции       */ // Они могут пригодиться
Packet Transmitter::packs_create_start_pack() {
    return _handler.create_start_pack();
}
Packet Transmitter::packs_create_end_pack() {
    return _handler.create_end_pack();
} //
/*       Ненужные функции       */
Packet Transmitter::packs_create_confirm_pack() {
    Packet empty;
    empty = _librarian.take_one();
    return _handler.create_confirm_pack(empty.number);
}
void Transmitter::libs_make_the_library(std::string& message) {
    while ((message[message.size() - 1] == ' ') || (message[message.size() - 1] == '\t')) { // Пробел и символ табуляции
        message.erase(message.size() - 2, message.size() - 1);
    }
    _handler.pack_the_message(message, _librarian.send_packs);// сразу передаем в процедуру массив для библиотекаря
}
void Transmitter::libs_add_pack(const Packet& pack) {
    _librarian.add_pack(pack);
}
void Transmitter::libs_get_info(const Packet& pack) {
    _librarian.get_info(pack);
}
void Transmitter::libs_delete_previous() {
    _librarian.delete_previous();
}
unsigned int Transmitter::libs_get_lib_size() {
    return _librarian.send_packs.size();
}
Packet Transmitter::libs_take_one() {
    return _librarian.take_one();
}
std::string Transmitter::unpack_lib() {
    return _handler.unpack_the_message(_librarian);
}




Packet Handler::create_start_pack() {
    Packet new_one;
    new_one.header = Core::name + (9 << 12);
    new_one.number = counter;
    counter++;
    return new_one;
}
Packet Handler::create_end_pack() {
    Packet new_one;
    new_one.header = Core::name + (15 << 12);
    new_one.number = counter;
    counter++;
    return new_one;
}
Packet Handler::create_confirm_pack( int num) {
    Packet new_one;
    new_one.header = Core::name + (commit_pack << 12);
    new_one.number = num;
    counter++;
    return new_one;
}
void Handler::pack_the_message(std::string& message, std::vector <Packet>& send_message) {
    unsigned int Length = (message.size() + 15) / 16; // Округление в большую сторону
    send_message.resize(Length + 2);
    send_message[0] = create_start_pack(); // Пакет начала приема
    for (unsigned int i = 1; i < Length + 1; i++) {
        for (unsigned int j = i * 16; j < (i + 1) * 16; j++) {
            if (j > message.size() - 1) {
                send_message[i].includes[j % 16] = 0;
                continue;
            }
            send_message[i].includes[j % 16] = message[j]; // j % 16 тк возвратит место в массиве
        }
        send_message[i].header = Core::name + (data_pack << 12);
        send_message[i].number = counter;
        counter++;
    }
    send_message[Length + 1] = create_end_pack(); // Пакет конца приема
}
std::string Handler::unpack_the_message(Librarian& libr) {
    std::string return_message;
    for (unsigned int i = 0; i < libr.send_packs.size(); i++) {
        for (int j = 0; j < 16; j++) {
            return_message += libr.send_packs[i].includes[j];
        }
    }
    return return_message;
}

Packet Librarian::take_one() {
    if (send_packs.empty()) {
        Packet empty;
        empty.number = 0;
        empty.header = 0;
        return empty; // Возвращение пустого пакета
    }
    return send_packs[0]; // Возвращение первого пакета || При получении проверять пустой ли пакет или нет
}
void Librarian::delete_previous() {
    if (send_packs.size() > 1) {
        auto i = send_packs.cbegin(); // Тарабарщина, которая возвращает иератор вектора на 1 элемент ||  auto тк я хз что за тип функция возвращает
        send_packs.erase(i);
    }
    else send_packs.clear();
}
void Librarian::get_info(const Packet& pack) {
    std::cout << '\n';
    std::cout << "<---------------------------------------------->" << '\n';
    std::cout << "header --->  " << pack.header << '\n';
    std::cout << "number --->  " << pack.number << '\n';
    std::cout << "data   --->  ";
    for (int i = 0; i < 16; i++) {
        std::cout << pack.includes[i];
    }
    std::cout << "<" << '\n'; // Показывет на недостающие символы
}
void Librarian::add_pack(const Packet& pack) {
    send_packs.push_back(pack);
} 

int main()
{
    Core core;
    Handler hand;
    Librarian libr;
    Transmitter trans(hand,libr);
    LPCTSTR Port = L"COM5";
    Intermediary inter(Port,trans);
    hand.counter = 0;
    inter.wait_for_event();

    /* Ожидание приема или передачи */

   /* std::getline(std::cin, message);
    std::cout << '\n';
    trans.libs_make_the_library(message);
    inter.send_the_message();

   /*trans.make_the_library(message);
    int fixed_size = trans.libs_get_lib_size();
    for (unsigned int i = 0; i < fixed_size; i++) {
        trans.libs_get_info(trans.libs_take_one()); // Вывод всех пакетов с помощью только Transmtter
        trans.libs_add_pack(trans.libs_take_one()); // Вводим первый пакет в конец(чтобы можно было обратно собрать строку
        trans.libs_delete_previous();
    }
    std::cout << '\n';
    std::cout << trans.unpack_lib();*/
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
