#include <iostream>
#include <iterator>
#include <vector>
#include <stack>
#include <list>
#include <string>
#include <locale>
#include <exception>
#include <fstream>
#include <streambuf>

#include <stdio.h>
#include <string.h>

#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2

using namespace std;

// Shamelessly taken from stackoverflow
static int getLine (string prmpt, char *buff, size_t sz) {
	int ch, extra;

	// Get line with buffer overrun protection.
	cout << prmpt;
	fflush (stdout);

	if (fgets (buff, sz, stdin) == NULL) return NO_INPUT;

	// If it was too long, there'll be no newline. In that case, we flush
	// to end of line so that excess doesn't affect the next call.
	if (buff[strlen(buff)-1] != '\n') {
		extra = 0;
		while (((ch = getchar()) != '\n') && (ch != EOF))
			extra = 1;
		return (extra == 1) ? TOO_LONG : OK;
	}

	// Otherwise remove newline and give string back to caller.
	buff[strlen(buff)-1] = '\0';
	return OK;
}

template<typename T>
ostream& operator<< (ostream& out, const vector<T>& vector) {
	out << "vector[ ";
	for(auto const&character : vector) cout << character << " ";
	return out << "]";
}

template<typename T>
ostream& operator<< (ostream& out, const list<T>& list) {
	out << "list[ ";
	for(auto const&character : list) cout << character << " ";
	return out << "]";
}

template<typename T>
ostream& operator<< (ostream& out, const stack<T>& stack) {
	return out << "stack[size=" << stack.size() << "]";
}

enum ValueType {BOOLEAN, INTEGER, REAL};

struct Node {
	virtual ostream& show(ostream& out) const {return out << "Hi, my name is Raul";}
};
ostream& operator<< (ostream& out, const Node& node) {
	return node.show(out);
}

struct SymbolNode : Node {
	string symbol;

	SymbolNode(string symbol) {
		this->symbol = symbol;
	}

	virtual ostream& show(ostream& out) const {
		return out << symbol;
	}
};

struct ValueNode : Node {
	ValueType type;
	union {
		bool boolValue;
		int  intValue;
		long floatValue;
	};

	ValueNode(bool  value) {this->type = BOOLEAN; this->boolValue  = value;}
	ValueNode(int   value) {this->type = INTEGER; this->intValue   = value;}
	ValueNode(float value) {this->type = REAL;	this->floatValue = value;}

	virtual ostream& show(ostream& out) const {
		switch(this->type) {
			case BOOLEAN: return cout << this->boolValue;
			case INTEGER: return cout << this->intValue;
			case REAL: return cout << this->floatValue;
			default: return cout << "INVALIDTYPE!!!";
		}
	}
};

struct CallNode : Node {
	list<Node*> args;

	CallNode(list<Node*> args) {
		this->args = args;
	}

	virtual ostream& show(ostream& out) const {
		cout << "( ";
		for(auto const&character : this->args) cout << *character << " ";
		return out << ")";
	}
};

Node* parseNode(string input) {
	if(input == "True" || input == "False") return new ValueNode(input == "True");

	try {
		return new ValueNode(stoi(input));
	} catch(const exception& e) {
		return new SymbolNode(input);
	}
}

vector<string> tokenise(string input) {
	vector<string> output = vector<string>();
	string word = "";

	for(auto const& character : input) {
		switch(character) {
			case ')': case '(':
				if(word != "") output.push_back(word);
				output.push_back(string(1, character));
				word = "";
				break;
			case ' ':
				if(word != "") output.push_back(word);
				word = "";
				break;
			default:
				word += character;
		}
	}

	if(word != "") output.push_back(word);
	return output;
}

Node* parse(vector<string> input) {
	stack<string> progress = stack<string>();
	list<list<Node*>> nodes = list<list<Node*>>();
	nodes.push_back(list<Node*>());

	for(auto const& symbol : input) {
		if(symbol == "(") {
			progress.push(symbol);
			nodes.push_back(list<Node*>());
		} else if(symbol == ")") {
			// Try to construct our node
			bool found = false;

			while(progress.size() > 0) {
				string test = progress.top();
				progress.pop();

				if(test == "(") {
					found = true;
					break;
				} else {
					Node * temp = parseNode(test);
					nodes.back().push_front(temp);
				}
			}

			if(nodes.size() < 2) throw underflow_error("Unmatched brackets!");

			CallNode * node = new CallNode(nodes.back());
			nodes.pop_back();
			nodes.back().push_back(node);

			if(!found) return nullptr;
		} else {
			progress.push(symbol);
		}
	}

	if(nodes.size() == 1 && nodes.front().size() == 1) {
		return nodes.front().front();
	} else {
		throw logic_error("Bad brackets (Probably)");
	}
}

