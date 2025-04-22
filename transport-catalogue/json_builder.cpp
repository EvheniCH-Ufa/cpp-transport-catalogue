#include "json_builder.h"
#include <cassert>

//#include <iostream>
// int st_dick = 0; // для отладки

namespace json
{
	Builder::Builder()
	{
		commands_stack_.emplace(JSONCommands::Build);
	}

	Builder& Builder::Value(std::string& value)
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

	Builder& Builder::Value(int value)
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

	Builder& Builder::Value(double value)
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

	DictItemContext Builder::StartDict()
	{
		//std::cout << "StartDict " << std::endl; // для отладки

		if (!node_stack_.empty() && node_containers_stack_.empty())
		{
			throw std::logic_error("Object is finished!");
		}

		if (commands_stack_.top() != JSONCommands::Build
			&& commands_stack_.top() != JSONCommands::KEY
			&& commands_stack_.top() != JSONCommands::ARRAY)
		{
			throw std::logic_error("Wrong value!");
		}

//        std::cout << "st_dick = " << st_dick << std::endl; // otladka

		commands_stack_.emplace(JSONCommands::DICT);

//        ++st_dick;  /*otladka*/

		Dict dict;
		Node* new_node = new Node(dict);
		node_stack_.emplace(new_node);
		node_containers_stack_.emplace(node_stack_.top());

		return DictItemContext(*this);
	}

	Builder& Builder::EndDict()
	{
	//	std::cout << "EndDict " << std::endl; // для отладки

		if (!node_stack_.empty() && node_containers_stack_.empty())
		{
			throw std::logic_error("Object is finished!");
		}

		if (GetLastContType() != TypeLastCont::DICT) // то есть текущий контейнер это не MAP
		{
			throw std::logic_error("Ne vernoe zavershenie Dict");
		}
			
		if (node_stack_.empty())
		{
			throw std::logic_error("Ne vernoe zavershenie element of Dict"); // то есть стек пуст, а мы что-то хотим
		}

		Dict new_dict;
		// когда он становится равен ссылке на базовый контейнер, значит мы выбрали все элементы
		while (node_stack_.top() != node_containers_stack_.top()) 
		{
			auto node = node_stack_.top();
			if (!node->AsMap().empty())
			{
				new_dict.insert({ node->AsMap().begin()->first
							, node->AsMap().begin()->second });
			}
			
			node_stack_.pop();
			delete node;
		}  // и теперь нужно сделать нормальный мап

		auto node = node_stack_.top();
		delete node;
		node_stack_.pop();
		node_containers_stack_.pop();

		Node* new_node = new Node(std::move(new_dict));
		commands_stack_.pop(); // delete command dict

		if (!commands_stack_.empty() && commands_stack_.top() == JSONCommands::KEY)
		{
			Dict dict;
			dict.insert({ node_stack_.top()->AsMap().begin()->first
							, std::move(*new_node) });

			Node* new_node_dict = new Node(std::move(dict));
			node_stack_.emplace(new_node_dict);
			commands_stack_.pop(); // delete command key
		}
		else
		{
			node_stack_.emplace(new_node);
		}
		return *this;
	}

	KeyItemContext Builder::Key(std::string key)		
	{
		//std::cout << "add Key " << key << std::endl;  // для отладки
		{
			if (!node_stack_.empty() && node_containers_stack_.empty())
			{
				throw std::logic_error("Object is finished!");
			}

			if (GetLastContType() != TypeLastCont::DICT)
			{
				throw std::logic_error("Out of Dict");
			}
			if (commands_stack_.top() == JSONCommands::KEY)
			{
				throw std::logic_error("Insert double Key to Dict");
			}
		}

		commands_stack_.emplace(JSONCommands::KEY);

		Node* null_node = new Node(nullptr);
		Dict dict;
		dict[std::move(key)] = std::move(*null_node);

		Node* new_node = new Node(std::move(dict));
		node_stack_.emplace(new_node);

		return KeyItemContext (*this);
	}

	ArrayItemContext Builder::StartArray()
	{
		//std::cout << "StartArray " << std::endl;  // для отладки

		if (!node_stack_.empty() && node_containers_stack_.empty())
		{
			throw std::logic_error("Object is finished!");
		}

		if (commands_stack_.top() != JSONCommands::Build
			&& commands_stack_.top() != JSONCommands::KEY
			&& commands_stack_.top() != JSONCommands::ARRAY)
		{
			throw std::logic_error("Wrong value!");
		}
		commands_stack_.push(JSONCommands::ARRAY);

		Array arr;
		Node* new_node = new Node(std::move(arr));
		node_stack_.emplace(new_node);
		node_containers_stack_.emplace(node_stack_.top());

		return ArrayItemContext(*this);
	}

