#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (!input) {
                throw ParsingError("Array parsing error"s);
            }
            return Node(move(result));
        }

        using Number = std::variant<int, double>;

        Node LoadInt(istream& input) {
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
                        return Node(std::stoi(parsed_num));
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(std::stod(parsed_num));
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        Node LoadString(istream& input) {
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

        Node LoadNull(istream& input) {
            std::string result;
            while (std::isalpha(input.peek())) {
                result.push_back(input.get());
            }
            if (result == "null"s) {
                return Node();
            }
            else {
                throw ParsingError("Null parsing error");
            }
        }

        Node LoadBool(istream& input) {
            std::string result;
            while (std::isalpha(input.peek())) {
                result.push_back(input.get());
            }
            if (result == "true"s) {
                return Node(true);
            }
            else if (result == "false"s) {
                return Node(false);
            }
            else {
                throw ParsingError("Bool parsing error");
            }
        }
        

        Node LoadDict(istream& input) {
            Dict result;
            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (!input) {
                throw ParsingError("Failed to load dictionary");
            }
            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            if (!(input >> c)) {
                throw ParsingError("no data"s);
            }
            switch (c) {
            case '[':
                return LoadArray(input);
            case '{':
                return LoadDict(input);
            case '"':
                return LoadString(input);
            case 't':
                [[fallthrough]];
            case 'f':
                input.putback(c);
                return LoadBool(input);
            case 'n':
                input.putback(c);
                return LoadNull(input);
            default:
                input.putback(c);
                return LoadInt(input);
            }
        }

    }  // namespace


    void Node::SetValue(Value val) {
        value_ = val;
    }

    bool Node::IsInt() const { return std::holds_alternative<int>(value_); }
    bool Node::IsDouble() const { return std::holds_alternative<double>(value_) || IsInt(); } //Возвращает true, если в Node хранится int либо double.
    bool Node::IsPureDouble() const { return std::holds_alternative<double>(value_); } //Возвращает true, если в Node хранится double.
    bool Node::IsBool() const { return std::holds_alternative<bool>(value_); }
    bool Node::IsString() const { return std::holds_alternative<std::string>(value_); }
    bool Node::IsNull() const { return std::holds_alternative<std::nullptr_t>(value_); }
    bool Node::IsArray() const { return std::holds_alternative<Array>(value_); }
    bool Node::IsMap() const { return std::holds_alternative<Dict>(value_); }

    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(value_);
        }
        else {
            throw std::logic_error("Err type");
        }
    }
    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(value_);
        }
        else {
            throw std::logic_error("Err type");
        }
    }
    double Node::AsDouble() const {
        if (IsPureDouble()) {
            return std::get<double>(value_);
        }
        else if (IsInt()) {
            return double(std::get<int>(value_));
        }
        else {
            throw std::logic_error("Err type");
        }
    } //Возвращает значение типа double, если внутри хранится double либо int.В последнем случае возвращается приведённое в double значение.
    const std::string& Node::AsString() const {
        if (IsString()) {
            return std::get<std::string>(value_);
        }
        else {
            throw std::logic_error("Err type");
        }
    }
    const Array& Node::AsArray() const {
        if (IsArray()) {
            return std::get<Array>(value_);
        }
        else {
            throw std::logic_error("Err type");
        }
    }
    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return std::get<Dict>(value_);
        }
        else {
            throw std::logic_error("Err type");
        }
    }

    bool operator==(const Node& lhs, const Node& rhs) {
        if (lhs.IsInt() && rhs.IsInt() && lhs.AsInt() == rhs.AsInt()) return true;
        else if (lhs.IsPureDouble() && rhs.IsPureDouble() && lhs.AsDouble() == rhs.AsDouble()) return true;
        else if (lhs.IsBool() && rhs.IsBool() && lhs.AsBool() == rhs.AsBool()) return true;
        else if (lhs.IsString() && rhs.IsString() && lhs.AsString() == rhs.AsString()) return true;
        else if (lhs.IsNull() && rhs.IsNull()) return true;
        else if (lhs.IsArray() && rhs.IsArray() && lhs.AsArray() == rhs.AsArray()) return true;
        else if (lhs.IsMap() && rhs.IsMap() && lhs.AsMap() == rhs.AsMap()) return true;
        else return false;
    }

    bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    bool operator==(const Document& lhs, const Document& rhs) {
        if (lhs.GetRoot() == rhs.GetRoot()) return true;
        else return false;
    }

    bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);;
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    PrintContext::PrintContext(std::ostream& out)
        : out(out) {
    }

    PrintContext::PrintContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }


    void PrintContext::PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext PrintContext::Indented() const {
        return { out, indent_step, indent_step + indent };
    }

    void PrintValue(std::nullptr_t, const PrintContext& ctx) {
        ctx.out << "null"sv;
    }

    void PrintValue(const bool value, const PrintContext& ctx) {
        ctx.out << std::boolalpha << value;
    }

    void PrintValue(const std::string& str, const PrintContext& ctx) {
        ctx.out.put('"');
        for (const char c : str) {
            switch (c) {
            case '\r':
                ctx.out << "\\r"sv;
                break;
            case '\n':
                ctx.out << "\\n"sv;
                break;
            case '"':
                [[fallthrough]];
            case '\\':
                ctx.out.put('\\');
                [[fallthrough]];
            default:
                ctx.out.put(c);
                break;
            }
        }
        ctx.out.put('"');
    }

    void PrintValue(const Array& arr, const PrintContext& ctx) {
        ctx.out << "[\n";
        bool first{ true };
        auto new_ctx = ctx.Indented();
        for (const auto& val : arr) {
            if (!first) {
                ctx.out << ",\n";
            }
            first = false;
            new_ctx.PrintIndent();
            PrintNode(val, new_ctx);
        }
        ctx.out << '\n';
        ctx.PrintIndent();
        ctx.out << ']';
    }


    void PrintValue(const Dict& dict, const PrintContext& ctx) {
        ctx.out << "{\n";
        bool first{ true };
        auto new_ctx = ctx.Indented();
        for (const auto& [key, val] : dict) {
            if (!first) {
                ctx.out << ",\n";
            }
            first = false;
            new_ctx.PrintIndent();
            ctx.out << '"' << key << "\": "sv;
            PrintNode(val, new_ctx);
        }
        ctx.out << '\n';
        ctx.PrintIndent();
        ctx.out << '}';
    }


    void PrintNode(const Node& node, const PrintContext& ctx) {
        std::visit(
            [&ctx](const auto& value) { PrintValue(value, ctx); },
            node.GetValue());
    }

    void Print(const Document& doc, std::ostream& output) {
        (void)&doc;
        (void)&output;
        PrintContext pc(output);
        PrintNode(doc.GetRoot(), pc);
    }


}  // namespace json