// Calculator for simple expressions
// Transform to reverse polish notation(https://en.wikipedia.org/wiki/Shunting-yard_algorithm)

#include <iostream>
#include <string>
#include <queue>
#include <stack>
#include <vector>
#include <stdexcept>
#include <variant>
#include <string_view>
#include <charconv>
#include <cassert>
#include <cstring>

class Calculator
{
public:
    Calculator();
    // calculates expression
    // throws exception if encountered error
    double Caluclate(const std::string& str);

private:
    // arithmetic operation(+, -, *, /) and parentheses
    enum Operator
    {
        Plus,
        Minus,
        Multiply,
        Division,
        LeftBracket,
        RightBracket,
    };

    bool is_operator_arithmetic(Operator op) const;
    bool is_first_greater_priority(Operator operator1, Operator operator2) const;

    typedef std::variant<double, Operator> Token;

    void check_on_ascii(const std::string& str) const;
    void preprocess_str(std::string& str) const;
    void parse_normal_rotation(const std::string& str);
    void convert_to_reverse_polish();
    double calc_result_from_polish() const;

    std::vector<Token> reverse_polish;
    std::vector<Token> normal_rotation;
};

Calculator::Calculator() {}

double Calculator::Caluclate(const std::string& str)
{
    if(str.empty()) {
        throw std::runtime_error("Empty string");
    }
    check_on_ascii(str);

    std::string modified_str = str;
    preprocess_str(modified_str);

    parse_normal_rotation(modified_str);
    convert_to_reverse_polish();
    double res = calc_result_from_polish();

    reverse_polish.clear();
    normal_rotation.clear();

    return res;
}

bool Calculator::is_operator_arithmetic(Operator op) const
{
    if(op == Operator::Plus || op == Operator::Minus || op == Operator::Multiply || op == Operator::Division) {
        return true;
    } else {
        return false;
    }
}

bool Calculator::is_first_greater_priority(Operator operator1, Operator operator2) const
{
    assert(is_operator_arithmetic(operator1) && is_operator_arithmetic(operator2));
    if((operator1 == Operator::Multiply || operator1 == Operator::Division) &&
        (operator2 == Operator::Plus || operator2 == Operator::Minus))
    {
        return true;
    } else {
        return false;
    }
}

void Calculator::check_on_ascii(const std::string& str) const
{
    for (auto c: str) {
        if (static_cast<unsigned char>(c) > 127) {
            throw std::runtime_error("Bad input, supports only ascii characters");
        }
  }
}

void Calculator::preprocess_str(std::string& str) const
{
    for(int i = 0; i < str.size(); i++) {
        if(str[i] == ',') {
            str[i] = '.';
        }
    }
}

void Calculator::parse_normal_rotation(const std::string& str)
{
    normal_rotation.clear();
    
    normal_rotation.push_back(0.0);
    if(str[0] != '+' && str[0] != '-') {
        normal_rotation.push_back(Operator::Plus);
    }

    int start_index = 0;
    while(start_index < str.size()) {
        if(str[start_index] == ' ') {
            start_index++;
            continue;
        }

        if(str[start_index] == '+') {
            normal_rotation.push_back(Operator::Plus);
            start_index++;
            continue;
        } else if(str[start_index] == '-') {
            normal_rotation.push_back(Operator::Minus);
            start_index++;
            continue;
        } else if(str[start_index] == '*') {
            normal_rotation.push_back(Operator::Multiply);
            start_index++;
            continue;
        } else if(str[start_index] == '/') {
            normal_rotation.push_back(Operator::Division);
            start_index++;
            continue;
        } else if(str[start_index] == '(') {
            normal_rotation.push_back(Operator::LeftBracket);
            start_index++;
            continue;
        } else if(str[start_index] == ')') {
            normal_rotation.push_back(Operator::RightBracket);
            start_index++;
            continue;
        }

        double value = 0;
        auto [ptr, ec] = std::from_chars(str.data() + start_index, str.data() + str.size(), value);
        if(ec == std::errc::invalid_argument) {
            throw std::runtime_error("Couldnt parse, bad character");
        } else {
            normal_rotation.push_back(value);
            start_index = ptr - str.data();
        }
    }
}

