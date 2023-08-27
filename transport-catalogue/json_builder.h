#pragma once
 
#include "json.h"
#include <stack>
#include <string>
#include <memory>
 
namespace transport_catalogue {
namespace detail {
namespace json {
namespace builder {
 
// Создание классов контекстов для более удобного построения JSON-объекта    
class KeyContext;
class DictionaryContext;
class ArrayContext;
 
class Builder {
public:
    Node make_node(const Node::Value& value_); // для получения узла JSON-дерева из значения
    void add_node(const Node& node); 
    KeyContext key(const std::string& key_); // для начала создания ключа в словаре
    Builder& value(const Node::Value& value); // для задания значения   
    DictionaryContext start_dict(); // для начала создания словаря
    Builder& end_dict(); // для конца создания словаря   
    ArrayContext start_array(); // для начала создания массива
    Builder& end_array(); // для конца создания массива
    Node build(); // для построения и получения узла JSON-дерева
    
 
private:
    Node root_;
    std::vector<std::unique_ptr<Node>> nodes_stack_;
};
 /* Класс `BaseContext` является базовым классом для классов `KeyContext`, `DictionaryContext` и `ArrayContext`. Вся функциональность этих классов тут */
class BaseContext {
public:
    BaseContext(Builder& builder); // конструктор принимает ссылку на объект `Builder`  
    KeyContext key(const std::string& key); // для задания ключа в качестве имени следующего элемента
    Builder& value(const Node::Value& value);  // для задания значения следующего элемента  
    DictionaryContext start_dict(); // для начала создания словаря в массиве
    Builder& end_dict(); // для завершения создания словаря   
    ArrayContext start_array(); // для начала создания массива в массиве
    Builder& end_array(); // для завершения создания массива    
  
protected:
    Builder& builder_;
};
 
class KeyContext : public BaseContext {
public:
    KeyContext(Builder& builder); // принимается ссылка на объект `Builder`
    KeyContext key(const std::string& key) = delete; // удаление из класса
    BaseContext end_dict() = delete;
    BaseContext end_array() = delete; 
    DictionaryContext value(const Node::Value& value); // для задания значения следующего элемента в словаре
};
 
class DictionaryContext : public BaseContext {
public:
    DictionaryContext(Builder& builder); // принимается ссылка на объект `Builder`
    DictionaryContext start_dict() = delete; 
    ArrayContext start_array() = delete;
    Builder& end_array() = delete; 
    Builder& value(const Node::Value& value) = delete; // для задания значения следующего элемента
};
 
class ArrayContext : public BaseContext {
public:
    ArrayContext(Builder& builder); // принимается ссылка на объект `Builder`
    KeyContext key(const std::string& key) = delete; // удаление из класса
    Builder& end_dict() = delete; 
    ArrayContext value(const Node::Value& value); // для задания значения следующего элемента
};
 
}//end namespace builder
}//end namespace json
}//end namespace detail
}//end namespace transport_catalogue
