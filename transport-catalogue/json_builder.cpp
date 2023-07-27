#include "json_builder.h"

namespace json {
    ArrItemContext Builder::StartArray() {
        if (root_ == nullptr)
        {
            root_ = Array();
            nodes_stack_.emplace_back(&root_);
            return ArrItemContext{ *this };
        }

        // Вызов StartArray() после Value() 
        if (nodes_stack_.empty())
        {
            throw std::logic_error("StartArray() called after Value()");
        }

        if (nodes_stack_.back()->IsArray())
        {
            const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Array());
            Node* node = &const_cast<Array&>(nodes_stack_.back()->AsArray()).back();
            nodes_stack_.push_back(node);
        }
        else if (nodes_stack_.back()->IsDict())
        {
            if (key_started)
            {
                const_cast<Dict&>(nodes_stack_.back()->AsDict())[key_] = Array();
                Node* node = &const_cast<Dict&>(nodes_stack_.back()->AsDict()).at(key_);
                nodes_stack_.push_back(node);
                key_started = false;
            }
            else
            {
                // Вызов StartArray() внутри словаря без создания ключа Key()
                throw std::logic_error("StartArray() called (Key() expected)");
            }
        }
        else
        {
            throw std::logic_error("StartArray() error");
        }

        return ArrItemContext{ *this };
    }

    DictItemContext Builder::StartDict()
    {
        if (root_ == nullptr)
        {
            root_ = Dict();
            nodes_stack_.emplace_back(&root_);
            return DictItemContext{ *this };
        }

        // Вызов StartDict() после Value() 
        if (nodes_stack_.empty())
        {
            throw std::logic_error("StartDict() called after Value()");
        }

        if (nodes_stack_.back()->IsArray())
        {
            const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Dict());
            Node* node = &const_cast<Array&>(nodes_stack_.back()->AsArray()).back();
            nodes_stack_.push_back(node);
        }
        else if (nodes_stack_.back()->IsDict())
        {
            if (key_started)
            {
                const_cast<Dict&>(nodes_stack_.back()->AsDict())[key_] = Dict();
                Node* node = &const_cast<Dict&>(nodes_stack_.back()->AsDict()).at(key_);
                nodes_stack_.push_back(node);
                key_started = false;
            }
            else
            {
                // Вызов StartDict() внутри словаря без создания ключа Key()
                throw std::logic_error("StartDict() called (Key() expected)");
            }
        }
        else
        {
            throw std::logic_error("StartDict() error.");
        }
        return DictItemContext{ *this };
    }

    KeyContext Builder::Key(const std::string key) {
        if (key_started) {
            throw std::logic_error("Key() called after Key()");
        }

        if ((!nodes_stack_.empty()) && (nodes_stack_.back()->IsDict()))
        {
            key_ = std::move(key);
            key_started = true;
            const_cast<Dict&>(nodes_stack_.back()->AsDict())[key_] = Node();
        }
        else
        {
            throw std::logic_error("Key() called in wrong place");
        }
        return KeyContext{ *this };
    }

    Builder& Builder::Value(const Node::Value value) {
        if (root_ == nullptr)
        {
            // GetValue() - для преобразования входящего std::variant
            root_.GetValue() = std::move(value);
            return *this;
        }

        // Вызов Value() после Value() вне массива или словаря
        if (nodes_stack_.empty())
        {
            throw std::logic_error("Value() \"" + std::get<std::string>(value) + "\" called after another Value()");
        }


        if (nodes_stack_.back()->IsArray()) {
            Array& arr = const_cast<Array&>(nodes_stack_.back()->AsArray());
            //arr.emplace_back(std::move(value));
            arr.emplace_back();
            arr.back().GetValue() = std::move(value);

        }
        else if (nodes_stack_.back()->IsDict())
        {
            if (key_started)
            {
                const_cast<Dict&>(nodes_stack_.back()->AsDict())[key_].GetValue() = std::move(value);
                key_started = false;
            }
            else
            {
                throw std::logic_error("Error adding \"" + std::get<std::string>(value) + "\" to dictionary key \"" + key_);
            }
        }
        else
        {
            throw std::logic_error("Error adding \"" + std::get<std::string>(value) + "\"  - unknown container");
        }

        return *this;
    }


    Builder& Builder::EndArray()
    {
        if ((!nodes_stack_.empty()) && (nodes_stack_.back()->IsArray()))
        {
            nodes_stack_.pop_back();
        }
        else
        {
            throw std::logic_error("EndArray() called in wrong place");
        }

        return *this;
    }

    Builder& Builder::EndDict()
    {
        if ((!nodes_stack_.empty()) && (nodes_stack_.back()->IsDict()))
        {
            nodes_stack_.pop_back();
        }
        else
        {
            throw std::logic_error("EndDict() called in wrong place");
        }

        return *this;
    }

    json::Node Builder::Build() {
        if (root_ == nullptr)
        {
            throw std::logic_error("Empty node");
        }
        else if (!nodes_stack_.empty())
        {
            throw std::logic_error("Build() called before End() method");
        }

        return root_;

    }

    KeyContext BaseContext::Key(const std::string key) {
        return builder_.Key(key);
    }
    BaseContext BaseContext::Value(const Node::Value value) {
        return builder_.Value(value);
    }
    DictItemContext BaseContext::StartDict() {
        return builder_.StartDict();
    }
    DictItemContext KeyContext::Value(const Node::Value value) {
        return BaseContext::Value(value);
    }
    ArrItemContext BaseContext::StartArray() {
        return builder_.StartArray();
    }
    ArrItemContext ArrItemContext::Value(const Node::Value value) {
        return BaseContext::Value(value);
    }
    BaseContext BaseContext::EndDict() {
        return builder_.EndDict();
    }
    BaseContext BaseContext::EndArray() {
        return builder_.EndArray();
    }
    Node BaseContext::Build() {
        return builder_.Build();
    }

}