Node* execute(Node* root) {
	if(CallNode* c = dynamic_cast<CallNode*>(root)) {
		if(c->args.size() == 0) throw runtime_error("No arguments!");

		if(SymbolNode* s = dynamic_cast<SymbolNode*>(c->args.front())) {
			string symbol = s->symbol;
			list<Node*> evaluated = list<Node*>();

			// Execute
			list<Node*>::const_iterator iterator;
			for (iterator = next(c->args.begin(), 1); iterator != c->args.end(); ++iterator) {
				evaluated.push_back(execute(*iterator));
			}
			c->args = evaluated;

			if(symbol == "exit") {
				cout << "   Exiting." << endl;
				exit(0);
			} else if(symbol == "credits") {
				// YOLO lazy.
				cout << "   Inspired by Lisp, Created by Matthew Consterdine." << endl;
				return new ValueNode(0);
			} else if(symbol == "random") {
				// xkcd
				return new ValueNode(4);
			} else if(symbol == "+") {
				int total = 0;

				if(ValueNode* v = dynamic_cast<ValueNode*>(*c->args.begin())) {
					total = v->intValue;
				} else {
					throw runtime_error("Not a value!");
				}

				for (iterator = next(c->args.begin(), 1); iterator != c->args.end(); ++iterator) {
					if(ValueNode* v = dynamic_cast<ValueNode*>(*iterator)) {
						total += v->intValue;
					} else {
						throw runtime_error("Not a value!");
					}
				}
				return new ValueNode(total);
			} else if(symbol == "-") {
				int total = 0;

				if(ValueNode* v = dynamic_cast<ValueNode*>(*c->args.begin())) {
					total = v->intValue;
				} else {
					throw runtime_error("Not a value!");
				}

				for (iterator = next(c->args.begin(), 1); iterator != c->args.end(); ++iterator) {
					if(ValueNode* v = dynamic_cast<ValueNode*>(*iterator)) {
						total -= v->intValue;
					} else {
						throw runtime_error("Not a value!");
					}
				}
				return new ValueNode(total);
			} else if(symbol == "*") {
				int total = 0;

				if(ValueNode* v = dynamic_cast<ValueNode*>(*c->args.begin())) {
					total = v->intValue;
				} else {
					throw runtime_error("Not a value!");
				}

				for (iterator = next(c->args.begin(), 1); iterator != c->args.end(); ++iterator) {
					if(ValueNode* v = dynamic_cast<ValueNode*>(*iterator)) {
						total *= v->intValue;
					} else {
						throw runtime_error("Not a value!");
					}
				}
				return new ValueNode(total);
			} else if(symbol == "/") {
				int total = 0;

				if(ValueNode* v = dynamic_cast<ValueNode*>(*c->args.begin())) {
					total = v->intValue;
				} else {
					throw runtime_error("Not a value!");
				}

				for (iterator = next(c->args.begin(), 1); iterator != c->args.end(); ++iterator) {
					if(ValueNode* v = dynamic_cast<ValueNode*>(*iterator)) {
						total /= v->intValue;
					} else {
						throw runtime_error("Not a value!");
					}
				}
				return new ValueNode(total);
			} else {
				throw runtime_error("Not implemented!");
			}

			return new ValueNode(0);
		} else {
			throw runtime_error("Not a symbol!");
		}
	} else if(ValueNode* v = dynamic_cast<ValueNode*>(root)) {
		return v;
	} else {
		throw runtime_error("Invalid node!");
	}
}

// Interactive repl
int repl() {
	while(true) {
		int rc;
		char buff[256];

		rc = getLine(">> ", buff, sizeof(buff));
		if (rc == NO_INPUT) {
			// Extra NL since my system doesn't output that on EOF.
			printf ("\n   No input!\n");
			return 1;
		} else if (rc == TOO_LONG) {
			printf ("   Input too long [%s]!\n", buff);
			return 1;
		} else if(string(buff).size() == 0) {
			// Do nothing if nothing is entered
			continue;
		} else {
			try {
				vector<string> tokenised = tokenise(buff);
				// cout << "   Tokened: " << tokenised << endl;
				Node* parsed = parse(tokenised);
				// cout << "   Parsed: " << *parsed << endl;
				cout << "<< " << *execute(parsed) << endl;

			} catch(exception& e) {
				cerr << "   Exception: " << e.what() << endl;
			}
		}
	}

	return 0;
}

// Load file from disk and execute
int disk(char* file) {
	std::ifstream t(file);
	std::string str;

	t.seekg(0, std::ios::end);
	str.reserve(t.tellg());
	t.seekg(0, std::ios::beg);

	str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

	try {
		cout << *execute(parse(tokenise(str))) << endl;
	} catch(exception& e) {
		cerr << "Exception: " << e.what() << endl;
	}

	return 0;
}

int main(int argc, char* argv[]) {
	if(argc == 1) {
		return repl();
	} else {
		return disk(argv[1]);
	}
}