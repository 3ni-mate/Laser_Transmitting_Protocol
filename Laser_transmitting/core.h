#pragma once
#define MESSAGE_START 9
#define MESSAGE_END 15
#define MESSAGE_CONFIRMPTION 10
#define MESSAGE_REQUEST 13
#define MESSAGE_REQUEST_CONFIRMPTION 11
#define MESSAGE_DATA 12
#include <iostream>
#include <string>
#include <vector>
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
    void add_pack(const Packet& pack); // Добавляет пакет в библиотеку
};
class Handler : public Core {
public:
    int counter = 0;
    void pack_the_message(std::string& message, std::vector <Packet>& send_message);
    Packet create_start_pack();
    Packet create_end_pack();
    Packet create_confirm_pack(int num);
    Packet create_connection_request_pack();
    Packet create_connection_request_answer_pack();
    std::string unpack_the_message(Librarian& libr); // Функция возвращающая строку
    int return_type(Packet &input);
};
class Transmitter : public Core {
private:
    Handler& _handler;
    Librarian& _librarian;
public:
    Transmitter(Handler& hander, Librarian& librarian_arg) : _handler(hander), _librarian(librarian_arg) { // Инициализация переменных

    }
    Packet pack_create_confirm_pack(); // Пакет подтверждения сообщения
    Packet pack_create_request_pack();
    Packet pack_create_request_answer_pack();
    int pack_return_type(Packet& input);
    void lib_make_the_library(std::string& message);
    void lib_add_pack(const Packet& pack); // Процедура, нужная для взаимодействия с библиотекарем(чтобы библиотекаря полностью  скрыть из main)
    void lib_delete_previous(); // Процедура, удаляющая первый пакет заданного библиотекаря
    unsigned int lib_get_lib_size(); // Функция, выдающая размер библиотеки заданного библиотекаря
    Packet lib_take_one(); // Функция, выдающая пакет из заданного библиотекаря
    std::string unpack_lib();
};
Packet Transmitter::pack_create_confirm_pack() {
    Packet empty;
    empty = _librarian.take_one();
    return _handler.create_confirm_pack(empty.number);
}
void Transmitter::lib_make_the_library(std::string& message) {
    while ((message[message.size() - 1] == ' ') || (message[message.size() - 1] == '\t')) { // Пробел и символ табуляции
        message.erase(message.size() - 2, message.size() - 1);
    }
    _handler.pack_the_message(message, _librarian.send_packs);// сразу передаем в процедуру массив для библиотекаря
}
void Transmitter::lib_add_pack(const Packet& pack) {
    _librarian.add_pack(pack);
}
void Transmitter::lib_delete_previous() {
    _librarian.delete_previous();
}
unsigned int Transmitter::lib_get_lib_size() {
    return _librarian.send_packs.size();
}
Packet Transmitter::lib_take_one() {
    return _librarian.take_one();
}
Packet Transmitter::pack_create_request_pack() {
    return _handler.create_connection_request_pack();
}
Packet Transmitter::pack_create_request_answer_pack() {
    return _handler.create_connection_request_answer_pack();
}
std::string Transmitter::unpack_lib() {
    return _handler.unpack_the_message(_librarian);
}
int Transmitter::pack_return_type(Packet& input) {
    return _handler.return_type(input);
}


int Handler::return_type(Packet& input) {
    return (input.header >> 12);
}
Packet Handler::create_connection_request_pack() {
    Packet new_one;
    new_one.header = Core::name + (MESSAGE_REQUEST << 12);
    new_one.number = counter;
    counter++;
    return new_one;
}
Packet Handler::create_connection_request_answer_pack() {
    Packet new_one;
    new_one.header = Core::name + (MESSAGE_REQUEST_CONFIRMPTION << 12);
    new_one.number = counter;
    counter++;
    return new_one;
}
Packet Handler::create_start_pack() {
    Packet new_one;
    new_one.header = Core::name + (MESSAGE_START << 12);
    new_one.number = counter;
    counter++;
    return new_one;
}
Packet Handler::create_end_pack() {
    Packet new_one;
    new_one.header = Core::name + (MESSAGE_END << 12);
    new_one.number = counter;
    counter++;
    return new_one;
}
Packet Handler::create_confirm_pack(int num) {
    Packet new_one;
    new_one.header = Core::name + (MESSAGE_CONFIRMPTION << 12);
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
        send_message[i].header = Core::name + ((MESSAGE_DATA + 1 - 1) << 12);
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
void Librarian::add_pack(const Packet& pack) {
    send_packs.push_back(pack);
}