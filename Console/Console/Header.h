#pragma once
#define MESSAGE_START 9
#define MESSAGE_END 15
#define MESSAGE_CONFIRMPTION 10
#define MESSAGE_REQUEST 13
#define MESSAGE_REQUEST_CONFIRMPTION 11
#define MESSAGE_DATA 12
#include <vector>
#include <string>
short name = 234;
class Packet {
public:
    short header = 0;
    short number = 0;
    char data[16] = { 0 };
};
class Data_Storage {
public:
    std::vector<Packet> data_storage; // Данные на отправку
    std::vector<Packet> recieved_data_storage; // принятые данные 
    std::vector<Packet> service_storage; // Хранилище сервисных пакетов
    Packet take_current(); // Дать верхний пакет из хранилища
    bool delete_previous(); // Удалить уже отправленный пакет(после подтверждения приема)
};
class Handler { //  Класс создающий и отправляющий на хранение пакеты
private:
    int counter = 0; // Счетчик пакетов
    Packet create_start_packet();
    Packet create_finish_packet();
public:
    Data_Storage data_reciever; // Хранилище

    bool pack_message(std::string& message); // Запаковка сообщения с отправлением в хранилище
    std::string unpack_message(); // Распаковка сообщения из принятых данных
    int return_packet_type(Packet& packet);
    bool check_for_integrity(Packet& packet);
    Packet return_confirmption_packet(int number);
    bool add_to_storage(Packet packet);
};
Packet Handler::create_finish_packet() {
    Packet new_one;
    new_one.header = name + (MESSAGE_END << 12);
    new_one.number = counter;
    ++counter;
    return new_one;
}
Packet Handler::create_start_packet() {
    Packet new_one;
    new_one.header = name + (MESSAGE_START << 12);
    new_one.number = counter;
    ++counter;
    return new_one;
}
bool Handler::pack_message(std::string& message) {
    unsigned int Length = (message.size() + 15) / 16; // Округление в большую сторону
    data_reciever.data_storage.resize(Length + 2);
    data_reciever.data_storage[0] = create_start_packet(); // Пакет начала приема
    for (unsigned int i = 1; i < Length + 1; i++) {
        for (unsigned int j = i * 16; j < (i + 1) * 16; j++) {
            if (j > message.size() - 1) {
                data_reciever.data_storage[i].data[j % 16] = 0;
                continue;
            }
            data_reciever.data_storage[i].data[j % 16] = message[j]; // j % 16 тк возвратит место в массиве
        }
        data_reciever.data_storage[i].header = name + (MESSAGE_DATA << 12);
        data_reciever.data_storage[i].number = counter;
        ++counter;
    }
    data_reciever.data_storage[Length + 1] = create_finish_packet(); // Пакет конца приема
    return true; // Можно добавить проверку на целостность
}
std::string Handler::unpack_message() {
    std::string return_message;
    for (unsigned int i = 0; i < data_reciever.recieved_data_storage.size(); i++) {
        for (int j = 0; j < 16; j++) {
            return_message += data_reciever.recieved_data_storage[i].data[j];
        }
    }
    return return_message;
}
int Handler::return_packet_type(Packet& packet) {
    return (packet.header >> 12);
}
bool Handler::check_for_integrity(Packet& packet) {
    if (sizeof(packet) == 20) {
        switch (packet.header >> 12) {
        case MESSAGE_START: return true;
        case MESSAGE_END: return true;
        case MESSAGE_DATA: return true;
        case MESSAGE_CONFIRMPTION: return true;
        case MESSAGE_REQUEST: return true;
        case MESSAGE_REQUEST_CONFIRMPTION: return true;
        }
    }
    return false;
}
Packet Handler::return_confirmption_packet(int number) {
    Packet packet;
    packet.header += MESSAGE_CONFIRMPTION << 12;
    packet.number = number;
    data_reciever.service_storage.push_back(packet);
}
bool Handler::add_to_storage(Packet packet) {
    data_reciever.recieved_data_storage.push_back(packet);
}


bool Data_Storage::delete_previous() {
    if (data_storage.size() >= 1) {
        auto i = data_storage.cbegin(); // Возвращение итератора
        data_storage.erase(i);
        return true;
    }
    else if (data_storage.size() == 0) { // Если вектор пустой 
        return false;
    }
}
Packet Data_Storage::take_current() {
    if (data_storage.empty()) {
        exit(101); // Если хранилище пустое
    }
    return data_storage[0];
}
