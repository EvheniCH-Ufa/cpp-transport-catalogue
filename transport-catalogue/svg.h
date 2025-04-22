#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {

    // Задаёт цвет в виде трех компонент в таком порядке: red, green, blue
    struct Rgb {
        Rgb(unsigned char r, unsigned char g, unsigned char b) {
            red = r;
            green = g;
            blue = b;
        };

        unsigned char red = 0;
        unsigned char green = 0;
        unsigned char blue = 0;
    };

    // Задаёт цвет в виде четырёх компонент: red, green, blue, opacity
    struct Rgba {
        unsigned char red = 0;
        unsigned char green = 0;
        unsigned char blue = 0;
        double opacity = 1.0;
    };

//    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
    using Color = std::variant<std::string, svg::Rgb, svg::Rgba>;
    /*Для этого объявите в библиотеке тип svg::Color как std::variant,
    который объединяет типы std::monostate, std::string, svg::Rgb и svg::Rgba.
    Значение std::monostate обозначает отсутствующий цвет, который выводится в виде строки "none".
    */

    // Объявив в заголовочном файле константу со спецификатором inline,
    // мы сделаем так, что она будет одной на все единицы трансляции,
    // которые подключают этот заголовок.
    // В противном случае каждая единица трансляции будет использовать свою копию этой константы
    inline const Color NoneColor{ "none" };

    inline void PrintColor(std::ostream& out, std::monostate) {
        using namespace std::literals;
        out << "none"s;
    }
    inline void PrintColor(std::ostream& out, std::string color) {
        out << color;
    }
    inline void PrintColor(std::ostream& out, Rgb color) {
        using namespace std::literals;
        out << "rgb("s << static_cast<int>(color.red) << ","s
            << static_cast<int>(color.green) << ","s
            << static_cast<int>(color.blue) << ")"s;
    }
    inline void PrintColor(std::ostream& out, Rgba color) {
        using namespace std::literals;
        out << "rgba("s << static_cast<int>(color.red) << ","s
            << static_cast<int>(color.green) << ","s
            << static_cast<int>(color.blue) << ","s
            << color.opacity << ")"s;
    }

    inline std::ostream& operator<<(std::ostream& output, const Color& color) {
      //  using namespace std::literals;
        std::visit(
        [&output](auto value) {
            // Это универсальная лямбда-функция (generic lambda).
            // Внутри неё нужная функция PrintColor будет выбрана за счёт перегрузки функций.
            PrintColor(output, value);
        }, color);
        return output;     // Оператор должен вернуть ссылку на переданный поток вывода
    }

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };
    inline std::ostream& operator<<(std::ostream& output, const StrokeLineCap& r) { // inline  - для исключения множественного определения
        using namespace std::literals;
        std::string str = ""s;
        switch (r)
        {
        case StrokeLineCap::BUTT:   str = "butt"s; break;
        case StrokeLineCap::ROUND:  str = "round"s; break;
        case StrokeLineCap::SQUARE: str = "square"s; break;
        default: break;
        }
        output << str;
        return output;     // Оператор должен вернуть ссылку на переданный поток вывода
    }

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };
    inline std::ostream& operator<<(std::ostream& output, const StrokeLineJoin& r) {
        using namespace std::literals;
        std::string str = ""s;
        switch (r)
        {
        case StrokeLineJoin::ARCS:       str = "arcs"s; break;
        case StrokeLineJoin::BEVEL:      str = "bevel"s; break;
        case StrokeLineJoin::MITER:      str = "miter"s; break;
        case StrokeLineJoin::MITER_CLIP: str = "miter-clip"s; break;
        case StrokeLineJoin::ROUND:      str = "round"s; break;
        default: break;
        }
        output << str;
        return output;     // Оператор должен вернуть ссылку на переданный поток вывода
    }

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


    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {     // задаёт значение свойства fill — цвет заливки.По умолчанию свойство не выводится.
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color) {   // задаёт значение свойства stroke — цвет контура.По умолчанию свойство не выводится.
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {   // задаёт значение свойства stroke-width — толщину линии.По умолчанию свойство не выводится.
            stroke_width_ = std::move(width);
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {   // задаёт значение свойства stroke-linecap — тип формы конца линии.По умолчанию свойство не выводится.
            stroke_linecap_ = std::move(line_cap);
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {   // задаёт значение свойства stroke-linejoin — тип формы соединения линий.По умолчанию свойство не выводится.
            stroke_linejoin_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        // Метод RenderAttrs выводит в поток общие для всех путей атрибуты fill и stroke
        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_linecap_) {
                out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
            }
            if (stroke_linejoin_) {
                out << " stroke-linejoin=\""sv << *stroke_linejoin_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_ = std::nullopt;                // — цвет заливки.По умолчанию свойство не выводится.
        std::optional<Color> stroke_color_ = std::nullopt;              // — цвет контура.По умолчанию свойство не выводится.
        std::optional<double> stroke_width_ = std::nullopt;             // — толщину линии.По умолчанию свойство не выводится.
        std::optional<StrokeLineCap> stroke_linecap_ = std::nullopt;    // — тип формы конца линии.По умолчанию свойство не выводится.
        std::optional<StrokeLineJoin> stroke_linejoin_ = std::nullopt;  // — тип формы соединения линий.По умолчанию свойство не выводится.

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
   
/*Прочие методы и данные, необходимые для реализации элемента <polyline>*/
    private:
        std::vector<Point> points_;
        void RenderObject(const RenderContext& context) const override;
    };

    /*Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text */
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
        Point pos_ = { 0.0, 0.0 };
        Point offset_ = { 0.0, 0.0 };
        uint32_t font_size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string text_data_;
        void RenderObject(const RenderContext& context) const override;
    };

    class ObjectContainer {
    public:
        template <typename Obj>
        /* Метод Add добавляет в svg-документ любой объект-наследник svg::Object*/
        void Add(Obj obj) {
            object_ptrs_.emplace_back(std::make_unique<Obj>(std::move(obj)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
        virtual ~ObjectContainer() = default;

    protected:
        std::vector<std::unique_ptr<Object>> object_ptrs_;

    };

    class Document : public ObjectContainer {
    public:

        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

        // Прочие методы и данные, необходимые для реализации класса Document
    };

    class Drawable
    {
    public:
        virtual ~Drawable() = default;

        virtual void Draw(ObjectContainer& container) const = 0;
    };
}  // namespace svg