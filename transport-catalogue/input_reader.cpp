#include "input_reader.h"

#include <algorithm>
#include <iostream>
#include <cassert>
#include <iterator>

using namespace std;

string_view Trim(string_view string);
vector<string_view> Split(string_view string, char delim);

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(string(str.substr(not_space2)));

    return {lat, lng};
}

/**
 * Парсит строку вида "9900m to Rasskazovka, 100m to Marushkino" и возвращает вектор пар (Остановка куда, растояние до нее)
 * Пример: {<Rasskazovka, 9900>, <Marushkino, 100>}
 *
 * Такой подход обосоновывается тем, что в метод AddDistance будет передаваться id(Остановка относительно которой указано расстояние)
 * И из вектора пар и id будет собрано значение для словаря distance_;
 */
vector<pair<string, size_t>> ParseDistance(string_view str) {
    vector<pair<string, size_t>> result{};

    // Получаем вектор формата {"9900m to Rasskazovka", "100m to Marushkino"}
    auto split_str = Split(str, ',');


    for(const string_view& info : split_str){
        auto first_space_pos = info.find(' ');
        size_t distance = std::stoi(string(info.substr(0, first_space_pos - 1)));
        auto second_space_pos = info.find(' ', first_space_pos + 1);
        string stop = string(info.substr(second_space_pos + 1));
        result.push_back({stop, distance});
    }

    return result;
}

/**
 * Парсит строку вида "55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino"
 * и возвращает пару строк, first -> (широта, долгота), second -> (дистанции до точки)
 */
pair<string_view, string_view> ParseDiscription(string_view str) {
    //Находим вторую запятую, берем строку до нее и после неё
    //Это и будут наши first и second
    auto comma_1 = str.find(',');
    auto comma_2 = str.find(',', comma_1 + 1);

    return {Trim(str.substr(0,comma_2)),
            Trim(str.substr(comma_2 + 1))};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
string_view Trim(string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
vector<string_view> Split(string_view string, char delim) {
    vector<string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
vector<string_view> ParseRoute(string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    vector<string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {string(line.substr(0, space_pos)),
            string(line.substr(not_space, colon_pos - not_space)),
            string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    /*
     * Сначала считываем остановки, затем маршруты.
     * В методе добавления маршрута заложена проверка, существует ли остановка,
     * переданная в параметрах, для этого должны быть заранее считаны все остановки.
     */

    //Идея для увелечения скорости работы, завести числовой вектор, в котором сохранить значение индексов остановок при первом проходе.
    //Затем при добавлении маршрутов не проверять индексы, под которыми находятся остановки

    //Идея: Написать отдельную функцию, которая будет разбивать описание на часть, где координаты и где расстояния,
    //затем передавть в соответствующие парсеры.

    for(const CommandDescription&  command_stop : commands_){
        if(command_stop.command == "Stop"){
            pair<string_view, string_view> pars_discriprion = ParseDiscription(command_stop.description);
            //First -> координаты, Second - расстояния
            catalogue.AddStop(command_stop.id, ParseCoordinates(pars_discriprion.first));
            catalogue.AddDistance(command_stop.id, ParseDistance(pars_discriprion.second));
        }
    }

    for(const CommandDescription&  command_bus : commands_){
        if(command_bus.command == "Bus"){
            catalogue.AddBus(command_bus.id, ParseRoute(command_bus.description));
        }
    }
}


void InputReader::ReadRequests(istream& input, size_t base_request_count) {
    for (size_t i = 0; i < base_request_count; ++i) {
        string line;
        getline(input, line);
        ParseLine(line);
    }
}

