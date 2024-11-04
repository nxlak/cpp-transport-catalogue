#pragma once

#include <string>
#include <vector>
#include "json.h"

namespace json {

class Builder;

class KeyItemContext;
class DictItemContext;
class ArrayItemContext;

class DictItemContext {
public:
    explicit DictItemContext(Builder& builder) : builder_(builder) {}

    KeyItemContext Key(std::string key);
    Builder& EndDict();

private:
    Builder& builder_;
};

class ArrayItemContext {
public:
    explicit ArrayItemContext(Builder& builder) : builder_(builder) {}

    ArrayItemContext& Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();

private:
    Builder& builder_;
};

class KeyItemContext {
public:
    explicit KeyItemContext(Builder& builder) : builder_(builder) {}

    DictItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();

private:
    Builder& builder_;
};

class Builder {
public:
    Builder();
    Node Build();
    Builder& Value(Node::Value value);
    KeyItemContext Key(std::string key);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();

private:
    Node root_;
    std::vector<Node*> nodes_stack_;

    Node::Value& GetCurrentValue();
    //я не придумал ничего лучше, чем использование bool one_shot + про отдельный метод AddObject мне писал даже наставник 
    //я уже увидел и разобрал хорошее решение, почему я не могу на него ориентироваться? 
    void AddObject(Node::Value value, bool one_shot);
};

}  // namespace json
