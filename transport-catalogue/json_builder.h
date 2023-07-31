#pragma once

#include "json.h"

#include <optional>

namespace json {

    // Создание классов контекстов для более удобного построения JSON-объекта
    class DictItemContext;
    class ArrayItemContext;
    class DictKeyContext;
    
    class Builder {

    public:

        Builder(); 
        DictKeyContext Key(std::string key); // для начала создания ключа в словаре
        Builder& Value(Node::Value value); // для задания значения
        DictItemContext StartDict(); // для начала создания словаря
        Builder& EndDict(); // для конца создания словаря
        ArrayItemContext StartArray(); // для начала создания массива
        Builder& EndArray(); // для конца создания массива
        Node Build(); // для построения и получения узла JSON-дерева
        Node GetNode(Node::Value value); // для получения узла JSON-дерева из значения

    private:

        Node root_{ nullptr };
        std::vector<Node*> nodes_stack_;
        std::optional<std::string> key_{ std::nullopt };
    };
          
   class DictItemContext {

    public:

        DictItemContext(Builder& builder);
        DictKeyContext Key(std::string key); // для начала создания ключа в словаре
        Builder& EndDict(); // для завершения создания словаря

    private:

        Builder& bld_;
    };

    class ArrayItemContext {

    public:

        ArrayItemContext(Builder& builder);
        ArrayItemContext Value(Node::Value value); // для задания значения в массиве
        DictItemContext StartDict(); // для начала создания словаря в массиве
        Builder& EndArray(); // для завершения создания массива
        ArrayItemContext StartArray(); // для начала создания массива в массиве

    private:

        Builder& bld_;
    };

    class DictKeyContext {

    public:

        DictKeyContext(Builder& builder);
        DictItemContext Value(Node::Value value); // для задания значения в словаре
        ArrayItemContext StartArray(); // для начала создания массива в словаре
        DictItemContext StartDict(); // для начала создания словаря в словаре

    private:

        Builder& bld_;
    };
    

} // namespace json
