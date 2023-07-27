#pragma once
#include "json.h"
namespace json {

class Builder;
class BaseContext;
class DictItemContext;
class ArrItemContext;
class KeyContext;

class Builder {
public:   
    ArrItemContext StartArray();
    DictItemContext StartDict();
    KeyContext Key(const std::string key);
    Builder& Value(const Node::Value value);
    Builder& EndArray();
    Builder& EndDict();
    Node Build();

private:
    // конструируемый объект.
    Node root_{ nullptr };
    // стек указателей на те вершины JSON, которые ещё не построены
	std::vector<Node*> nodes_stack_;
    // имя ключа
    std::string key_;
    // ключ задан и ждёт значения
    bool key_started{ false };
};

// BaseContext - класс, который хранит в себе ссылку на Builder.
// Методы BaseContext вызывают методы Builder'a. 
class BaseContext : public Builder
{
public:
    BaseContext(Builder& b) : builder_(b)
    {}

    KeyContext Key(const std::string s);
    BaseContext Value(const Node::Value);
    DictItemContext StartDict();
    ArrItemContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();
    Node Build();

private:
    Builder& builder_;
};

// Вспомогательный класс для выявления ошибки при компиляции
// За вызовом StartDict следует не Key и не EndDict.
// После вызова Value, последовавшего за вызовом Key, вызван не Key и не EndDict.
class DictItemContext : public BaseContext {
public:
    DictItemContext(BaseContext base) : BaseContext(base) {}

    BaseContext Value(const Node::Value) = delete;
    DictItemContext StartDict() = delete;
    Builder& StartArray() = delete;
    BaseContext EndArray() = delete;
    Node Build() = delete;
};

// Вспомогательный класс для выявления ошибки при компиляции
// За вызовом StartArray следует не Value, не StartDict, не StartArray и не EndArray.
// После вызова StartArray и серии Value следует не Value, не StartDict, не StartArray и не EndArray.
class ArrItemContext : public BaseContext {
public:
    ArrItemContext(BaseContext base) : BaseContext(base) {}

    ArrItemContext Value(const Node::Value);
    KeyContext Key(const std::string) = delete;
    BaseContext EndDict() = delete;
    Node Build() = delete;
};

// Вспомогательный класс для выявления ошибки при компиляции
// Непосредственно после Key вызван не Value, не StartDict и не StartArray.
class KeyContext : public BaseContext {
public:
    KeyContext(BaseContext base) : BaseContext(base) {}

    DictItemContext Value(const Node::Value);
    KeyContext Key(const std::string s) = delete;
    BaseContext EndDict() = delete;
    BaseContext EndArray() = delete;
    Node Build() = delete;
};
}
