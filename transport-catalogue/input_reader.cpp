#include "input_reader.h"

#include <algorithm>
#include <iostream>
#include <cassert>
#include <iterator>
#include <unordered_set>

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
vector<Distance> ParseDistance(const string& from ,string_view str) {
    vector<Distance> result{};

    // Получаем вектор формата {"9900m to Rasskazovka", "100m to Marushkino"}
    auto split_str = Split(str, ',');

    for(const string_view& info : split_str){
        auto first_space_pos = info.find(' ');
        double distance = std::stod(string(info.substr(0, first_space_pos - 1)));

        auto second_space_pos = info.find(' ', first_space_pos + 1);
        string to = string(info.substr(second_space_pos + 1));
        result.push_back({from, to, distance});
    }

    return result;
}

/**
 * Парсит строку вида "55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino"
 * и возвращает пару строк, "55.595884, 37.209755"
 */
string_view ParseCoordinatePart(string_view str) {
    //Находим вторую запятую, берем строку до нее
    auto comma_1 = str.find(',');
    auto comma_2 = str.find(',', comma_1 + 1);

    return Trim(str.substr(0,comma_2));
}

/**
 * Парсит строку вида "55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino"
 * и возвращает пару строк, "9900m to Rasskazovka, 100m to Marushkino"
 */
string_view ParseDistancePart(string_view str) {
    //Находим вторую запятую, берем строку после неё
    auto comma_1 = str.find(',');
    auto comma_2 = str.find(',', comma_1 + 1);

    return Trim(str.substr(comma_2 + 1));
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
     * Сначала считываем остановки, дистанции, затем маршруты.
     * В методе добавления маршрута заложена проверка, существует ли остановка,
     * переданная в параметрах, для этого должны быть заранее считаны все остановки.
    */

    //Здесь хранятся инедексы остановок. Вставка и поиск O(1) в среднем, при повторных обходах запросов выше скорость, чем сравнивать строки
    unordered_set<size_t> index_stop{};
    vector<Distance> distances{};
    distances.reserve(commands_.size());

    //Добавление остановок
    for(size_t i = 0; i < commands_.size(); ++i){
        if(commands_[i].command == "Stop"){
            index_stop.insert(i);
            string_view coordinates_part = ParseCoordinatePart(commands_[i].description);
            string_view distance_part = ParseDistancePart(commands_[i].description);

            //За один проход забираю все дистанции
            for(const Distance& dist : ParseDistance(commands_[i].id, distance_part)){
                distances.push_back(std::move(dist));
            }
            catalogue.AddStop(commands_[i].id, ParseCoordinates(coordinates_part));
        }
    }

    //Добавление расстояний
    for(const auto& dist : distances){
        catalogue.AddDistance(dist.name_stop_from, dist.name_stop_to, dist.distance);
    }

    //Добавление маршрутов
    for(size_t i = 0; i < commands_.size(); ++i){
        if(index_stop.find(i) == index_stop.end()){
            catalogue.AddBus(commands_[i].id, ParseRoute(commands_[i].description));
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

