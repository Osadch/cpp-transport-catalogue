#include "json.h"

using namespace std;

namespace json {

    namespace {

        //  функции для парсинга нод 
        
        std::string ParseString(std::istream& input) {
            std::string result;
            while (true) {
                int c = input.get();
                if (c == std::char_traits<char>::eof() || c == '\"') {
                    break;
                }
                else if (c == '\\') {
                    c = input.get();
                    if (c == '\"') {
                        result += '\"';
                    }
                    else if (c == 'r') {
                        result += '\r';
                    }
                    else if (c == 'n') {
                        result += '\n';
                    }
                    else if (c == 't') {
                        result += '\t';
                    }
                    else if (c == '\\') {
                        result += '\\';
                    }
                    else {
                        throw ParsingError("Invalid escape sequence in string"s);
                    }
                }
                else {
                    result += static_cast<char>(c);
                }
            }
            if (input.fail()) {
                throw ParsingError("Failed to parse string from stream"s);
            }
            return result;
        }

        using Number = std::variant<int, double>;

        Number ParseNumber(std::istream& input) {
            std::string numeric_string;
            auto read_char = [&numeric_string, &input] {
                int c = input.get();
                if (c == std::char_traits<char>::eof()) {
                    throw ParsingError("Failed to read number from stream"s);
                }
                numeric_string += static_cast<char>(c);
            };
            auto read_digits = [&numeric_string, &input, &read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected in numeric string"s);
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
            bool is_integer = true;
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_integer = false;
            }
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_integer = false;
            }
            double numeric_value;
            try {
                if (is_integer) {
                    numeric_value = std::stoi(numeric_string);
                }
                else {
                    numeric_value = std::stod(numeric_string);
                }
            }
            catch (...) {
                throw ParsingError("Failed to convert number"s);
            }
            return is_integer ? static_cast<int>(numeric_value) : numeric_value;
        }
               
        //  функции парсинга нод

        Node LoadNode(istream& input);
        Node LoadArray(istream& input) {
            Array result;
            char c;
            while (input >> c) {
                if (c == ']') {
                    break;
                }
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            // проверяем, что массив заканчивается на ]
            if (c != ']') {
                throw ParsingError("Failed to parse array node");
            }
            return Node(move(result));
        }     

        Node LoadDict(istream& input) {
            Dict result;
            char c;
            input >> c;
            // проверяем, если словарь пустой
            if (c == '}') {
                return Node(Dict{});
            }
            else {
                input.putback(c);
            }
            while (input >> c) {
                // считываем ключ
                input.putback(c);
                string key = ParseString(input);
                // считываем разделитель
                input >> c;
                if (c != ':') {
                    throw ParsingError("Failed to parse dict node");
                }
                // считываем значение и записываем в словарь
                result.emplace(move(key), LoadNode(input));
                // считываем следующий символ (должен быть либо "}" либо ",")
                input >> c;
                if (c == '}') {
                    break;
                }
                else if (c != ',') {
                    throw ParsingError("Failed to parse dict");
                }
            }
            // проверяем, что если поток закончился, то последний символ был }
            if (c != '}') {
                throw ParsingError("Failed to parse dict node");
            }
            return Node(move(result));
        }

        Node LoadBool(istream& input) {
            std::string res = ParseString(input);
            if (res != "true"s && res != "false"s) {
                throw ParsingError("Failed to parse bool node");
            }
            if (res == "true") {
                return Node(true);
            }
            else {
                return Node(false);
            }
        }


Node LoadNode(istream& input)
{
    return Node();
}


}  // namespace

    // методы проверки на тип значения 

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

    
    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        Document result{ LoadNode(input) };
        if (char c; input >> c) {
            throw ParsingError("Failed to parse document"s);
        }
        return result;
    }

    namespace {

        // печать нодов 

        std::string AddEscapes(const std::string& str) {
            std::string result;
            for (auto c : str) {
                if (c == '\"') {
                    result += "\\\"";
                }
                else if (c == '\r') {
                    result += "\\r";
                }
                else if (c == '\n') {
                    result += "\\n";
                }
                else if (c == '\\') {
                    result += "\\\\";
                }
                else {
                    result += c;
                }
            }
            return result;
        }

        void PrintNode(const Node& node, RenderContext ctx);

        void PrintNullNode(const Node&, RenderContext ctx) {
            ctx.out << "null"sv;
        }

        void PrintBoolNode(const Node& node, RenderContext ctx) {
            if (node.AsBool()) {
                ctx.out << "true"sv;
            }
            else {
                ctx.out << "false"sv;
            }
        }

        void PrintIntNode(const Node& node, RenderContext ctx) {
            ctx.out << node.AsInt();
        }

        void PrintDoubleNode(const Node& node, RenderContext ctx) {
            ctx.out << node.AsDouble();
        }

        void PrintStringNode(const Node& node, RenderContext ctx) {
            ctx.out << "\""sv << AddEscapes(node.AsString()) << "\""sv;
        }

        void PrintArrayNode(const Node& node, RenderContext ctx) {
            auto arr = node.AsArray();
            auto size = arr.size();
            if (size != 0) {
                ctx.out << "["sv;
                PrintNode(arr.at(0), ctx);
                for (auto i = 1u; i < size; ++i) {
                    ctx.out << ", "sv;
                    PrintNode(arr.at(i), ctx);
                }
                ctx.out << "]"sv;
            }
            else {
                ctx.out << "[]"sv;
            }
        }

        void PrintMapNode(const Node& node, RenderContext ctx) {
            auto map = node.AsMap();
            auto size = map.size();
            if (size != 0) {
                ctx.out << "{"sv << std::endl;
                RenderContext map_ctx(ctx.out, ctx.indent + 2);
                map_ctx.RenderIndent();
                PrintNode(map.begin()->first, map_ctx);
                map_ctx.out << ": "sv;
                PrintNode(map.begin()->second, map_ctx);
                for (auto it = std::next(map.begin()); it != map.end(); ++it) {
                    map_ctx.out << ","sv << std::endl;
                    map_ctx.RenderIndent();
                    PrintNode(it->first, map_ctx);
                    map_ctx.out << ": "sv;
                    PrintNode(it->second, map_ctx);
                }
                ctx.out << std::endl;
                ctx.RenderIndent();
                ctx.out << "}"sv;
            }
            else {
                ctx.out << "{}"sv;
            }
        }

        void PrintNode(const Node& node, RenderContext ctx) {
            if (node.IsNull()) {
                PrintNullNode(node, ctx);
            }
            else if (node.IsBool()) {
                PrintBoolNode(node, ctx);
            }
            else if (node.IsInt()) {
                PrintIntNode(node, ctx);
            }
            else if (node.IsPureDouble()) {
                PrintDoubleNode(node, ctx);
            }
            else if (node.IsString()) {
                PrintStringNode(node, ctx);
            }
            else if (node.IsArray()) {
                PrintArrayNode(node, ctx);
            }
            else if (node.IsMap()) {
                PrintMapNode(node, ctx);
            }
        }

    } // namespace

    void Print(const Document& doc, std::ostream& output) {
        RenderContext ctx(output, 0);
        PrintNode(doc.GetRoot(), ctx);
    }

}  // namespace json
