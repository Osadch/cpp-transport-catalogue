#include "json.h"

using namespace std;

namespace json {

    namespace {

        using Number = std::variant<int, double>;

        Node LoadNode(istream& input);

        Node LoadNull(istream& input) {
            const string nameNull = "null";
            for (size_t i = 0; i < nameNull.size(); i++) {
                if (nameNull.at(i) == input.get()) continue;
                else throw ParsingError("Null parsing error");
            }
            return {};
        }

        std::string LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    ++it;
                    if (it == end) {
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    
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
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

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

            if (input.peek() == '0') {
                read_char();
            }
            else {
                read_digits();
            }

            bool is_int = true;
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

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
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadBool(istream& input) {
            const string nameFalse = "false";
            const string nameTrue = "true";
            char c = input.get();
            bool value = (c == 't');
            std::string const* name = value ? &nameTrue : &nameFalse;
            for (size_t i = 1; i < name->size(); i++) {
                if (name->at(i) == input.get()) continue;
                else throw ParsingError("Bool parsing error");
            }
            return Node(value);
        }

        Node LoadArray(istream& input) {
            Array result;
            if (input.peek() == -1) throw ParsingError("Array parsing error");

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            return Node(std::move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;
            if (input.peek() == -1) throw ParsingError("Array parsing error");

            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input);
                input >> c;
                result.insert({ std::move(key), LoadNode(input) });
            }

            return Node(std::move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    bool Node::IsNull() const noexcept {
        return std::holds_alternative<std::nullptr_t>(*this);
    }
    bool Node::IsBool() const noexcept {
        return std::holds_alternative<bool>(*this);
    }
    bool Node::IsInt() const noexcept {
        return std::holds_alternative<int>(*this);
    }
    bool Node::IsDouble() const noexcept {
        return (std::holds_alternative<int>(*this) ||
            std::holds_alternative<double>(*this));
    }
    bool Node::IsPureDouble() const noexcept {
        return std::holds_alternative<double>(*this);
    }
    bool Node::IsString() const noexcept {
        return std::holds_alternative<std::string>(*this);
    }
    bool Node::IsArray() const noexcept {
        return std::holds_alternative<Array>(*this);
    }
    bool Node::IsMap() const noexcept {
        return std::holds_alternative<Dict>(*this);
    }

    const Array& Node::AsArray() const {
        if (IsArray()) {
            return std::get<Array>(*this);
        }
        else {
            throw std::logic_error("Node data is not array"s);
        }
    }

    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return std::get<Dict>(*this);
        }
        else {
            throw std::logic_error("Node data is not map"s);
        }
    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(*this);
        }
        else {
            throw std::logic_error("Node data is not bool"s);
        }
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(*this);
        }
        else {
            throw std::logic_error("Node data is not int"s);
        }
    }

    double Node::AsDouble() const {
        if (IsInt()) {
            return static_cast<double>(std::get<int>(*this));
        }
        else if (IsPureDouble()) {
            return std::get<double>(*this);
        }
        else {
            throw std::logic_error("Node data is not double"s);
        }
    }

    const string& Node::AsString() const {
        if (IsString()) {
            return std::get<std::string>(*this);
        }
        else {
            throw std::logic_error("Node data is not string"s);
        }
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void PrintValue(std::nullptr_t, const PrintContext& ctx) {
        ctx.out << "null"sv;
    }

    void PrintValue(std::string value, const PrintContext& ctx) {
        ctx.out << "\""sv;
        for (const char& c : value) {
            if (c == '\n') {
                ctx.out << "\\n"sv;
                continue;
            }
            if (c == '\r') {
                ctx.out << "\\r"sv;
                continue;
            }
            if (c == '\"') ctx.out << "\\"sv;
            if (c == '\t') {
                ctx.out << "\t"sv;
                continue;
            }
            if (c == '\\') ctx.out << "\\"sv;
            ctx.out << c;
        }
        ctx.out << "\""sv;
    }

    void PrintValue(bool value, const PrintContext& ctx) {
        ctx.out << std::boolalpha << value;
    }

    void PrintValue(Array array, const PrintContext& ctx) {
        ctx.out << "[\n"sv;
        auto inner_ctx = ctx.Indented();
        bool first = true;
        for (const auto& elem : array) {
            if (first) first = false;
            else ctx.out << ",\n"s;
            inner_ctx.PrintIndent();
            PrintNode(elem, inner_ctx);
        }
        ctx.out << "\n"s;
        ctx.PrintIndent();
        ctx.out << "]"sv;
    }

    void PrintValue(Dict dict, const PrintContext& ctx) {
        ctx.out << "{\n"sv;
        auto inner_ctx = ctx.Indented();
        bool first = true;
        for (auto& [key, node] : dict) {
            if (first) first = false;
            else ctx.out << ",\n"s;
            inner_ctx.PrintIndent();
            PrintValue(key, ctx);
            ctx.out << ": ";
            PrintNode(node, inner_ctx);
        }
        ctx.out << "\n"s;
        ctx.PrintIndent();
        ctx.out << "}"sv;
    }

    void PrintNullNode(const Node&, PrintContext ctx) {
        ctx.out << "null"sv;
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintContext ctx{ output };
        PrintNode(doc.GetRoot(), ctx);
    }

}  // namespace json
