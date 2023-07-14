#include "svg.h"

namespace svg {

    using namespace std::literals;


    std::ostream& operator<< (std::ostream& out, Color color) {
        std::visit(ColorPrinter{ out }, color);
        return out;
    }


    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    void ObjectContainer::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(std::move(point));
        //points_.push_back(static_cast<std::string>(point.x));
        //points_ += (std::to_string(point.x) + ","s + std::to_string(point.y));
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first_{ false };
        for (const Point p : points_)
        {
            if (first_) {
                out << ' ';               
            }
            first_ = true;
            out << p.x << ',' << p.y;
        }          
        out << "\" "sv;
        RenderAttrs(out); 
        out << "/> "sv;
    }

    Text& Text::SetPosition(Point pos) { 
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) { 
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) { 
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) { 
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) { 
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) { 
        data_ = std::move(data);
        for (size_t i = 0; i < data_.length(); ++i) {
            switch (data_[i])
            {
            case '&':
                data_.replace(i, 1, "&amp;");
                break;
            case '"':
                data_.replace(i, 1, "&quot;");
                break;
            case '\'':
                data_.replace(i, 1, "&apos;");
                break;
            case '<':
                data_.replace(i, 1, "&lt;");
                break;
            case '>':
                data_.replace(i, 1, "&gt;");
            }
        }

        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_ << "\" "sv;
        if (font_family_) {
            out << "font-family=\""sv << *font_family_ << "\" "sv;
        }
        if (font_weight_) {
            out << "font-weight=\""sv << *font_weight_ << "\" "sv;
        }
        RenderAttrs(out);
        out << ">"sv << data_ << "</text>"sv;
    }


    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        for (const auto& object : objects_) {
            object->Render(out);
        }

        out << "</svg>"sv;
    }
}  // namespace svg