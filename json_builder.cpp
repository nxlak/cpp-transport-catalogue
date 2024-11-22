#include "json_builder.h"

namespace json {
    
void Builder::CheckContextForValue() {
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && !current_key_.has_value()) {
        throw std::logic_error("Value() called without key!");
    }
    if (nodes_stack_.empty() && !root_.IsNull()) {
        throw std::logic_error("Value() already set!");
    }
}

void Builder::CheckContextForKey() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict() || current_key_.has_value()) {
        throw std::logic_error("Key() called in incorrect context!");
    }
}

void Builder::CheckCompleted() {
    if (is_complete_) {
        throw std::logic_error("Already competed!");
    }
}
    
Node Builder::Build() {
    if (!is_complete_ || !nodes_stack_.empty()) {
        throw std::logic_error("Not ready for Build!");
    }
    return std::move(root_);
}

Builder::KeyContext Builder::Key(std::string key) {
    CheckCompleted();
    CheckContextForKey();
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict() || current_key_.has_value()) {
        throw std::logic_error("Key() called in incorrect context!");
    }
    current_key_ = std::move(key);
    return KeyContext(*this);
}

Builder& Builder::Value(Node::Value value) {
    CheckCompleted();
    CheckContextForValue();

    if (nodes_stack_.empty() && root_.IsNull()) {
        root_ = Node(std::move(value));
        is_complete_ = true;
        return *this;
    }

    if (!nodes_stack_.empty()) {
        if (nodes_stack_.back()->IsDict()) {
            if (!current_key_.has_value()) {
                throw std::logic_error("Value() called without key!");
            }
            std::get<Dict>(nodes_stack_.back()->GetValue())[std::move(current_key_.value())] = Node(std::move(value));
            current_key_.reset();
        } else if (nodes_stack_.back()->IsArray()) {
            std::get<Array>(nodes_stack_.back()->GetValue()).push_back(Node(std::move(value)));
        } else {
            throw std::logic_error("Value() called in incorrect context!");
        }
    }

    return *this;
}

Builder::DictItemContext Builder::StartDict() {
    CheckCompleted();
    if (!nodes_stack_.empty()) {
        if (nodes_stack_.back()->IsDict()) {
            if (!current_key_.has_value()) {
                throw std::logic_error("StartDict() called without key!");
            }
            Dict& dict = std::get<Dict>(nodes_stack_.back()->GetValue());
            Node dict_node(Dict{});
            dict[current_key_.value()] = dict_node;
            nodes_stack_.push_back(&dict[current_key_.value()]);
            current_key_.reset();
        } else if (nodes_stack_.back()->IsArray()) {
            Array& array = std::get<Array>(nodes_stack_.back()->GetValue());
            array.emplace_back(Dict{});
            nodes_stack_.push_back(&array.back());
        }
    } else if (root_.IsNull()) {
        root_ = Node(Dict{});
        nodes_stack_.push_back(&root_);
    } else {
        throw std::logic_error("StartDict() called in incorrect context!");
    }

    return DictItemContext(*this);
}

Builder& Builder::EndDict() {
    CheckCompleted();
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
        throw std::logic_error("EndDict() called without StartDict!");
    }
    nodes_stack_.pop_back();
    if (nodes_stack_.empty()) {
        is_complete_ = true;
    }
    return *this;
}

Builder::ArrayItemContext Builder::StartArray() {
    CheckCompleted();
    if (!nodes_stack_.empty()) {
        if (nodes_stack_.back()->IsDict()) {
            if (!current_key_.has_value()) {
                throw std::logic_error("StartArray() called without key!");
            }
            Dict& dict = std::get<Dict>(nodes_stack_.back()->GetValue());
            Node array_node(Array{});
            dict[current_key_.value()] = array_node;
            nodes_stack_.push_back(&dict[current_key_.value()]);
            current_key_.reset();
        } else if (nodes_stack_.back()->IsArray()) {
            Array& array = std::get<Array>(nodes_stack_.back()->GetValue());
            array.emplace_back(Array{});
            nodes_stack_.push_back(&array.back());
        }
    } else if (root_.IsNull()) {
        root_ = Node(Array{});
        nodes_stack_.push_back(&root_);
    } else {
        throw std::logic_error("StartArray() called in incorrect context!");
    }

    return ArrayItemContext(*this);
}

Builder& Builder::EndArray() {
    CheckCompleted();
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("EndArray() called without StartArray!");
    }
    nodes_stack_.pop_back();
    if (nodes_stack_.empty()) {
        is_complete_ = true;
    }
    return *this;
}

// KeyContext
Builder::DictItemContext Builder::KeyContext::StartDict() {
    return builder_.StartDict();
}

Builder::ArrayItemContext Builder::KeyContext::StartArray() {
    return builder_.StartArray();
}

Builder::ValueAfterKeyContext Builder::KeyContext::Value(Node::Value value) {
    builder_.Value(value);
    return ValueAfterKeyContext(builder_);
}

// DictItemContext
Builder::KeyContext Builder::DictItemContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder& Builder::DictItemContext::EndDict() {
    return builder_.EndDict();
}

// ArrayItemContext
Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
    builder_.Value(std::move(value));
    return *this;
}

Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
    return builder_.StartDict();
}

Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
    return builder_.StartArray();
}

Builder& Builder::ArrayItemContext::EndArray() {
    return builder_.EndArray();
}

// ValueAfterKeyContext
Builder::KeyContext Builder::ValueAfterKeyContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder& Builder::ValueAfterKeyContext::EndDict() {
    return builder_.EndDict();
}

}  // namespace json