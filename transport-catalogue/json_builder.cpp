#include "json_builder.h"
#include <exception>
#include <variant>
#include <utility>

using namespace std::literals;

namespace json {

// DictItemContext 
KeyItemContext DictItemContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder& DictItemContext::EndDict() {
    return builder_.EndDict();
}

// ArrayItemContext 
ArrayItemContext& ArrayItemContext::Value(Node::Value value) {
    builder_.Value(std::move(value));
    return *this;
}

DictItemContext ArrayItemContext::StartDict() {
    return builder_.StartDict();
}

ArrayItemContext ArrayItemContext::StartArray() {
    return builder_.StartArray();
}

Builder& ArrayItemContext::EndArray() {
    return builder_.EndArray();
}

// KeyItemContext 
DictItemContext KeyItemContext::Value(Node::Value value) {
    builder_.Value(std::move(value));
    return DictItemContext(builder_);
}

DictItemContext KeyItemContext::StartDict() {
    return builder_.StartDict();
}

ArrayItemContext KeyItemContext::StartArray() {
    return builder_.StartArray();
}

// Builder 
Builder::Builder()
    : root_()
    , nodes_stack_{&root_}
{}

Node Builder::Build() {
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Attempt to build JSON which isn't finalized"s);
    }
    return std::move(root_);
}

KeyItemContext Builder::Key(std::string key) {
    Node::Value& host_value = GetCurrentValue();
    
    if (!std::holds_alternative<Dict>(host_value)) {
        throw std::logic_error("Key() outside a dict"s);
    }
    
    nodes_stack_.push_back(
        &std::get<Dict>(host_value)[std::move(key)]
    );
    return KeyItemContext(*this);
}
    
Builder& Builder::Value(Node::Value value) {
    AddObject(std::move(value), true);
    return *this;
}

DictItemContext Builder::StartDict() {
    AddObject(Dict{}, false);
    return DictItemContext(*this);
}

ArrayItemContext Builder::StartArray() {
    AddObject(Array{}, false);
    return ArrayItemContext(*this);
}

    
Builder& Builder::EndDict() {
    if (!std::holds_alternative<Dict>(GetCurrentValue())) {
        throw std::logic_error("EndDict() outside a dict"s);
    }
    nodes_stack_.pop_back();
    return *this;
}

Builder& Builder::EndArray() {
    if (!std::holds_alternative<Array>(GetCurrentValue())) {
        throw std::logic_error("EndDict() outside an array"s);
    }
    nodes_stack_.pop_back();
    return *this;
}

Node::Value& Builder::GetCurrentValue() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Attempt to change finalized JSON"s);
    }
    return nodes_stack_.back()->GetValue();
}

void Builder::AddObject(Node::Value value, bool one_shot) {
    Node::Value& host_value = GetCurrentValue();
    if (std::holds_alternative<Array>(host_value)) {
        Node& node = std::get<Array>(host_value).emplace_back(std::move(value));
        if (!one_shot) {
            nodes_stack_.push_back(&node);
        }
    } else {
        if (!std::holds_alternative<std::nullptr_t>(GetCurrentValue())) {
            throw std::logic_error("New object in wrong context"s);
        }
        host_value = std::move(value);
        if (one_shot) {
            nodes_stack_.pop_back();
        }
    }
}
    
}  // namespace json
