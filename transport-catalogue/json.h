#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <algorithm>

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
        using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;
        
        Node() = default;

        template <typename T>
        Node(T val) : value_(val) {}

        Node(std::string val) : value_(move(val)) {}
        Node(Dict val) : value_(move(val)) {}
        Node(Array val) : value_(move(val)) {}

        bool IsInt() const;
        bool IsDouble() const; //Возвращает true, если в Node хранится int либо double.
        bool IsPureDouble() const; //Возвращает true, если в Node хранится double.
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const; //Возвращает значение типа double, если внутри хранится double либо int.В последнем случае возвращается приведённое в double значение.
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;
        Value GetValue() const {
            return value_;
        }
        void SetValue(Value val);
        

    private:
        Value value_;
    };

    bool operator==(const Node& lhs, const Node& rhs);
    bool operator!=(const Node& lhs, const Node& rhs);
    template <typename T>
    Node CreateNode(T val) {
        return Node(val);
    }

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    template <typename T>
    Document Create(T val) {
        return Document{ CreateNode(val)};
    }

    bool operator==(const Document& lhs, const Document& rhs);
    bool operator!=(const Document& lhs, const Document& rhs);
    void Print(const Document& doc, std::ostream& output);

    // Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        PrintContext(std::ostream& out);
        PrintContext(std::ostream& out, int indent_step, int indent);

        void PrintIndent() const;

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const;
    };

    // Шаблон, подходящий для вывода double и int
    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx);
    // Перегрузка функции PrintValue для вывода значений null
    void PrintValue(std::nullptr_t, const PrintContext& ctx);
    // Перегрузка функции PrintValue для вывода значений bool
    void PrintValue(bool value, const PrintContext& ctx);
    // Перегрузка функции PrintValue для вывода значений string
    void PrintValue(const std::string& str, const PrintContext& ctx);
    // Перегрузка функции PrintValue для вывода значений Array
    void PrintValue(const Array& arr, const PrintContext& ctx);
    // Перегрузка функции PrintValue для вывода значений Dict
    void PrintValue(const Dict& dict, const PrintContext& ctx);

    void PrintNode(const Node& node, const PrintContext& ctx);

    void Print(const Document& doc, std::ostream& output);

    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }

}  // namespace json