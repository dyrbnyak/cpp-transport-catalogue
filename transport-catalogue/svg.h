#pragma once
// Объявите этот макрос в самом начале файла, чтобы при подключении <cmath> были объявлены макросы M_PI и другие
#define _USE_MATH_DEFINES

#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>



namespace svg {

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

struct Rgb {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};


using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{};

inline std::ostream& operator<<(std::ostream& out, StrokeLineCap value) {
    std::string_view sv;
    switch (value) {
    case StrokeLineCap::BUTT:
        sv = "butt";
        break;
    case StrokeLineCap::ROUND:
        sv = "round";
        break;
    case StrokeLineCap::SQUARE:
        sv = "square";
        break;
    }
    return out << sv;
}



inline std::ostream& operator<<(std::ostream& out, StrokeLineJoin value) {
    std::string_view sv;
    switch (value) {
    case StrokeLineJoin::ARCS:
        sv = "arcs";
        break;
    case StrokeLineJoin::BEVEL:
        sv = "bevel";
        break;
    case StrokeLineJoin::MITER:
        sv = "miter";
        break;
    case StrokeLineJoin::MITER_CLIP:
        sv = "miter-clip";
        break;
    case StrokeLineJoin::ROUND:
        sv = "round";
        break;
    }
    return out << sv;
}


inline std::ostream& operator<<(std::ostream& out, const Color& color) {
    if (auto* rgb = std::get_if<Rgb>(&color)) {
        out << "rgb(" << static_cast<int>(rgb->red) << "," << static_cast<int>(rgb->green) << "," << static_cast<int>(rgb->blue) << ")";

    } else if (auto* rgba = std::get_if<Rgba>(&color)) {
        out << "rgba(" << static_cast<int>(rgba->red) << "," << static_cast<int>(rgba->green) << "," << static_cast<int>(rgba->blue) << "," << rgba->opacity << ")";

    } else if (const auto* str = std::get_if<std::string>(&color); str) {
        out << *str;

    } else {
        // Для std::monostate или других случаев, когда цвет не задан
        out << "none";
    }
    return out;
}


/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }


    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }


    Owner& SetStrokeWidth(double width){
        stroke_width_ = std::move(width);
        return AsOwner();
    }


    Owner& SetStrokeLineCap(StrokeLineCap line_cap){
        stroke_line_cap_ = std::move(line_cap);
        return AsOwner();
    }


    Owner& SetStrokeLineJoin(StrokeLineJoin line_join){
        stroke_line_join_ = std::move(line_join);
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    // Метод RenderAttrs выводит в поток общие для всех путей атрибуты fill и stroke
    void RenderAttrs(std::ostream &out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;  // Добавили пробел
        }
        if (stroke_width_) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;  // Добавили пробел
        }
        if (stroke_line_cap_) {
            out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;  // Добавили пробел
        }
        if (stroke_line_join_) {
            out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;  // Добавили пробел
        }
    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
};



/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle() = default;

    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);
private:
    void RenderObject(const RenderContext& context) const override;

    Point center_{0,0};
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline() = default;
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
     */
private:
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> polyline_{};
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    Text() = default;

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    // Прочие данные и методы, необходимые для реализации элемента <text>
private:
    void HtmlEncodeString(std::ostream& out, std::string_view sv) const;

    void RenderObject(const RenderContext& context) const override;

    Point pos_{0.0, 0.0};
    Point offset_{0.0, 0.0};
    uint32_t size_ = 1;
    std::string font_family_{};
    std::string font_weight_{};
    std::string data_{};
};


class ObjectContainer {
public:
    template <typename ObjectType>
    void Add(ObjectType object) {
        AddPtr(std::make_unique<ObjectType>(std::move(object)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
protected:
    ~ObjectContainer() = default;
};

class Document : public ObjectContainer {
public:
    Document() = default;

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    //Вывод "шапки"
    // <?xml version="1.0" encoding="UTF-8" ?>
    // <svg xmlns="http://www.w3.org/2000/svg" version="1.1">
    void RenderHead(std::ostream& out) const;

    //Вывод Объектов в порядке их добавления
    void RenderObject(std::ostream& out) const;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;
private:
    std::vector<std::unique_ptr<Object>> objects_ {};
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;

    virtual ~Drawable() = default;
private:
};

}  // namespace svg





namespace shapes {

class Triangle : public svg::Drawable {
public:
    Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
        : p1_(p1)
        , p2_(p2)
        , p3_(p3) {
    }

    // Реализует метод Draw интерфейса svg::Drawable
    void Draw(svg::ObjectContainer& container) const override;

private:
    svg::Point p1_, p2_, p3_;
};

class Star : public svg::Drawable{
public:
    Star(svg::Point center, double outer_rad, double inner_rad, int num_rays)
        : center_(center)
        , outer_rad_(outer_rad)
        , inner_rad_(inner_rad)
        , num_rays_(num_rays){
    }

    svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) const;

    void Draw(svg::ObjectContainer& container) const override;
private:
    svg::Point center_;
    double outer_rad_;
    double inner_rad_;
    int num_rays_;
};


class Snowman : public svg::Drawable{
public:
    Snowman(svg::Point head_center, double head_radius)
        : head_center_(head_center)
        , head_radius_(head_radius){
    }

    void Draw(svg::ObjectContainer& container) const override;
private:
    svg::Point head_center_;
    double head_radius_;
};

} // namespace shapes