	Builder& Builder::EndArray()
	{
		//std::cout << "EndArray " << std::endl; // для отладки

		if (!node_stack_.empty() && node_containers_stack_.empty())
		{
			throw std::logic_error("Object is finished!");
		}

		if (GetLastContType() != TypeLastCont::ARRAY) // то есть текущий контейнер это не MAP
		{
			throw std::logic_error("Ne vernoe zavershenie Array");
		}

		if (node_stack_.empty())
		{
			throw std::logic_error("Stack is empty"); // то есть стек пуст, а мы что-то хотим
		}

		Array arr;
		// когда он становится равен ссылке на базовый контейнер, значит мы выбрали все элементы
		while (node_stack_.top() != node_containers_stack_.top())
		{
			Node* node = node_stack_.top();
			arr.push_back(std::move(*node)); // тут что-то лежит...
			node_stack_.pop();
			delete node;
		}  

		auto node = node_stack_.top(); // сам массив
		delete node;
		node_stack_.pop();
		node_containers_stack_.pop();

		Array rev_arr(arr.rbegin(), arr.rend());

		Node* arr_node = new Node(std::move(rev_arr));
		
		if (node_stack_.empty() || !node_stack_.top()->IsMap())
		{
			node_stack_.emplace(arr_node);
		}
		else if (node_stack_.top()->IsMap())
		{
			Dict dict;
			dict.insert({ node_stack_.top()->AsMap().begin()->first
							, std::move(*arr_node) });

			Node* new_node = new Node(std::move(dict));
			node_stack_.emplace(new_node);
		}

		commands_stack_.pop(); // delete command array
		if (!commands_stack_.empty() && commands_stack_.top() == JSONCommands::KEY)
		{
			commands_stack_.pop(); // delete command key
		}

		return *this;
	}

	Node& Builder::Build()
	{
	//	std::cout << "Build " << std::endl; // для отладки

		if (node_stack_.empty())
		{
			throw std::logic_error("Stack is empty!");
		};

		if (node_stack_.size() > 1)
		{
			throw std::logic_error("Stack is not empted!");
		}

		if (!node_containers_stack_.empty())
		{
			throw std::logic_error("Array or Dict not finished!");
		}

		root_ = node_stack_.top();
		node_stack_.pop();
		
		return *root_;
	}

	TypeLastCont Builder::GetLastContType()
	{
		if (node_containers_stack_.empty())
		{
			return TypeLastCont::NONE;
		}

		if (node_containers_stack_.top()->IsArray())
		{
			return TypeLastCont::ARRAY;
		}
		else if (node_containers_stack_.top()->IsMap())
		{
			return TypeLastCont::DICT;
		}
		return TypeLastCont::NONE;
	}



	ArrayItemContext KeyItemContext::StartArray()
	{
		return builder_.StartArray();
	}

	DictItemContext KeyItemContext::StartDict()
	{
		return builder_.StartDict();
	}



	KeyItemContext DictItemContext::Key(std::string key)
	{
		return builder_.Key(key);
	}

	Builder& DictItemContext::EndDict()
	{
		return builder_.EndDict();
	}



	Builder& ArrayItemContext::EndArray()
	{
		return builder_.EndArray();
	}

	ArrayItemContext ArrayItemContext::StartArray()
	{
		return builder_.StartArray();
	}

	DictItemContext ArrayItemContext::StartDict()
	{
		return builder_.StartDict();
	}



	Builder& ValueItemContextAfterKey::EndDict()
	{
		return builder_.EndDict();
	}

	KeyItemContext ValueItemContextAfterKey::Key(std::string key)
	{
		return builder_.Key(key);
	}



	ValueItemContextAfterKey KeyItemContext::Value(std::string value)
	{
		builder_.Value(value);
		return ValueItemContextAfterKey(builder_);
	}

	ValueItemContextAfterKey KeyItemContext::Value(int value)
	{
		builder_.Value(value);
		return ValueItemContextAfterKey(builder_);
	}

	ValueItemContextAfterKey KeyItemContext::Value(double value)
	{
		builder_.Value(value);
		return ValueItemContextAfterKey(builder_);
	}
}