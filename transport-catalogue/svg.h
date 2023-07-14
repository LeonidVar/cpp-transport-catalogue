#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {

    // Задаёт цвет в виде трех компонент в таком порядке: red, green, blue
    struct Rgb {
        Rgb() = default;
        Rgb(int r, int g, int b)
            :red(r), green(g), blue(b){}
        uint8_t red{ 0 };
        uint8_t green{ 0 };
        uint8_t blue{ 0 };
    };

    struct Rgba {
        Rgba() = default;
        Rgba(int r, int g, int b, double op)
            :red(r), green(g), blue(b), opacity(op) {}
        uint8_t red{ 0 };
        uint8_t green{ 0 };
        uint8_t blue{ 0 };
        double opacity{ 1.0 };
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    struct ColorPrinter {
        std::ostream& out;

        void operator()(std::monostate) const {
            out << "none";
        }
        void operator()(std::string color) const {
            out << color;
        }
        void operator()(Rgb color) const {
            out << "rgb(" << (int) color.red << ',' << (int) color.green << ',' << (int) color.blue << ')';
        }
        void operator()(Rgba color) const {
            out << "rgba(" << (int) color.red << ',' << (int) color.green << ',' << (int) color.blue << ',' << color.opacity << ')';
        }
    };

    std::ostream& operator<< (std::ostream& out, Color color);

    // Объявив в заголовочном файле константу со спецификатором inline,
    // мы сделаем так, что она будет одной на все единицы трансляции,
    // которые подключают этот заголовок.
    // В противном случае каждая единица трансляции будет использовать свою копию этой константы
    inline const Color NoneColor{ };

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

    inline std::ostream& operator<< (std::ostream& out, StrokeLineCap slc) {
        using namespace std::literals;
        switch (slc) 
        {
            case StrokeLineCap::BUTT: out << "butt"sv; break;
            case StrokeLineCap::ROUND: out << "round"sv; break;
            case StrokeLineCap::SQUARE: out << "square"sv; break;
        }
        return out;
    }

    inline std::ostream& operator<< (std::ostream& out, StrokeLineJoin slj) {
        using namespace std::literals;
        switch (slj) 
        {
            case StrokeLineJoin::ARCS: out << "arcs"sv; break;
            case StrokeLineJoin::BEVEL: out << "bevel"sv; break; 
            case StrokeLineJoin::MITER: out << "miter"sv; break; 
            case StrokeLineJoin::MITER_CLIP: out << "miter-clip"sv; break; 
            case StrokeLineJoin::ROUND: out << "round"sv; break;
        }
        return out;
    }

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
        Owner& SetStrokeWidth(double width) {
            width_ = width;
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = std::move(line_cap);
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv;
                std::visit(ColorPrinter{ out }, *fill_color_);
                out << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv;
                std::visit(ColorPrinter{ out }, *stroke_color_);
                out << "\""sv;
            }
            if (width_) {
                out << " stroke-width=\""sv << *width_ << "\""sv;
            }
            if (line_cap_) {
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }
            if (line_join_) {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
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
        std::optional<double> width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

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
            return { out, indent_step, indent + indent_step };
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

    class ObjectContainer {
    public:
        virtual ~ObjectContainer() = default;

        template <typename Obj>
        void Add(Obj object) {
            //objects_.emplace_back(std::make_unique<Obj>(std::move(object)));
            AddPtr(std::make_unique<Obj>(std::move(object)));
        }

        // Добавляет в svg-документ объект-наследник svg::Object
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    class Drawable {
    public:
        virtual ~Drawable() = default;

        virtual void Draw(ObjectContainer& object_container) const = 0;

    private:

    };
    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text> {
    public:
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

    private:
        Point pos_;
        Point offset_;
        uint32_t size_{ 1 };
        std::optional<std::string> font_weight_;
        std::optional<std::string> font_family_;
        std::string data_;

        void RenderObject(const RenderContext& context) const override;
    };

    class Document : public ObjectContainer {
    public:
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj);

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

        // Прочие методы и данные, необходимые для реализации класса Document

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

}  // namespace svg