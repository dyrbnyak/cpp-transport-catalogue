#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}




// ---------- Circle ------------------

Circle &Circle::SetCenter(Point center){
    center_ = center;
    return *this;
}

Circle &Circle::SetRadius(double radius){
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << "/>"sv;
}





// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point){
    polyline_.push_back(std::move(point));

    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    out << "<polyline points=\""sv;
    bool first = true;
    for (const auto& point : polyline_) {
        if (!first) {
            out << " "sv;  // Ровно один пробел между координатами
        }
        first = false;
        out << point.x << ","sv << point.y;
    }
    out << "\"";
    RenderAttrs(context.out);
    out << " />"sv;


}





// ---------- Text ------------------

Text &Text::SetPosition(Point pos){
    pos_ = pos;
    return *this;
}

Text &Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}

Text &Text::SetFontSize(uint32_t size){
    size_ = size;
    return *this;
}

Text &Text::SetFontFamily(std::string font_family){
    font_family_ = font_family;
    return *this;
}

Text &Text::SetFontWeight(std::string font_weight){
    font_weight_ = font_weight;
    return *this;
}

Text &Text::SetData(std::string data){
    data_ = data;
    return *this;
}

void Text::HtmlEncodeString(std::ostream &out, std::string_view sv) const{
    for (char c : sv) {
        switch (c) {
        case '"':
            out << "&quot;"sv;
            break;
        case '<':
            out << "&lt;"sv;
            break;
        case '>':
            out << "&gt;"sv;
            break;
        case '&':
            out << "&amp;"sv;
            break;
        case '\'':
            out << "&apos;"sv;
            break;
        default:
            out.put(c);
        }
    }
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    out << "<text "sv;
    RenderAttrs(context.out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << size_ << "\" "sv;

    if (!font_family_.empty()) {
        out << "font-family=\""sv << font_family_ << "\" "sv;
    }

    if (!font_weight_.empty()) {
        out << "font-weight=\""sv << font_weight_ << "\" "sv;
    }

    out << ">"sv;

    // Экранирование специальных символов в data_
    HtmlEncodeString(out, data_);


    out << "</text>"sv;
}





// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::RenderHead(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << "\n";
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << "\n";
}

void Document::RenderObject(std::ostream& out) const {
    RenderContext context(out, 2, 2);

    for (const auto& obj : objects_) {
        if (obj) {
            obj->Render(context);
        }
    }
}

void Document::Render(std::ostream& out) const {
    RenderHead(out);
    RenderObject(out);
    out << "</svg>"sv;
}


}  // namespace svg



namespace shapes {

// ---------- Tringle ------------------
void Triangle::Draw(svg::ObjectContainer &container) const {
    container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
}




// ---------- Star ------------------
svg::Polyline Star::CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) const{
    using namespace svg;
    Polyline polyline;
    for (int i = 0; i <= num_rays; ++i) {
        double angle = 2 * M_PI * (i % num_rays) / num_rays;
        polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
        if (i == num_rays) {
            break;
        }
        angle += M_PI / num_rays;
        polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
    }
    return polyline;
}

void Star::Draw(svg::ObjectContainer& container) const{
    container.Add(CreateStar(center_, outer_rad_, inner_rad_, num_rays_)
                      .SetFillColor("red")
                      .SetStrokeColor("black"));
}



// ---------- Circle ------------------
void Snowman::Draw(svg::ObjectContainer& container) const{
    using namespace svg;
    container.Add(Circle().SetCenter({head_center_.x, head_center_.y + head_radius_ * 5})
                      .SetRadius(head_radius_ * 2)
                      .SetFillColor("rgb(240,240,240)")
                      .SetStrokeColor("black"));

    container.Add(Circle().SetCenter({head_center_.x, head_center_.y + head_radius_ * 2})
                      .SetRadius(head_radius_ * 1.5)
                      .SetFillColor("rgb(240,240,240)")
                      .SetStrokeColor("black"));

    container.Add(Circle().SetCenter({head_center_.x, head_center_.y})
                      .SetRadius(head_radius_)
                      .SetFillColor("rgb(240,240,240)")
                      .SetStrokeColor("black"));
}


} // namespace shapes
