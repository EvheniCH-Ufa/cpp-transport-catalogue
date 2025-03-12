#pragma once
#include<stack>
#include<string>

#include"json.h"

namespace json
{
	enum class TypeLastCont { NONE, ARRAY, DICT };
	enum class JSONCommands { NONE, Build, ARRAY, DICT, KEY };

	class ArrayItemContext;
	class DictItemContext;
	class KeyItemContext;
	class ValueItemContextAfterKey;

	class Builder
	{
	public:
		Builder();

		Builder& Value(std::string& value);
		Builder& Value(int value);
		Builder& Value(double value);
		DictItemContext StartDict();
		Builder& EndDict();
		KeyItemContext Key(std::string key);
		ArrayItemContext StartArray();
		Builder& EndArray();
		Node& Build();
	private:
		std::stack<Node*> node_stack_;
		std::stack<Node*> node_containers_stack_;
		std::stack<JSONCommands> commands_stack_;
		Node* root_ = nullptr;

		TypeLastCont GetLastContType();
	};


	class DictItemContext {
	public:
		DictItemContext(Builder& builder) : builder_(builder) {}
		DictItemContext() = delete;

		KeyItemContext Key(std::string key);
		Builder& EndDict();
	private:
		Builder& builder_;
	};

	class KeyItemContext {
	public:
		KeyItemContext(Builder& builder) : builder_(builder) {}
		KeyItemContext() = delete;

		ValueItemContextAfterKey Value(std::string value);
		ValueItemContextAfterKey Value(int value);
		ValueItemContextAfterKey Value(double value);
		ArrayItemContext StartArray();
		DictItemContext StartDict();
	private:
		Builder& builder_;
	};

	class ArrayItemContext {
	public:
		ArrayItemContext(Builder& builder) : builder_(builder) {}
		ArrayItemContext() = delete;

		ArrayItemContext StartArray();
		Builder& EndArray();
		DictItemContext StartDict();

		template <typename T>
		ArrayItemContext Value(T value);
	private:
		Builder& builder_;
	};
	
	class ValueItemContextAfterKey {
	public:
		ValueItemContextAfterKey(Builder& builder) : builder_(builder) {}
		ValueItemContextAfterKey() = delete;

		Builder& EndDict();
		KeyItemContext Key(std::string key);
	private:
		Builder& builder_;
	};
	
	template<typename T>
	inline ArrayItemContext ArrayItemContext::Value(T value)
	{
		builder_.Value(value);
		return ArrayItemContext(builder_);
	}

} //namespace json 