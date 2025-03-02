#include "json.h"
#include <cmath>
#include <algorithm>


using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            char c;
            for (; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c != ']')
            {
                throw ParsingError("Array parsing error");
            }


            return Node(move(result));
        }

        using Number = std::variant<int, double>;

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        Node LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }
            return Node(move(s));
        }

        Node LoadDict(istream& input) {
            Dict result;
            char c;

            for (; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                if (key.empty())
                {
                    throw ParsingError("Dict parsing error");
                }
                input >> c;

                result.insert({ move(key), LoadNode(input) });
            }

            if (c != '}')
            {
                throw ParsingError("Dict parsing error");
            }
            return Node(move(result));
        }

        Node LoadNull(istream& input) {
            //   using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();

            const std::string s1 = "ull";
            const std::string s2 = "ULL";
            size_t pos = 0;

            while (pos < s1.size())
            {
                if (it == end) {
                    // Поток закончился до того, как прочитали остатаки - rue?
                    throw ParsingError("Null parsing error");
                }
                const char ch = *it;
                if ((ch != s1[pos]) && (ch != s2[pos])) {
                    throw ParsingError("Null parsing error");
                };
                ++it;
                ++pos;
            }

            while (it != end && *it != ',' && *it != ']' && *it != '}')
            {
                //    char ch = *it;
                if (*it != '\t' && *it != ' ' && *it != '\r' && *it != '\n') {
                    throw ParsingError("Null parsing error"); // еще есть что-то кроме мусора
                }
                ++it;
            }
            return Node(nullptr);
        }

        Node LoadTrue(istream& input) {
            //   using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();

            const std::string s1 = "rue";
            const std::string s2 = "RUE";
            size_t pos = 0;

            while (pos < s1.size())
            {
                if (it == end) {
                    // Поток закончился до того, как прочитали остатаки - rue?
                    throw ParsingError("True parsing error");
                }
                const char ch = *it;
                if ((ch != s1[pos]) && (ch != s2[pos])) {
                    throw ParsingError("true parsing error");
                };
                ++it;
                ++pos;
            }

            while (it != end && *it != ',' && *it != ']' && *it != '}')
            {
                //    char ch = *it;
                if (*it != '\t' && *it != ' ' && *it != '\r' && *it != '\n') {
                    throw ParsingError("true parsing error"); // еще есть что-то кроме мусора
                }
                ++it;
            }
            return Node(true);
        }

        Node LoadFalse(istream& input) {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();

            const std::string s1 = "alse";
            const std::string s2 = "ALSE";
            size_t pos = 0;

            while (pos < s1.size())
            {
                if (it == end) {
                    // Поток закончился до того, как прочитали остатаки - rue?
                    throw ParsingError("False parsing error");
                }
                const char ch = *it;
                if ((ch != s1[pos]) && (ch != s2[pos])) {
                    throw ParsingError("False parsing error");
                };
                ++it;
                ++pos;
            }

            while (it != end && *it != ',' && *it != ']' && *it != '}')
            {
                //   char ch = *it;
                if (*it != '\t' && *it != ' ' && *it != '\r' && *it != '\n') {
                    throw ParsingError("False parsing error"); // еще есть что-то кроме мусора
                }
                ++it;
            }
            return Node(false);
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;
            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == ']') {
                throw ParsingError("Array parsing error");
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '}') {
                throw ParsingError("Array parsing error");
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if ((c == 'n') || (c == 'N')) {
                return LoadNull(input);
            }
            else if ((c == 't') || (c == 'T')) {
                return LoadTrue(input);
            }
            else if ((c == 'f') || (c == 'F')) {
                return LoadFalse(input);
            }
            else {
                input.putback(c);
                //        return LoadDigit(input);

                auto number = LoadNumber(input);
                if (number.index() == 0)
                {
                    return Node(std::get<int>(number));
                }
                else
                {
                    return Node(std::get<double>(number));
                }
            }
        }

    }  // namespace



    bool Node::IsInt() const
    {
        return std::holds_alternative<int>(value_);
    }
    bool Node::IsDouble() const // Возвращает true, если в Node хранится int либо double.
    {
        return (std::holds_alternative<int>(value_) || std::holds_alternative<double>(value_));
    }
    bool Node::IsPureDouble() const //Возвращает true, если в Node хранится double.
    {
        return std::holds_alternative<double>(value_);
    }
    bool Node::IsBool() const
    {
        return std::holds_alternative<bool>(value_);
    }
    bool Node::IsString() const
    {
        return std::holds_alternative<std::string>(value_);
    }
    bool Node::IsNull() const
    {
        return std::holds_alternative<nullptr_t>(value_);
    }
    bool Node::IsArray() const
    {
        return std::holds_alternative<Array>(value_);
    }
    bool Node::IsMap() const
    {
        return std::holds_alternative<Dict>(value_);
    }


    int Node::AsInt() const
    {
        // std::get_if вернёт указатель на значение нужного типа 
        // либо nullptr, если variant содержит значение другого типа.
        if (const auto* result = std::get_if<int>(&value_)) {
            return *result;
        }
        else {
            throw std::logic_error("Wrong type to Int!!!");
        }
    }
    bool Node::AsBool() const
    {
        if (const auto* result = std::get_if<bool>(&value_)) {
            return *result;
        }
        else {
            throw std::logic_error("Wrong type to bool!!!");
        }
    }
    double Node::AsDouble() const //Возвращает значение типа double, если внутри хранится double либо int. В последнем случае возвращается приведённое в double значение.
    {
        if (const auto* result = std::get_if<double>(&value_)) {
            return static_cast<double>(*result);
        }
        else if (const auto* result = std::get_if<int>(&value_)) {
            return static_cast<double>(*result);
        }
        else {
            throw std::logic_error("Wrong type to Double!!!");
        }
    }
    const std::string& Node::AsString() const
    {
        if (const auto* result = std::get_if<std::string>(&value_)) {
            return *result;
        }
        else {
            throw std::logic_error("Wrong type to String!!!");
        }
    }
    const Array& Node::AsArray() const
    {
        if (const auto* result = std::get_if<Array>(&value_)) {
            return *result;
        }
        else {
            throw std::logic_error("Wrong type to Array!!!");
        }
    }
    const Dict& Node::AsMap() const
    {
        if (const auto* result = std::get_if<Dict>(&value_)) {
            return *result;
        }
        else {
            throw std::logic_error("Wrong type to Map!!!");
        }
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    const Node& Document::GetRequests(const std::string& request) const
    {
        return root_.AsMap().at(request);// .GetValue();
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void PrintNode(const Node& node, std::ostream& out);


    // Перегрузка функции PrintValue для вывода значений string
    void PrintValue(const std::string& value, std::ostream& out) {
        string::const_iterator  it = value.begin();
        std::string result;
        std::string buff;
        bool is_first = false;
        while (it != value.end())
        {
            char ch = *it;
            switch (ch)
            {
            case '\\':
                buff += "\\\\";
                break;
            case '\r':
                if (!is_first)
                {
                    buff += "\\r";
                }
                break;
            case '\n':
                if (!is_first)
                {
                    buff += "\\n";
                }
                break;
            case '\t':
                if (!is_first)
                {
                    buff += "\\t";
                }
                break;
            case '\"':
                buff += "\\\"";
                break;
            default:
                buff.push_back(ch);
                break;
            }

            if (!buff.empty())
            {
                result += buff;
                buff.clear();
                is_first = false;
            }
            ++it;
        }
        out << "\"" << result << "\"";
    }


    // Перегрузка функции PrintValue для вывода значений null
    void PrintValue(std::nullptr_t, std::ostream& out) {
        out << "null"sv;
    }

    // Перегрузка функции PrintValue для вывода значений bool
    void PrintValue(bool value, std::ostream& out) {
        if (value) { out << "true"sv; }
        else { out << "false"sv; };
    }


    void PrintValue(json::Array array, std::ostream& out) {
        if (array.size() == 0)
        {
            out << "[]"sv;
            return;
        }

        out << "["sv << "\n"sv;
        bool is_first = true;
        for (const auto& node : array)
        {
            if (!is_first)
            {
                out << ","sv << "\n"sv;
            }
            is_first = false;
            PrintNode(node, out);
        }
        out << "\n"sv << "]"sv;
    }

    void PrintValue(json::Dict dict, std::ostream& out) {
        out << "{"sv << "\n"sv;;
        bool is_first = true;
        for (const auto& node : dict)
        {
            if (!is_first)
            {
                out << ","sv << "\n"sv;
            }
            is_first = false;
            out << "\"" << node.first << "\":";
            PrintNode(node.second, out);
        }

        //   assert(LoadJSON("{ \"key1\": \"value1\", \"key2\": 42 }"s).GetRoot() == dict_node);

        out << "\n"sv << "}"sv;

    }

    // Шаблон, подходящий для вывода double и int
    template <typename Value>
    void PrintValue(const Value& value, std::ostream& out) {
        out << value;
    }


    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(
            [&out](const auto& value) { PrintValue(value, out); },
            node.GetValue());
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }
}  // namespace json