void Calculator::convert_to_reverse_polish()
{
    reverse_polish.clear();

    std::stack<Operator> operator_stack;
    for(const Token& token : normal_rotation) {
        if(std::holds_alternative<double>(token)) {
            reverse_polish.push_back(token);
        } else if (std::holds_alternative<Operator>(token)) {
            Operator op = std::get<Operator>(token);
            if(is_operator_arithmetic(op)) {
                while(!operator_stack.empty()) {
                    Operator op2 = operator_stack.top();
                    if(op2 == Operator::LeftBracket) {
                        break;
                    }
                    if(is_first_greater_priority(op, op2)) {
                        break;
                    }
                    reverse_polish.push_back(op2);
                    operator_stack.pop();
                }
                operator_stack.push(op);
            } else if(op == Operator::LeftBracket) {
                operator_stack.push(op);
            } else if(op == Operator::RightBracket) {
                while(!operator_stack.empty()) {
                    Operator op2 = operator_stack.top();
                    if(is_operator_arithmetic(op2)) {
                        reverse_polish.push_back(op2);
                        operator_stack.pop();
                    } else {
                        break;
                    }
                }
                if(operator_stack.empty() || operator_stack.top() != Operator::LeftBracket) {
                    throw std::runtime_error("Bad parentheses");
                }
                operator_stack.pop();
            }
        } else {
            assert(false);
        }
    }
    while(!operator_stack.empty()) {
        if(operator_stack.top() == Operator::LeftBracket) {
            throw std::runtime_error("Bad parentheses");
        }
        reverse_polish.push_back(operator_stack.top());
        operator_stack.pop();
    }
}

double Calculator::calc_result_from_polish() const
{
    assert(!reverse_polish.empty());

    std::vector<double> res_stack;
    for(const Token& token : reverse_polish) {
        if(std::holds_alternative<double>(token)) {
            res_stack.push_back(std::get<double>(token));
        } else {
            Operator op = std::get<Operator>(token);
            assert(is_operator_arithmetic(op));
            assert(res_stack.size() >= 2);
            double a = res_stack[res_stack.size() - 1];
            double b = res_stack[res_stack.size() - 2];
            double res = 0.0;
            res_stack.pop_back();
            res_stack.pop_back();
            if(op == Operator::Plus) {
                res = b + a;
            } else if (op == Operator::Minus) {
                res = b - a;
            } else if (op == Operator::Multiply) {
                res = b * a;
            } else if (op == Operator::Division) {
                const double eps = 1e-9;
                if(abs(a) < eps) {
                    throw std::runtime_error("Division by zero");
                }
                res = b / a;
            } else {
                assert(false);
            }
            res_stack.push_back(res);
        }
    }
    assert(res_stack.size() == 1);
    return res_stack[0];
}

void RunCalculator() {
    Calculator calculator;
    std::string str;
    std::getline(std::cin, str);

    try { 
        double res = calculator.Caluclate(str);
        std::cout.precision(2);
        std::cout << std::fixed << res << '\n';
    } catch(const std::exception& e) {
        std::cout << e.what();
    }
}

void RunTest() {
    std::cout << "Running tests\n";
    const char* const expressions[] = {
        "-1 + 5 - 3",
        "-10 + (8 * 2.5) - (3 / 1,5)",
        "1 + (2 * (2.5 + 2.5 + (3 - 2))) - (3 / 1.5)",
        "1",
        "62834501 * 231 + (5534121 - 312312312) * 132 - 123125345",
        "(3.3 + 4.45) + 7.31 * 2.99 - 1.34 - 9.23",
        "5,23 - 2,12 + 4,66 / (8,12 - (5,44 + 1,66)) + 9,99",
        "(8 -   1 +   3) /   6 - ((  3 + 7) * 2   )",
        "1.1 + 2.1 + abc",
        "(1+2",
        "(1+2))",
        "1/0",
        };
    const double answers[] = {
        1,
        8,
        11,
        1,
        -26103076826,
        19.0369,
        17.668627451,
        -18.3333,
        0,
        0,
        0,
        0,
    };
    const bool should_fail[] = {
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        true,
        true,
        true,
        true,
    };

    const int tests_count = 12;
    int failed_count = 0;
    Calculator calculator;
    for(int i = 0; i < tests_count; i++) {
        bool failed = false;
        try{
            const double result = calculator.Caluclate(expressions[i]);
            if(should_fail[i]) {
                failed = true;
            } else {
                const double eps = 1e-5;
                if(abs(answers[i] - result) > eps) {
                    failed = true;
                }
            }
        }
        catch(const std::exception& e) {
            if(!should_fail[i]) {
                failed = true;
            }
        }
        if(failed) {
            failed_count++;
            std::cout << "Failed test: " + std::to_string(i) + '\n';
        } else {
            std::cout << "Passed test: " + std::to_string(i) + '\n';
        }
    }

    if(failed_count) {
        std::cout << "\nFailed " + std::to_string(failed_count) + " tests\n";
    } else {
        std::cout << "\nAll tests passed!\n";
    }
}

int main(int argc, char** argv)
{
    if(argc > 2) {
        std::cout << "Too many parametres\n";
        return 0;
    }
    if(argc == 1) {
        RunCalculator();
    } else {
        if(!strcmp(argv[1], "test")) {
            RunTest();
        } else {
            std::cout << "Unknown parameter\n";
            return 0;
        }
    }
}