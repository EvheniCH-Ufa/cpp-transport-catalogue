#include "svg.h"

namespace svg {

    using namespace std::literals;

    // ---------------------------- Object ----------------------------
    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
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
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text ------------------
    Text& Text::SetPosition(Point pos)
    {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size)
    {
        font_size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data)
    {
        text_data_ = data;

   /*     Текст отображается следующим образом : строка “<text ”, затем через пробел свойства в произвольном порядке, затем символ “>”,
            содержимое надписи и, наконец, закрывающий тег “< / text>”.Символы ", <, >, ' и & имеют особое значение и при выводе должны экранироваться:
            - Двойная кавычка " заменяется на &quot;.
            Точка с запятой в представлении этого и следующих спецсимволов — обязательная часть экранирующей последовательности.
            - Одинарная кавычка или апостроф ' заменяется на &apos;.
            - Символы < и > заменяются на &lt; и &gt; соответственно.
            - Амперсанд & заменяется на &amp; .
            */
        size_t index_find = text_data_.find("&", 0);
        while (index_find  != std::string::npos)
        {
            text_data_.replace(index_find, 1, "&amp;");
            index_find = text_data_.find("&", index_find + 1);
        }

        index_find = text_data_.find("\"", 0);
        while (index_find != std::string::npos)
        {
            text_data_.replace(index_find, 1, "&quot;");
            index_find = text_data_.find("\"", index_find + 1);
        }

        index_find = text_data_.find("'", 0);
        while (index_find != std::string::npos)
        {
            text_data_.replace(index_find, 1, "&apos;");
            index_find = text_data_.find("'", index_find + 1);
        }

        index_find = text_data_.find("<", 0);
        while (index_find != std::string::npos)
        {
            text_data_.replace(index_find, 1, "&lt;");
            index_find = text_data_.find("<", index_find + 1);
        }

        index_find = text_data_.find(">", 0);
        while (index_find != std::string::npos)
        {
            text_data_.replace(index_find, 1, "&gt;");
            index_find = text_data_.find(">", index_find + 1);
        }
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"s;
        RenderAttrs(out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" dx=\""sv
            << offset_.x << "\" dy=\""sv << offset_.y << "\""sv
            << " font-size=\""sv << font_size_ << "\""sv;
        if (font_family_ != "")
        {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (font_weight_ != "")
        {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">"sv;
        if (text_data_ != "")
        {
            out << text_data_;
        }
        out << "</text>"sv;;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point)
    {
        points_.push_back(std::move(point));
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool is_not_first = false;
        for (const auto& point : points_)
        {
            if (!is_not_first)
            {
                out << point.x << "," << point.y;
                is_not_first = true;
                continue;
            }
            out << " "sv << point.x << "," << point.y;
        }
        out << "\""s;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj)
    {
        object_ptrs_.push_back(std::move(obj));
    }

    // Выводит в ostream svg-представление документа
    void Document::Render(std::ostream& out) const
    {
        RenderContext context(out);
        context.indent = 2;

        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        for (auto it_beg = object_ptrs_.begin(); it_beg != object_ptrs_.end(); it_beg++)
        {
            it_beg->get()->Render(context);
        }
        out << "</svg>"sv;
    }
}  // namespace svg