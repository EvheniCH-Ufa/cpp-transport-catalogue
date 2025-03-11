#pragma once
#include<stack>
#include<string>

#include"json.h"

namespace json
{
	enum class TypeLastCont {NONE, ARRAY, DICT };

	enum class JSONCommands { NONE, Build, ARRAY, DICT, KEY };


	class ArrayItemContext;
	class ValueItemContextAfterArray;

	class DictItemContext;
	class KeyItemContext;
	class ValueItemContextAfterKey;

	class Builder
	{
	public:
		Builder();

		Builder& Value(std::string& value) //for: string, int, double
		{
			//	std::cout << "add Value " << value << std::endl;  // для отладки

			if (!node_stack_.empty() && node_containers_stack_.empty())
			{
				throw std::logic_error("Object is finished!");
			}

			if (GetLastContType() == TypeLastCont::NONE)
			{
				Node* new_node = new Node(std::move(value));
				node_stack_.emplace(new_node);
				return *this;
			}

			if (commands_stack_.top() != JSONCommands::Build
				&& commands_stack_.top() != JSONCommands::KEY
				&& commands_stack_.top() != JSONCommands::ARRAY)
			{
				throw std::logic_error("Wrong value!");
			}

			if (GetLastContType() == TypeLastCont::DICT)
			{
				commands_stack_.pop(); //delete coomand key

				auto last_elem = node_stack_.top();
				node_stack_.pop();

				Node* node = new Node(std::move(value));
				Dict dict;
				dict[(*last_elem->AsMap().begin()).first] = std::move(*node);

				Node* new_node = new Node(std::move(dict));
				node_stack_.emplace(new_node);
				delete last_elem;
				return *this;
			}

			if (GetLastContType() == TypeLastCont::ARRAY)
			{
				Node* new_node = new Node(std::move(value));
				node_stack_.emplace(new_node);
			}
			return *this;
		}

		Builder& Value(int value) //for: string, int, double
		{
			//	std::cout << "add Value " << value << std::endl; // для отладки

			if (!node_stack_.empty() && node_containers_stack_.empty())
			{
				throw std::logic_error("Object is finished!");
			}

			if (GetLastContType() == TypeLastCont::NONE)
			{
				Node* new_node = new Node(std::move(value));
				node_stack_.emplace(new_node);
				return *this;
			}

			if (commands_stack_.top() != JSONCommands::Build
				&& commands_stack_.top() != JSONCommands::KEY
				&& commands_stack_.top() != JSONCommands::ARRAY)
			{
				throw std::logic_error("Wrong value!");
			}


			if (GetLastContType() == TypeLastCont::DICT)
			{
				commands_stack_.pop(); //delete coomand key

				auto last_elem = node_stack_.top();
				node_stack_.pop();

				Node* node = new Node(std::move(value));
				Dict dict;
				dict[(*last_elem->AsMap().begin()).first] = std::move(*node);

				Node* new_node = new Node(std::move(dict));
				node_stack_.emplace(new_node);
				delete last_elem;
				return *this;
			}

			if (GetLastContType() == TypeLastCont::ARRAY)
			{
				Node* new_node = new Node(std::move(value));
				node_stack_.emplace(new_node);
			}
			return *this;
		}


		Builder& Value(double value) 
		{
			//	std::cout << "add Value " << value << std::endl; // для отладки

			if (!node_stack_.empty() && node_containers_stack_.empty())
			{
				throw std::logic_error("Object is finished!");
			}

			if (GetLastContType() == TypeLastCont::NONE)
			{
				Node* new_node = new Node(std::move(value));
				node_stack_.emplace(new_node);
				return *this;
			}

			if (commands_stack_.top() != JSONCommands::Build
				&& commands_stack_.top() != JSONCommands::KEY
				&& commands_stack_.top() != JSONCommands::ARRAY)
			{
				throw std::logic_error("Wrong value!");
			}


			if (GetLastContType() == TypeLastCont::DICT)
			{
				commands_stack_.pop(); //delete coomand key

				auto last_elem = node_stack_.top();
				node_stack_.pop();

				Node* node = new Node(std::move(value));
				Dict dict;
				dict[(*last_elem->AsMap().begin()).first] = std::move(*node);

				Node* new_node = new Node(std::move(dict));
				node_stack_.emplace(new_node);
				delete last_elem;
				return *this;
			}

			if (GetLastContType() == TypeLastCont::ARRAY)
			{
				Node* new_node = new Node(std::move(value));
				node_stack_.emplace(new_node);
			}
			return *this;
		}

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
		DictItemContext(Builder& builder) : builder_(builder)
		{}

		DictItemContext() = delete;

		KeyItemContext Key(std::string key);

		Builder& EndDict()
		{
			return builder_.EndDict();
		}

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
		ValueItemContextAfterArray Value(T value);

	private:
		Builder& builder_;
	};

	class ValueItemContextAfterArray {
	public:
		ValueItemContextAfterArray(Builder& builder) : builder_(builder) {}
		ValueItemContextAfterArray() = delete;

		Builder& EndArray();
		ArrayItemContext StartArray();
		DictItemContext StartDict();

		template <typename T>
		ValueItemContextAfterArray Value(T value);

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
	inline ValueItemContextAfterArray ArrayItemContext::Value(T value)
	{
		builder_.Value(value);
		return ValueItemContextAfterArray(builder_);
	}

	template<typename T>
	inline ValueItemContextAfterArray ValueItemContextAfterArray::Value(T value)
	{
		builder_.Value(value);
		return ValueItemContextAfterArray(builder_);
	}

	inline ValueItemContextAfterKey KeyItemContext::Value(std::string value)
	{
		builder_.Value(value);
		return ValueItemContextAfterKey(builder_);
	}

	inline ValueItemContextAfterKey KeyItemContext::Value(int value)
	{
		builder_.Value(value);
		return ValueItemContextAfterKey(builder_);
	}
	inline ValueItemContextAfterKey KeyItemContext::Value(double value)
	{
		builder_.Value(value);
		return ValueItemContextAfterKey(builder_);
	}
} //namespace json 