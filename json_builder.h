#pragma once

#include "json.h"
#include <stdexcept>
#include <vector>
#include <optional>
#include <variant>

namespace json {

class Builder {
public:
    class KeyContext;
    class DictItemContext;
    class ArrayItemContext;
    class ValueAfterKeyContext;
    
    Builder() = default;

    Node Build();

    KeyContext Key(std::string key);
    Builder& Value(Node::Value value);

    DictItemContext StartDict();
    Builder& EndDict();

    ArrayItemContext StartArray();
    Builder& EndArray();

private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> current_key_;
    bool is_complete_ = false;

    Node& GetCurrentNode();
    void CheckCompleted();

    void CheckContextForValue();
    void CheckContextForKey();
};

class Builder::KeyContext {
public:
    KeyContext(Builder& builder) : builder_(builder) {}

    DictItemContext StartDict();
    ArrayItemContext StartArray();
    ValueAfterKeyContext Value(Node::Value value);

private:
    Builder& builder_;
};

class Builder::DictItemContext {
public:
    DictItemContext(Builder& builder) : builder_(builder) {}

    KeyContext Key(std::string key);
    Builder& EndDict();

private:
    Builder& builder_;
};

class Builder::ArrayItemContext {
public:
    ArrayItemContext(Builder& builder) : builder_(builder) {}

    ArrayItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();

private:
    Builder& builder_;
};

class Builder::ValueAfterKeyContext {
public:
    ValueAfterKeyContext(Builder& builder) : builder_(builder) {}

    KeyContext Key(std::string key);
    Builder& EndDict();

private:
    Builder& builder_;
};

}  // namespace json