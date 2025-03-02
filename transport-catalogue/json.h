#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using NodeValue = std::variant<std::nullptr_t, std::string, int, double, bool, Array, Dict>;
        Node() = default;
        Node(const NodeValue& value) : value_(value) {};
        Node(const std::string& value) : value_(value) {};
        Node(const int value) : value_(value) {};
        Node(const double value) : value_(value) {};
        Node(const bool value) : value_(value) {};
        Node(std::nullptr_t) : value_(nullptr) {};
        Node(const Array& value) : value_(value) {};
        Node(const Dict& value) : value_(value) {};

        Node& operator=(NodeValue value)
        {
            value_ = std::move(value);
            return *this; // возвращаем текущий объект
        }

        NodeValue GetValue() const
        {
            return value_;
        }

        //  Следующие методы Node сообщают, хранится ли внутри значение некоторого типа :
        bool IsInt() const;
        bool IsDouble() const; // Возвращает true, если в Node хранится int либо double.
        bool IsPureDouble() const; //Возвращает true, если в Node хранится double.
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        //    Ниже перечислены методы, которые возвращают хранящееся внутри Node значение заданного типа.
        //    Если внутри содержится значение другого типа, должно выбрасываться исключение std::logic_error.
        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const; //.Возвращает значение типа double, если внутри хранится double либо int.В последнем случае возвращается приведённое в double значение.
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;
        //    Объекты Node можно сравнивать между собой при помощи == и != .
        //    Значения равны, если внутри них значения имеют одинаковый тип и содержимое.

    private:
        NodeValue value_ = nullptr;
    };


    inline bool operator == (const Node& left, const Node& right)
    {
        return ((left.GetValue().index() == right.GetValue().index()) && (left.GetValue() == right.GetValue()));
    }
    inline bool operator != (const Node& left, const Node& right)
    {
        return ((left.GetValue().index() != right.GetValue().index()) || (left.GetValue() != right.GetValue()));
    }

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        const Node& GetRequests(const std::string& request) const ; // пока по верхнему уровню - достаточно

    private:
        Node root_;
    };

    inline bool operator == (const Document& left, const Document& right)
    {
        return ((left.GetRoot().GetValue().index() == right.GetRoot().GetValue().index()) && (left.GetRoot().GetValue() == right.GetRoot().GetValue()));
    }
    inline bool operator != (const Document& left, const Document& right)
    {
        return ((left.GetRoot().GetValue().index() != right.GetRoot().GetValue().index()) || (left.GetRoot().GetValue() != right.GetRoot().GetValue()));
    }


    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json
