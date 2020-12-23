// Laser_transmitting.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <string>
#include <vector>
#include <iostream>

class Core {
public:
    static const  int name = 2345;
};


class Packet : public Core {
public:
    // static int counter ; // Можно вызывать как Packet::counter (статическая переменная принадлежит классу)
    int header;
    int number;
    char includes[16];
};


class Librarian :public Core {
public:
    std::vector <Packet> send_packs;
    Packet take_one(); // Выдает верхний пакет из массива
    void delete_previous(); // Удаляет верхний пакет из массива
    void get_info(const Packet& pack);
};


class Input_Handler : public Core {
public:
    int counter = 0;
    const int data_pack = 1;
    void pack_the_message(std::string& message, std::vector <Packet>& send_message);
};


class Input_Transmitter : public Core {
public:
    void make_the_library(std::string& message, Input_Handler& handler, Librarian& libs);
};





void Input_Transmitter::make_the_library(std::string& message, Input_Handler& handler, Librarian& libs) {
    while (message[message.size() - 1] == ' ') {
        message.substr(0, message.size() - 1);
    }
    handler.pack_the_message(message, libs.send_packs);// сразу передаем в процедуру массив для библиотекаря
}

void Input_Handler::pack_the_message(std::string& message, std::vector <Packet>& send_message) {
    unsigned int Length = (message.size() + 15) / 16; // Округление в большую сторону
    send_message.resize(Length);
    for (unsigned int i = 0; i < Length; i++) {
        for (unsigned int j = i * 16; j < (i + 1) * 16; j++) {
            if (j > message.size() - 1) {
                send_message[i].includes[j % 16] = 0;
                continue;
            }
            send_message[i].includes[j % 16] = message[j]; // j % 16 тк возвратит место в массиве
        }
        send_message[i].header = Core::name + (data_pack << 14);
        send_message[i].number = counter;
        counter++;
    }
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
    std::cout << "----------------------------------------------" << '\n';
    std::cout << "header --->  " << pack.header << '\n';
    std::cout << "number --->  " << pack.number << '\n';
    std::cout << "data --->  ";
    for (int i = 0; i < 16; i++) {
        std::cout << pack.includes[i];
    }
}
int main()
{
    Core core;
    Input_Transmitter trans;
    Input_Handler hand;
    Librarian libr;
    hand.counter = 0;
    std::string message;
    std::getline(std::cin, message);
    std::cout << '\n';

    trans.make_the_library(message, hand, libr);
    int fixed_size = libr.send_packs.size();
    for (unsigned int i = 0; i < fixed_size; i++) {
        libr.get_info(libr.take_one());
        libr.delete_previous();
    }
    std::cout << "Test commiting 12ё2";
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
